/******************************************************************************
* File Name: mtb_executorch_wrapper.cpp
*
* Description: ExecutorTorch wrapper class implementation for MTB ML middleware
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

#include "mtb_executorch_wrapper.h"
#include <executorch/runtime/core/exec_aten/util/dim_order_util.h>
#include <executorch/runtime/platform/log.h>
#include <executorch/runtime/core/exec_aten/util/scalar_type_util.h>
#include <inttypes.h>
#include <executorch/runtime/core/error.h>
#include <cstring>
#include <algorithm>

namespace executorch_mtb
{

    MTBExecuTorch::MTBExecuTorch(const uint8_t *model_data,
                                 size_t model_size,
                                 uint8_t *method_arena,
                                 size_t method_arena_size,
                                 uint8_t *temp_arena,
                                 size_t temp_arena_size)
        : temp_arena_base_(temp_arena),
          temp_arena_size_(temp_arena_size),
          method_arena_base_(method_arena),
          method_arena_size_(method_arena_size),
          method_name_(nullptr),
          allocation_status_(Error::Ok),
          method_allocator_used_(0),
          has_quantization_params_(true)
    {
        ET_LOG(
            Debug,
            "MTBExecuTorch ctor: model_size=%zu",
            model_size
            );

        /* Execute initialization phases in sequence */
        allocation_status_ = load_program(model_data, model_size);
        if (allocation_status_ != Error::Ok)
        {
            return;
        }

        allocation_status_ = setup_method_allocator();
        if (allocation_status_ != Error::Ok)
        {
            return;
        }

        allocation_status_ = setup_temp_allocator();
        if (allocation_status_ != Error::Ok)
        {
            return;
        }

        allocation_status_ = load_method();
        if (allocation_status_ != Error::Ok)
        {
            return;
        }

        /* Record used arena size */
        method_allocator_used_ = method_allocator_->used_size();
        ET_LOG(
            Debug,
            "Method arena used=%zu free=%zu",
            method_allocator_used_,
            method_allocator_->free_size());

        allocation_status_ = Error::Ok;
    }

    MTBExecuTorch::~MTBExecuTorch()
    {
        /* Resources are cleaned up automatically by Box */
    }

    Error MTBExecuTorch::load_program(const uint8_t *model_data, size_t model_size)
    {
        /* Create BufferDataLoader instance */
        data_loader_.reset(model_data, model_size);

        /* Load Program */
        program_.reset(ET_UNWRAP(
            Program::load(&data_loader_.value()),
            "Program load failed"));

        /* Get first method name */
        method_name_ = ET_UNWRAP(
            program_->get_method_name(0),
            "Failed to get method name");
        ET_LOG(Debug, "Using method %s", method_name_);

        return Error::Ok;
    }

    Error MTBExecuTorch::setup_method_allocator()
    {
        method_allocator_.reset(method_arena_size_, method_arena_base_);
        ET_LOG(Debug, "Method allocator ready: arena_size=%zu", method_arena_size_);
        return Error::Ok;
    }

    Error MTBExecuTorch::setup_temp_allocator()
    {
        temp_allocator_.reset(temp_arena_size_, temp_arena_base_);
        ET_LOG(Debug, "Temp allocator ready: size=%zu", temp_arena_size_);
        return Error::Ok;
    }

    Error MTBExecuTorch::setup_planned_memory(const MethodMeta &method_meta)
    {
        size_t num_planned_buffers = method_meta.num_memory_planned_buffers();
        planned_spans_.clear();

        for (size_t i = 0; i < num_planned_buffers; ++i)
        {
            size_t buffer_size = static_cast<size_t>(ET_UNWRAP(
                method_meta.memory_planned_buffer_size(i),
                "Planned buffer %zu size query failed",
                i));

            /* Allocate from method arena with 16-byte alignment */
            uint8_t *buffer = reinterpret_cast<uint8_t *>(
                method_allocator_->allocate(buffer_size, 16UL));
            ET_CHECK_OR_RETURN_ERROR(
                buffer != nullptr,
                MemoryAllocationFailed,
                "Planned buffer %zu allocation failed",
                i);

            planned_buffers_.push_back(buffer);
            planned_spans_.push_back(Span<uint8_t>(buffer, buffer_size));
            ET_LOG(Debug, "Planned buffer %zu size=%zu", i, buffer_size);
        }

        planned_memory_.reset(
            Span<Span<uint8_t>>(planned_spans_.data(), planned_spans_.size()));
        ET_LOG(Debug, "Planned memory allocator initialized (%zu buffers)", planned_spans_.size());

        return Error::Ok;
    }

    Error MTBExecuTorch::load_method()
    {
        /* Get method metadata */
        MethodMeta method_meta = ET_UNWRAP(
            program_->method_meta(method_name_),
            "Failed to get method meta");

        /* Setup planned memory buffers */
        Error status = setup_planned_memory(method_meta);
        ET_CHECK_OK_OR_RETURN_ERROR(status);

        /* Create MemoryManager */
        memory_manager_.reset(
            &method_allocator_.value(),
            &planned_memory_.value(),
            &temp_allocator_.value());
        ET_LOG(Debug, "MemoryManager created");

        /* Load method */
        method_.reset(ET_UNWRAP(
            program_->load_method(method_name_, &memory_manager_.value(), nullptr),
            "Method load failed"));
        ET_LOG(Debug, "Method %s loaded", method_name_);

        status = initialize_input_tensors(method_meta);
        ET_CHECK_OK_OR_RETURN_ERROR(status);

        return populate_tensor_metadata(method_meta);
    }

    Error MTBExecuTorch::initialize_input_tensors(const MethodMeta &method_meta)
    {
        ET_CHECK_OR_RETURN_ERROR(
            method_.has_value() && method_allocator_.has_value(),
            InvalidState,
            "Method or allocator not initialized");

        /* Initialize local input tensor list */
        size_t num_inputs = static_cast<size_t>(method_->inputs_size());
        input_tensors_.clear();
        input_tensors_.reserve(num_inputs);

        /* Iterate inputs */
        for (size_t i = 0; i < num_inputs; ++i)
        {
            auto tensor_meta_result = method_meta.input_tensor_meta(i);
            ET_CHECK_OR_RETURN_ERROR(
                tensor_meta_result.ok(),
                InvalidState,
                "Failed to get tensor meta for input %zu",
                i);

            /* Get Metadata for input tensor from method metadata and validate */
            const TensorInfo &tensor_info = tensor_meta_result.get();
            auto sizes = tensor_info.sizes();
            auto dim_order = tensor_info.dim_order();
            size_t dims = sizes.size();

            ET_CHECK_OR_RETURN_ERROR(
                !tensor_info.is_memory_planned(),
                NotSupported,
                "Input %zu is memory planned; non-memory-planned inputs are required for direct tensor access.",
                i);
            ET_CHECK_OR_RETURN_ERROR(
                dims > 0,
                InvalidState,
                "Input %zu has no dimensions",
                i);
            ET_CHECK_OR_RETURN_ERROR(
                dim_order.size() == dims,
                InvalidState,
                "Input %zu dim_order length %zu does not match dims %zu",
                i,
                dim_order.size(),
                dims);

            /* Allocate metadata arrays from method arena to pass to newly created TensorImpl */
            auto *tensor_sizes =
                method_allocator_->allocateList<executorch::aten::TensorImpl::SizesType>(dims);
            auto *tensor_dim_order =
                method_allocator_->allocateList<executorch::aten::TensorImpl::DimOrderType>(dims);
            auto *tensor_strides =
                method_allocator_->allocateList<executorch::aten::TensorImpl::StridesType>(dims);
            ET_CHECK_OR_RETURN_ERROR(
                tensor_sizes != nullptr &&
                    tensor_dim_order != nullptr &&
                    tensor_strides != nullptr,
                MemoryAllocationFailed,
                "Failed to allocate input %zu tensor metadata",
                i);

            /* Copy sizes and dim order into method arena */
            for (size_t d = 0; d < dims; ++d)
            {
                tensor_sizes[d] = sizes[d];
                tensor_dim_order[d] = dim_order[d];
            }

            /* Calculate strides based on sizes and dim order */
            Error status = executorch::runtime::dim_order_to_stride(
                tensor_sizes,
                tensor_dim_order,
                dims,
                tensor_strides);
            ET_CHECK_OK_OR_RETURN_ERROR(
                status,
                "Failed to compute strides for input %zu",
                i);

            /* Allocate buffer for tensor */
            void *buffer = method_allocator_->allocate(tensor_info.nbytes(), 16UL);
            ET_CHECK_OR_RETURN_ERROR(
                buffer != nullptr,
                MemoryAllocationFailed,
                "Failed to allocate input %zu buffer (%zu bytes)",
                i,
                tensor_info.nbytes());

            /* Allocate TensorImpl object for this input tensor in method arena */
            auto *tensor_impl = method_allocator_->allocateInstance<executorch::aten::TensorImpl>();
            ET_CHECK_OR_RETURN_ERROR(
                tensor_impl != nullptr,
                MemoryAllocationFailed,
                "Failed to allocate TensorImpl for input %zu",
                i);

            /* Allocate TensorImpl in method arena and construct in-place with metadata and buffer pointer */
            new (tensor_impl) executorch::aten::TensorImpl(
                tensor_info.scalar_type(),
                static_cast<ssize_t>(dims),
                tensor_sizes,
                buffer,
                tensor_dim_order,
                tensor_strides);

            /* Wrap TensorImpl in EValue and register as method input */
            executorch::aten::Tensor input_tensor(tensor_impl);
            input_tensors_.push_back(EValue(input_tensor));

            status = method_->set_input(input_tensors_.back(), i);
            ET_CHECK_OK_OR_RETURN_ERROR(
                status,
                "Failed to register local input tensor %zu: status=0x%" PRIx32,
                i,
                static_cast<uint32_t>(status));
        }

        return Error::Ok;
    }

    Error MTBExecuTorch::build_tensor_meta(
        const TensorInfo &tinfo,
        const EValue &eval,
        size_t index,
        const char *io_tag,
        size_t concat_offset_bytes,
        TensorMeta &result)
    {
        auto sizes = tinfo.sizes();
        ET_CHECK_OR_RETURN_ERROR(
            !sizes.empty(),
            InvalidState,
            "%s %zu has no dimensions",
            io_tag,
            index);
        ET_CHECK_OR_RETURN_ERROR(
            eval.isTensor(),
            InvalidState,
            "%s %zu is not a tensor EValue",
            io_tag,
            index);

        /* Allocate dims array from method arena for stable lifetime */
        int *dims = static_cast<int *>(
            method_allocator_->allocate(sizes.size() * sizeof(int), alignof(int)));
        ET_CHECK_OR_RETURN_ERROR(
            dims != nullptr,
            MemoryAllocationFailed,
            "Failed to allocate dims for %s %zu",
            io_tag,
            index);

        /* Copy sizes to method arena */
        for (size_t d = 0; d < sizes.size(); ++d)
        {
            dims[d] = static_cast<int>(sizes[d]);
        }

        size_t elem_size = executorch::runtime::elementSize(tinfo.scalar_type());
        size_t nbytes = tinfo.nbytes();

        /* Populate metadata structure */
        result.ptr = static_cast<MTB_ML_DATA_T *>(
            const_cast<EValue &>(eval).toTensor().mutable_data_ptr<void>());
        result.bytes = nbytes;
        result.elements = (elem_size > 0) ? (nbytes / elem_size) : 0;
        result.dim_len = static_cast<int>(sizes.size());
        result.dims = dims;
        result.type_size = static_cast<uint8_t>(elem_size);
        result.concat_offset_bytes = concat_offset_bytes;

        /* Populate quantization parameters if present */
        auto zp = read_quantization_param(static_cast<int>(index), io_tag, "_zp");
        result.zero_point = (zp.has_value() && zp->isInt())
            ? std::optional<int>(static_cast<int>(zp->toInt())) : std::nullopt;

        auto sc = read_quantization_param(static_cast<int>(index), io_tag, "_scale");
        result.scale = (sc.has_value() && sc->isDouble())
            ? std::optional<float>(static_cast<float>(sc->toDouble())) : std::nullopt;

        return Error::Ok;
    }

    Error MTBExecuTorch::populate_tensor_metadata(const MethodMeta &method_meta)
    {
        ET_CHECK_OR_RETURN_ERROR(
            method_.has_value() && method_allocator_.has_value(),
            InvalidState,
            "Method or allocator not initialized");

        /* Get number of model inputs and outputs */
        input_count  = static_cast<int>(method_->inputs_size());
        output_count = static_cast<int>(method_->outputs_size());
        size_t num_inputs  = static_cast<size_t>(input_count);
        size_t num_outputs = static_cast<size_t>(output_count);

        /* Reset existing metadata and reserve space for new ones */
        input_meta.clear();
        output_meta.clear();
        input_meta.reserve(num_inputs);
        output_meta.reserve(num_outputs);

        ET_CHECK_OR_RETURN_ERROR(
            input_tensors_.size() == num_inputs,
            InvalidState,
            "Local input tensor count %zu does not match method input count %zu",
            input_tensors_.size(),
            num_inputs);

        Error status = Error::Ok;

        /* Running pointer offset counter */
        size_t input_concat_total = 0;
        for (size_t i = 0; i < num_inputs; ++i)
        {
            /* Get i-th input tensor metadata */
            auto tensor_meta_result = method_meta.input_tensor_meta(i);
            ET_CHECK_OR_RETURN_ERROR(
                tensor_meta_result.ok(),
                InvalidState,
                "Failed to get tensor meta for input %zu",
                i);

            /* Copy metadata to MTB-ML's own data structure */
            TensorMeta meta;
            status = build_tensor_meta(
                tensor_meta_result.get(),
                input_tensors_[i],
                i,
                "input",
                input_concat_total,
                meta);
            ET_CHECK_OK_OR_RETURN_ERROR(status);
            /* Update running offset */
            input_concat_total += meta.bytes;
            input_meta.push_back(meta);
        }
        input_concat_bytes = input_concat_total;

        /* Repeat for outputs */
        size_t output_concat_total = 0;
        for (size_t i = 0; i < num_outputs; ++i)
        {
            auto tensor_meta_result = method_meta.output_tensor_meta(i);
            ET_CHECK_OR_RETURN_ERROR(
                tensor_meta_result.ok(),
                InvalidState,
                "Failed to get tensor meta for output %zu",
                i);

            TensorMeta meta;
            status = build_tensor_meta(tensor_meta_result.get(), method_->get_output(i), i, "output", output_concat_total, meta);
            ET_CHECK_OK_OR_RETURN_ERROR(status);
            output_concat_total += meta.bytes;
            output_meta.push_back(meta);
        }
        output_concat_bytes = output_concat_total;

        ET_LOG(Debug, "Metadata: %zu inputs (%zu bytes), %zu outputs (%zu bytes)",
               num_inputs, input_concat_bytes,
               num_outputs, output_concat_bytes);

        /* Clear temp allocations used during metadata population */
        reset_temp_allocator();

        return Error::Ok;
    }

    Error MTBExecuTorch::RunSingleIteration()
    {
        ET_CHECK_OR_RETURN_ERROR(
            method_.has_value(),
            InvalidState,
            "RunSingleIteration called before method init");
        ET_LOG(Info, "Executing single iteration for %s", method_name_);
        Error status = method_->execute();
        if (status != Error::Ok)
        {
            ET_LOG(
                Error,
                "Method execution failed: status=0x%" PRIx32,
                static_cast<uint32_t>(status));
        }
        return status;
    }

    void MTBExecuTorch::SetInput(const MTB_ML_DATA_T *custom_input, int input_index)
    {
        if (custom_input == nullptr)
        {
            ET_LOG(Error, "SetInput: custom_input is null");
            return;
        }

        if (input_index < 0 || static_cast<size_t>(input_index) >= input_meta.size())
        {
            ET_LOG(Error, "SetInput: input_index %d out of range [0, %zu)",
                   input_index, input_meta.size());
            return;
        }

        const TensorMeta &meta = input_meta[input_index];
        std::memcpy(meta.ptr, custom_input, meta.bytes);
        ET_LOG(Info, "SetInput: copied %zu bytes to input %d", meta.bytes, input_index);
    }

    void MTBExecuTorch::reset_temp_allocator()
    {
        /* temp_allocator_ is always initialized */
        if (temp_allocator_.has_value())
        {
            temp_allocator_->reset();
        }
    }

    Error MTBExecuTorch::reset_all_variables()
    {
        /* Stub implementation - no RNN support */
        return Error::Ok;
    }

    std::optional<EValue> MTBExecuTorch::read_quantization_param(int index, const char *tensor_type, const char *suffix)
    {
        if (!has_quantization_params_)
        {
            return std::nullopt;
        }

        if (!program_.has_value() || !memory_manager_.has_value())
        {
            ET_LOG(Error, "read_quantization_param called before initialization");
            return std::nullopt;
        }

        char method_name[32];
        snprintf(method_name, sizeof(method_name), "%s%d%s", tensor_type, index, suffix);

        Result<Method> method_result = program_->load_method(
            method_name, &memory_manager_.value(), nullptr);

        if (!method_result.ok())
        {
            ET_LOG(Info, "Quantization method %s not found. Assuming FP32 model.", method_name);
            has_quantization_params_ = false;
            return std::nullopt;
        }

        Method quant_method = std::move(method_result.get());
        ET_LOG(Debug, "Loaded method %s", method_name);

        Error status = quant_method.execute();
        if (status != Error::Ok)
        {
            ET_LOG(Error, "Failed to execute %s: status=0x%" PRIx32,
                   method_name, static_cast<uint32_t>(status));
            return std::nullopt;
        }

        if (quant_method.outputs_size() == 0)
        {
            ET_LOG(Error, "Method %s has no outputs", method_name);
            return std::nullopt;
        }

        return std::optional<EValue>(quant_method.get_output(0));
    }

} /* namespace executorch_mtb */
