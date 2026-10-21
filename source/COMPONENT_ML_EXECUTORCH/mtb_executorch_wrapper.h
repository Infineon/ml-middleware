/******************************************************************************
* File Name: mtb_executorch_wrapper.h
*
* Description: ExecutorTorch wrapper class declaration for MTB ML middleware
*
*******************************************************************************
* (c) 2026, Infineon Technologies AG, or
* an affiliate of Infineon Technologies AG. All rights reserved.
*******************************************************************************
* This software, associated documentation and materials ("Software") is owned
* by Infineon Technologies AG or one of its affiliates ("Infineon") and is
* protected by and subject to worldwide patent protection, worldwide copyright
* laws, and international treaty provisions. Therefore, you may use this
* Software only as provided in the license agreement accompanying the software
* package from which you obtained this Software. If no license agreement
* applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE IS
* PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING,
* BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF THIRD-PARTY RIGHTS
* AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A SPECIFIC
* USE/PURPOSE OR MERCHANTABILITY. Infineon reserves the right to make changes
* to the Software without notice. You are responsible for properly designing,
* programming, and testing the functionality and safety of your intended
* application of the Software, as well as complying with any legal requirements
* related to its use. Infineon does not guarantee that the Software will be
* free from intrusion, data theft or loss, or other breaches ("Security
* Breaches"), and Infineon shall have no liability arising out of any Security
* Breaches. Unless otherwise explicitly approved by Infineon, the Software may
* not be used in any application where a failure of the Product or any
* consequences of the use thereof can reasonably be expected to result in
* personal injury.
*******************************************************************************/

#ifndef __MTB_EXECUTORCH_WRAPPER_H__
#define __MTB_EXECUTORCH_WRAPPER_H__

#include <executorch/extension/data_loader/buffer_data_loader.h>
#include <executorch/runtime/executor/program.h>
#include <executorch/runtime/core/memory_allocator.h>
#include <executorch/runtime/executor/memory_manager.h>
#include <vector>
#include <new>
#include <utility>
#include <optional>

#include "arm_memory_allocator.h"
#include "mtb_ml_model_defs.h"

#ifndef ET_ARM_BAREMETAL_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE
#define ET_ARM_BAREMETAL_SCRATCH_TEMP_ALLOCATOR_POOL_SIZE (0x200000)
#endif

#ifndef ET_ARM_BAREMETAL_METHOD_ALLOCATOR_POOL_SIZE
#define ET_ARM_BAREMETAL_METHOD_ALLOCATOR_POOL_SIZE (0x18000)
#endif

using executorch::extension::BufferDataLoader;
using executorch::runtime::Program;
using executorch::runtime::Method;
using executorch::runtime::MemoryManager;
using executorch::runtime::HierarchicalAllocator;
using executorch::runtime::Error;
using executorch::runtime::Result;
using executorch::runtime::EValue;
using executorch::aten::Tensor;
using executorch::runtime::MethodMeta;
using executorch::runtime::Span;
using executorch::runtime::Tag;
using executorch::runtime::TensorInfo;

namespace executorch_mtb {

/**
 * Box<T>
 *
 * Lightweight, in-place storage for a single instance of T.
 *
 * Purpose
 * - Provides manual, stack-allocated storage for a T without performing heap
 *   allocation. Useful where you want optional-like behavior or to avoid
 *   dynamic allocation for a contained object.
 */
template <typename T>
class Box {
public:
    Box() = default;
    Box(const Box&) = delete;
    Box& operator=(const Box&) = delete;

    template <typename... Args>
    void reset(Args&&... args) {
        if (has_value_) {
            reinterpret_cast<T*>(mem_)->~T();
        }
        new (mem_) T(std::forward<Args>(args)...);
        has_value_ = true;
    }

    bool has_value() const { return has_value_; }
    T& value() { return *ptr(); }
    const T& value() const { return *ptr(); }
    T* operator->() { return ptr(); }
    const T* operator->() const { return ptr(); }

private:
    T* ptr() { return reinterpret_cast<T*>(mem_); }
    const T* ptr() const { return reinterpret_cast<const T*>(mem_); }

    alignas(T) uint8_t mem_[sizeof(T)];
    bool has_value_ = false;
};

/**
 * MTBExecuTorch wrapper class
 * Follows TFLM MTBTFLiteMicro pattern for API compatibility
 */
class MTBExecuTorch {
public:
    /**
     * Multi-input/output metadata structure
     */
    struct TensorMeta {
        MTB_ML_DATA_T* ptr;         /* Pointer to tensor data */
        size_t bytes;               /* Size in bytes */
        size_t elements;            /* Number of elements */
        int dim_len;                /* Number of dimensions */
        int* dims;                  /* Dimension sizes array */
        std::optional<int> zero_point;   /* Quantization zero point (nullopt if not quantized) */
        std::optional<float> scale;      /* Quantization scale (nullopt if not quantized) */
        uint8_t type_size;          /* Element size in bytes */
        size_t concat_offset_bytes; /* Offset in concatenated buffer */
    };

    /**
     * Constructor - performs all ExecutorTorch initialization
     * @param model_data Pointer to PTE model binary
     * @param model_size Size of model binary in bytes
     * @param method_arena Pointer to method allocator arena
     * @param method_arena_size Size of method arena in bytes
    * @param temp_arena Pointer to temp allocator arena
    * @param temp_arena_size Size of temp arena in bytes
     */
    MTBExecuTorch(const uint8_t* model_data,
                  size_t model_size,
                  uint8_t* method_arena,
               size_t method_arena_size,
               uint8_t* temp_arena,
               size_t temp_arena_size);

