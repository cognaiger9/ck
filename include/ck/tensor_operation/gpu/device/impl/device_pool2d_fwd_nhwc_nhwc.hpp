// SPDX-License-Identifier: MIT
// Copyright (c) 2024, Advanced Micro Devices, Inc. All rights reserved.

#pragma once

#include <iostream>
#include <sstream>

#include "ck/tensor_description/tensor_descriptor.hpp"
#include "ck/tensor_description/tensor_descriptor_helper.hpp"
#include "ck/tensor_operation/gpu/device/tensor_layout.hpp"
#include "ck/tensor_operation/gpu/device/reduction_operator_mapping.hpp"
#include "ck/tensor_operation/gpu/device/device_pool_fwd.hpp"
#include "ck/tensor_operation/gpu/device/impl/device_reduce_common.hpp"
#include "ck/tensor_operation/gpu/grid/gridwise_2d_reduction_threadwise.hpp"
#include "ck/host_utility/device_prop.hpp"
#include "ck/host_utility/kernel_launch.hpp"

namespace ck {
namespace tensor_operation {
namespace device {

template <typename InDataType,
          typename OutDataType,
          typename IndexDataType, // enable if OutputIndex == true
          typename ComputeDataType,
          ck::ReduceTensorOp ReduceOpId,
          bool OutputIndex,
          ck::index_t BlockSize,
          ck::index_t MThreadClusterSize,
          ck::index_t KThreadClusterSize,
          ck::index_t MThreadSliceSize,
          ck::index_t KThreadSliceSize,
          ck::index_t InSrcOutDstVectorSize>
struct DevicePool2dFwd_NHWC_NHWC : public DevicePoolFwd<4,
                                                        2,
                                                        InDataType,
                                                        OutDataType,
                                                        IndexDataType,
                                                        tensor_layout::convolution::NHWC,
                                                        tensor_layout::convolution::NHWC,
                                                        ReduceOpId,
                                                        OutputIndex>
{
    static constexpr auto I0 = Number<0>{};
    static constexpr auto I1 = Number<1>{};

    static constexpr index_t InOutRank  = 4;
    static constexpr index_t WindowRank = 2;

    using ReduceOperation = typename reduce_binary_operator<ReduceOpId>::opType;

    using InElementwiseOperation =
        typename reduce_unary_operator<ReduceOpId, true, true>::InElementwiseOperation;

    using AccElementwiseOperation =
        typename reduce_unary_operator<ReduceOpId, true, true>::AccElementwiseOperation;

    static constexpr ck::index_t M_BlockTileSize = MThreadClusterSize * MThreadSliceSize;
    static constexpr ck::index_t K_BlockTileSize = KThreadClusterSize * KThreadSliceSize;

    static auto MakeABGridDescriptor_A_M_K_B_M(std::vector<ck::index_t> input_nchw_lengths,
                                               std::vector<ck::index_t> output_nchw_lengths,
                                               std::vector<ck::index_t> input_nchw_stride,
                                               std::vector<ck::index_t> output_nchw_stride,
                                               std::vector<ck::index_t> window_spatial_yx_lengths,
                                               std::vector<ck::index_t> window_yx_strides,
                                               std::vector<ck::index_t> window_yx_dilations,
                                               std::vector<ck::index_t> input_left_hw_pads,
                                               std::vector<ck::index_t> input_right_hw_pads)
    {
        const index_t N  = input_nchw_lengths[0];
        const index_t C  = input_nchw_lengths[1];
        const index_t Hi = input_nchw_lengths[2];
        const index_t Wi = input_nchw_lengths[3];

        const index_t Ho = output_nchw_lengths[2];
        const index_t Wo = output_nchw_lengths[3];
        const index_t Y  = window_spatial_yx_lengths[0];
        const index_t X  = window_spatial_yx_lengths[1];

        const index_t WindowStrideH = window_yx_strides[0];
        const index_t WindowStrideW = window_yx_strides[1];

        const index_t WindowDilationH = window_yx_dilations[0];
        const index_t WindowDilationW = window_yx_dilations[1];

        const index_t InLeftPadH = input_left_hw_pads[0];
        const index_t InLeftPadW = input_left_hw_pads[1];

        const index_t InRightPadH = input_right_hw_pads[0];
        const index_t InRightPadW = input_right_hw_pads[1];

        const index_t MRaw = N * Ho * Wo * C;
        const index_t MPad = math::integer_least_multiple(MRaw, M_BlockTileSize) - MRaw;

        const index_t KRaw = Y * X;
        const index_t KPad = math::integer_least_multiple(KRaw, K_BlockTileSize) - KRaw;

        // A[ReduceM, ReduceK]
        const index_t Ni_stride = input_nchw_stride[0];
        const index_t Ci_stride = input_nchw_stride[1];
        const index_t Hi_stride = input_nchw_stride[2];
        const index_t Wi_stride = input_nchw_stride[3];

        const auto in_grid_desc_n_hi_wi_c = make_naive_tensor_descriptor(
            make_tuple(N, Hi, Wi, C), make_tuple(Ni_stride, Hi_stride, Wi_stride, Ci_stride));

        const auto in_grid_desc_n_hip_wip_c = transform_tensor_descriptor(
            in_grid_desc_n_hi_wi_c,
            make_tuple(make_pass_through_transform(N),
                       make_pad_transform(Hi, InLeftPadH, InRightPadH),
                       make_pad_transform(Wi, InLeftPadW, InRightPadW),
                       make_pass_through_transform(C)),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}, Sequence<3>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}, Sequence<3>{}));