    /**
     * Destructor - cleans up all ExecutorTorch resources
     */
    ~MTBExecuTorch();

    /* Core execution methods (match TFLM interface) */

    /**
     * Execute single inference iteration
     * @return Error::Ok on success, error code otherwise
     */
    Error RunSingleIteration();

    /**
     * Copy input data to model input tensor
     * @param custom_input Pointer to input data
     * @param input_index Tensor index
     */
    void SetInput(const MTB_ML_DATA_T* custom_input, int input_index = 0);

    /**
     * Reset temp allocator before next inference
     */
    void reset_temp_allocator();

    /**
     * Reset RNN state variables if present
     * @return Error::Ok
     */
    Error reset_all_variables();

    /* Status and diagnostic methods */

    /**
     * Get initialization/allocation status
     * @return Error::Ok if initialization succeeded, error code otherwise
     */
    Error AllocationStatus() const { return allocation_status_; }

    /**
     * Get method allocator arena size used during initialization
     * @return Number of bytes used from method arena
     */
    size_t get_used_arena_size() const { return method_allocator_used_; }

    /* Fields populated during init — valid for the lifetime of the object */
    int input_count = 0;
    int output_count = 0;
    size_t input_concat_bytes = 0;
    size_t output_concat_bytes = 0;
    std::vector<TensorMeta> input_meta;
    std::vector<TensorMeta> output_meta;

private:
    /* Objects tracking the core ExecutorTorch components and resources.
     * Box<T> is used for manual lifetime management without heap allocation. */
    Box<BufferDataLoader> data_loader_;
    Box<Program> program_;
    Box<ArmMemoryAllocator> method_allocator_;
    Box<ArmMemoryAllocator> temp_allocator_;
    std::vector<uint8_t*> planned_buffers_;
    std::vector<Span<uint8_t>> planned_spans_;
    Box<HierarchicalAllocator> planned_memory_;
    Box<MemoryManager> memory_manager_;
    Box<Method> method_;

    uint8_t* temp_arena_base_;
    size_t temp_arena_size_;

    uint8_t* method_arena_base_;
    size_t method_arena_size_;

    /* Metadata */
    const char* method_name_;
    Error allocation_status_;
    size_t method_allocator_used_;
    bool has_quantization_params_;
    std::vector<EValue> input_tensors_;



    /* Initialization helper methods */
    /**
     * Load program from model binary
     * @param model_data Pointer to PTE model binary
     * @param model_size Size of model binary in bytes
     * @return Error::Ok on success, error code otherwise
     */
    Error load_program(const uint8_t* model_data, size_t model_size);

    /**
     * Read planned memory buffers encoded in method metadata and initialize planned memory allocator
     * @param method_meta Method metadata containing buffer requirements
     * @return Error::Ok on success, error code otherwise
     */
    Error setup_planned_memory(const MethodMeta& method_meta);

    /**
    * Setup method allocator for permanent allocations with the resolved (user-supplied or heap) method pool
     * @return Error::Ok on success, error code otherwise
     */
    Error setup_method_allocator();

    /**
    * Setup temp allocator for temporary allocations with the resolved (user-supplied or heap) temp pool
     * @return Error::Ok on success, error code otherwise
     */
    Error setup_temp_allocator();

    /**
     * Load method and create memory manager
     * @return Error::Ok on success, error code otherwise
     */
    Error load_method();

    /**
     * Allocate wrapper-owned input tensors from the method allocator and
     * register them with the loaded Method instance.
     * @param method_meta Method metadata containing tensor information
     * @return Error::Ok on success, error code otherwise
     */
    Error initialize_input_tensors(const MethodMeta& method_meta);

    /**
     * Populate input/output tensor metadata structures from model metadata
     * @param method_meta Method metadata containing tensor information
     * @return Error::Ok on success, error code otherwise
     */
    Error populate_tensor_metadata(const MethodMeta& method_meta);

    /**
     * Build a single TensorMeta from resolved TensorInfo and EValue
     * @param tinfo Tensor layout/type information
     * @param eval Live EValue providing the data pointer
     * @param index Tensor index (used for quantization param lookup and logging)
     * @param io_tag "input" or "output" (used for quantization param lookup and logging)
     * @param concat_offset_bytes Running byte offset in the concatenated buffer
     * @param out_meta Output TensorMeta to populate
     * @return Error::Ok on success, error code otherwise
     */
    Error build_tensor_meta(const TensorInfo& tinfo, const EValue& eval,
                            size_t index, const char* io_tag,
                            size_t concat_offset_bytes, TensorMeta& out_meta);

    /* Helper methods */
    /**
     * Execute a quantization parameter method to retrieve scale or zero point parameters
     * @param index Tensor index
     * @param tensor_type "input" or "output"
     * @param suffix "_zp" or "_scale"
     * @return EValue containing the result
     */
    std::optional<EValue> read_quantization_param(int index, const char* tensor_type, const char* suffix);
};

} /* namespace executorch_mtb */

#endif /* __MTB_EXECUTORCH_WRAPPER_H__ */