        const auto in_grid_desc_n_y_ho_x_wo_c = transform_tensor_descriptor(
            in_grid_desc_n_hip_wip_c,
            make_tuple(
                make_pass_through_transform(N),
                make_embed_transform(make_tuple(Y, Ho), make_tuple(WindowDilationH, WindowStrideH)),
                make_embed_transform(make_tuple(X, Wo), make_tuple(WindowDilationW, WindowStrideW)),
                make_pass_through_transform(C)),
            make_tuple(Sequence<0>{}, Sequence<1>{}, Sequence<2>{}, Sequence<3>{}),
            make_tuple(Sequence<0>{}, Sequence<1, 2>{}, Sequence<3, 4>{}, Sequence<5>{}));

        const auto in_grid_desc_reducemraw_reducekraw =
            transform_tensor_descriptor(in_grid_desc_n_y_ho_x_wo_c,
                                        make_tuple(make_merge_transform(make_tuple(N, Ho, Wo, C)),
                                                   make_merge_transform(make_tuple(Y, X))),
                                        make_tuple(Sequence<0, 2, 4, 5>{}, Sequence<1, 3>{}),
                                        make_tuple(Sequence<0>{}, Sequence<1>{}));

        const auto in_grid_desc_reducem_reducek = transform_tensor_descriptor(
            in_grid_desc_reducemraw_reducekraw,
            make_tuple(make_right_pad_transform(MRaw, MPad), make_right_pad_transform(KRaw, KPad)),
            make_tuple(Sequence<0>{}, Sequence<1>{}),
            make_tuple(Sequence<0>{}, Sequence<1>{}));

        // B[ReduceM]
        const index_t No_stride = output_nchw_stride[0];
        const index_t Co_stride = output_nchw_stride[1];
        const index_t Ho_stride = output_nchw_stride[2];
        const index_t Wo_stride = output_nchw_stride[3];

        const auto out_grid_desc_n_ho_wo_c = make_naive_tensor_descriptor(
            make_tuple(N, Hi, Wi, C), make_tuple(No_stride, Ho_stride, Wo_stride, Co_stride));

        const auto out_grid_desc_reducemraw =
            transform_tensor_descriptor(out_grid_desc_n_ho_wo_c,
                                        make_tuple(make_merge_transform(make_tuple(N, Ho, Wo, C))),
                                        make_tuple(Sequence<0, 1, 2, 3>{}),
                                        make_tuple(Sequence<0>{}));

        const auto out_grid_desc_reducem =
            transform_tensor_descriptor(out_grid_desc_reducemraw,
                                        make_tuple(make_right_pad_transform(MRaw, MPad)),
                                        make_tuple(Sequence<0>{}),
                                        make_tuple(Sequence<0>{}));

        return make_tuple(in_grid_desc_reducem_reducek, out_grid_desc_reducem);
    }

    using ABGridDescs =
        decltype(MakeABGridDescriptor_A_M_K_B_M({}, {}, {}, {}, {}, {}, {}, {}, {}));

    using AGridDesc_M_K = remove_cvref_t<decltype(ABGridDescs{}[I0])>;
    using BGridDesc_M   = remove_cvref_t<decltype(ABGridDescs{}[I1])>;

    struct Argument : public BaseArgument
    {
        Argument(const InDataType* p_in_dev,
                 OutDataType* p_out_dev,
                 IndexDataType* p_out_indices_dev,
                 std::vector<ck::index_t>& input_nchw_lengths,
                 std::vector<ck::index_t>& output_nchw_lengths,
                 std::vector<ck::index_t>& input_nchw_stride,
                 std::vector<ck::index_t>& output_nchw_stride,
                 std::vector<ck::index_t>&, // indices_nchw_stride
                 std::vector<ck::index_t>& window_spatial_yx_lengths,
                 std::vector<ck::index_t>& window_yx_strides,
                 std::vector<ck::index_t>& window_yx_dilations,
                 std::vector<ck::index_t>& input_left_hw_pads,
                 std::vector<ck::index_t>& input_right_hw_pads)
            : p_in_dev_{p_in_dev},
              p_out_dev_{p_out_dev},
              p_out_indices_dev_{p_out_indices_dev},
              a_grid_desc_m_k_{},
              b_grid_desc_m_{},
              input_nchw_lengths_{input_nchw_lengths},
              output_nchw_lengths_{output_nchw_lengths},
              input_nchw_stride_{input_nchw_stride},
              output_nchw_stride_{output_nchw_stride}
        {
            const auto descs = MakeABGridDescriptor_A_M_K_B_M(input_nchw_lengths,
                                                              output_nchw_lengths,
                                                              input_nchw_stride,
                                                              output_nchw_stride,
                                                              window_spatial_yx_lengths,
                                                              window_yx_strides,
                                                              window_yx_dilations,
                                                              input_left_hw_pads,
                                                              input_right_hw_pads);

            a_grid_desc_m_k_ = descs[I0];
            b_grid_desc_m_   = descs[I1];

            int32_t reduceLength = window_spatial_yx_lengths[0] * window_spatial_yx_lengths[1];

            std::tie(in_element_op_, acc_element_op_) =
                reduce_unary_operator<ReduceOpId, true, true>::GetElementwiseOperator(reduceLength);
        }

        const InDataType* p_in_dev_;
        OutDataType* p_out_dev_;
        IndexDataType* p_out_indices_dev_;
        AGridDesc_M_K a_grid_desc_m_k_;
        BGridDesc_M b_grid_desc_m_;

        InElementwiseOperation in_element_op_;
        AccElementwiseOperation acc_element_op_;

        // for checking vector load/store
        std::vector<ck::index_t> input_nchw_lengths_;
        std::vector<ck::index_t> output_nchw_lengths_;
        std::vector<ck::index_t> input_nchw_stride_;
        std::vector<ck::index_t> output_nchw_stride_;
    };

    struct Invoker : public BaseInvoker
    {
        float Run(const Argument& arg, const StreamConfig& stream_config = StreamConfig{})
        {
            // for NHWC, the dim C is the fastest dimension, and is not reduced.
            // Hence, it is in M dimension for reduction kernel.
            static constexpr index_t InSrcOutDstVectorDim = 0; // 0: M, 1: K

            using gridwise_reduce =
                GridwiseReduction_mk_to_m_threadwise<InDataType,
                                                     OutDataType,
                                                     ComputeDataType,
                                                     IndexDataType,
                                                     AGridDesc_M_K,
                                                     BGridDesc_M,
                                                     ReduceOperation,
                                                     InElementwiseOperation,
                                                     AccElementwiseOperation,
                                                     InMemoryDataOperationEnum::Set,
                                                     false, // propagate_nan
                                                     BlockSize,
                                                     MThreadSliceSize,
                                                     KThreadSliceSize,
                                                     InSrcOutDstVectorDim,
                                                     InSrcOutDstVectorSize,
                                                     InSrcOutDstVectorSize>;

            const auto kernel =
                kernel_reduce_threadwise<gridwise_reduce,
                                         OutputIndex,
                                         true,  // pooling need to return global index
                                         false, // don't have index input
                                         InDataType,
                                         OutDataType,
                                         ComputeDataType,
                                         IndexDataType,
                                         AGridDesc_M_K,
                                         BGridDesc_M,
                                         InElementwiseOperation,
                                         AccElementwiseOperation>;

            ck::index_t M = arg.a_grid_desc_m_k_.GetLength(I0);

            const index_t grid_size = (M / M_BlockTileSize);

            return launch_and_time_kernel(stream_config,
                                          kernel,
                                          dim3(grid_size),
                                          dim3(BlockSize),
                                          0,
                                          arg.a_grid_desc_m_k_,
                                          arg.b_grid_desc_m_,
                                          arg.in_element_op_,
                                          arg.acc_element_op_,
                                          float(1),
                                          arg.p_in_dev_,
                                          nullptr,
                                          float(0),
                                          arg.p_out_dev_,
                                          arg.p_out_indices_dev_);
        }

        float Run(const BaseArgument* p_arg,
                  const StreamConfig& stream_config = StreamConfig{}) override
        {
            return Run(*dynamic_cast<const Argument*>(p_arg), stream_config);
        }
    };

    bool IsSupportedArgument(const BaseArgument* p_arg) override
    {
        const Argument* pArg = dynamic_cast<const Argument*>(p_arg);

        // C should be fastest dimension
        if(pArg->input_nchw_stride_[1] != 1)
            return false;

        for(int i = 0; i < InOutRank; ++i)
        {
            if(pArg->input_nchw_stride_[i] == 1 &&
               pArg->input_nchw_lengths_[i] % InSrcOutDstVectorSize != 0)
                return false;

            if(pArg->output_nchw_stride_[i] == 1 &&
               pArg->output_nchw_lengths_[i] % InSrcOutDstVectorSize != 0)
                return false;
        }

        return true;
    }

    virtual std::unique_ptr<BaseArgument>
    MakeArgumentPointer(const void* p_in_dev,
                        void* p_out_dev,
                        void* p_out_indices_dev,
                        std::vector<ck::index_t> input_nchw_lengths,
                        std::vector<ck::index_t> window_yx_lengths,
                        std::vector<ck::index_t> output_nchw_lengths,
                        std::vector<ck::index_t> input_nchw_stride,
                        std::vector<ck::index_t> output_nchw_stride,
                        std::vector<ck::index_t> indices_nchw_stride,
                        std::vector<ck::index_t> window_yx_strides,
                        std::vector<ck::index_t> window_yx_dilations,
                        std::vector<ck::index_t> input_left_hw_pads,
                        std::vector<ck::index_t> input_right_hw_pads,
                        std::vector<ck::index_t> pooling_dims) override
    {
        if(input_nchw_lengths.size() != InOutRank || window_yx_lengths.size() != WindowRank ||
           input_nchw_lengths.size() != InOutRank || window_yx_strides.size() != WindowRank ||
           window_yx_dilations.size() != WindowRank || input_left_hw_pads.size() != WindowRank ||
           input_right_hw_pads.size() != WindowRank)
            throw std::runtime_error("dimension is incorrect");

        if(pooling_dims != std::vector<ck::index_t>{2, 3})
            throw std::runtime_error("pooling_dims only support {2, 3} in pool2d so far");

        if(output_nchw_stride != indices_nchw_stride)
            throw std::runtime_error(
                "output_nchw_stride need to be equal to indices_nchw_stride for now");

        return std::make_unique<Argument>(static_cast<const InDataType*>(p_in_dev),
                                          static_cast<OutDataType*>(p_out_dev),
                                          static_cast<IndexDataType*>(p_out_indices_dev),
                                          input_nchw_lengths,
                                          output_nchw_lengths,
                                          input_nchw_stride,
                                          output_nchw_stride,
                                          indices_nchw_stride,
                                          window_yx_lengths,
                                          window_yx_strides,
                                          window_yx_dilations,
                                          input_left_hw_pads,
                                          input_right_hw_pads);
    }

    std::unique_ptr<BaseInvoker> MakeInvokerPointer() override
    {
        return std::make_unique<Invoker>(Invoker{});
    }

    std::string GetTypeString() const override
    {
        auto str = std::stringstream();

        // clang-format off
        str << "DevicePool2dFwd_NHWC_NHWC<" << BlockSize << ",";
        str << "M_C" << MThreadClusterSize << "_S" << MThreadSliceSize << ",";
        str << "K_C" << KThreadClusterSize << "_S" << KThreadSliceSize << ",";
        str <<"InSrcOutDstVectorSize_" << InSrcOutDstVectorSize << ">";
        // clang-format on

        return str.str();
    }
};

} // namespace device
} // namespace tensor_operation
} // namespace ck
