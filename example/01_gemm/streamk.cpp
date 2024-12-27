// SPDX-License-Identifier: MIT
// Copyright (c) 2018-2024, Advanced Micro Devices, Inc. All rights reserved.

#include "common.hpp"

#include "ck/tensor_operation/gpu/device/impl/device_gemm_xdl_streamk.hpp"

using ADataType        = ck::half_t;
using BDataType        = ck::half_t;
using AccDataType      = float;
using CShuffleDataType = float;
using CDataType        = ck::half_t;

using F16 = ck::half_t;

using ALayout = Row;
using BLayout = Row;
// using BLayout = Col;
using CLayout = Row;

using AElementOp = PassThrough;
using BElementOp = PassThrough;
using CElementOp = PassThrough;

// clang-format off
using DeviceGemmStreamK = ck::tensor_operation::device::DeviceGemmXdlStreamK
// ######|     AData|     BData|     CData|     AccData| ALayout| BLayout| CLayout|           A|           B|           C|  Block|  MPer|  NPer| K0Per| K1| MPer| NPer| MXdl| NXdl|  ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockTransfer| ABlockLds|  BBlockTransfer| BBlockTransfer| BBlockTransfer| BlockTransfer| BBlockTransfer| BBlockTransfer| BBlockLds|    CShuffle|    CShuffle| CBlockTransferClusterLengths|  CBlockTransfer|
// ######|      Type|      Type|      Type|        Type|        |        |        | Elementwise| Elementwise| Elementwise|   Size| Block| Block| Block|   |  XDL|  XDL|  Per|  Per|   ThreadCluster|  ThreadCluster| SrcAccessOrder|   SrcVectorDim|      SrcScalar|      DstScalar| AddExtraM|   ThreadCluster|  ThreadCluster| SrcAccessOrder|  SrcVectorDim|      SrcScalar|      DstScalar| AddExtraN| MXdlPerWave| NXdlPerWave|         _MBlock_MWaveMPerXdl| ScalarPerVector|
// ######|          |          |          |            |        |        |        |   Operation|   Operation|   Operation|       |      |      |      |   |     |     | Wave| Wave| Lengths_K0_M_K1|   ArrangeOrder|               |               |      PerVector|   PerVector_K1|          | Lengths_K0_N_K1|   ArrangeOrder|               |              |      PerVector|   PerVector_K1|          |  PerShuffle|  PerShuffle|         _NBlock_NWaveNPerXdl|   _NWaveNPerXdl|
// ######|          |          |          |            |        |        |        |            |            |            |       |      |      |      |   |     |     |     |     |                |               |               |               |               |               |          |                |               |               |              |               |               |          |            |            |                             |                |
    < ADataType, BDataType, CDataType, AccDataType, ALayout, BLayout, CLayout,  AElementOp,  BElementOp,  CElementOp,         256,   MPB,   NPB,     4,  8,MPXDL,NPXDL, MXPW, NXPW,     S<4, 64, 1>,     S<1, 0, 2>,     S<1, 0, 2>,              2,            ASP,            ADB,     BOOL1,     S<4, 64, 1>,      S<0, 2, 1>,     S<0, 2, 1>,          BS1,            BSP,            BDP,     BOOL2,        CMPS,           1,               S<1, 32, 1, 8>,            CSPS>;

          // < ADataType, BDataType, CDataType, AccDataType, ALayout, BLayout, CLayout,  AElementOp,  BElementOp,  CElementOp,    256,   256,   128,     4,   8,  32,   32,    4,    2,     S<4, 64, 1>,     S<1, 0, 2>,     S<1, 0, 2>,              2,              8,              8,         1,     S<4, 64, 1>,     S<0, 2, 1>,     S<0, 2, 1>,             1,              2,              2,         1,           1,           1,               S<1, 32, 1, 8>,              8>;
      //  < ADataType, BDataType, CDataType, AccDataType, ALayout, BLayout, CLayout,  AElementOp,  BElementOp,  CElementOp,    128,   32,   64,     4,   8,  32,   32,    1,    1,     S<4, 32, 1>,     S<1, 0, 2>,     S<1, 0, 2>,              2,              8,              8,         1,     S<4, 32, 1>,     S<0, 2, 1>,     S<0, 2, 1>,             1,              2,              8,         1,           1,           1,               S<1, 16, 1, 8>,              8>;
       // < ADataType, BDataType, CDataType, AccDataType, ALayout, BLayout, CLayout,  AElementOp,  BElementOp,  CElementOp,    128,   32,   128,     4,   8,  32,   32,    1,    1,     S<8, 16, 1>,     S<1, 0, 2>,     S<1, 0, 2>,              2,              8,              8,         1,     S<8, 16, 1>,     S<0, 2, 1>,     S<0, 2, 1>,             1,              2,              2,         1,           1,           1,               S<1, 32, 1, 4>,              8>;



// // clang-format on
// clang-format on

using DeviceGemmInstance = DeviceGemmStreamK;

using ReferenceGemmInstance = ck::tensor_operation::host::
    ReferenceGemm<ADataType, BDataType, CDataType, AccDataType, AElementOp, BElementOp, CElementOp>;

using ReferenceGemmInstanceGPU = ck::tensor_operation::device::ReferenceGemm<ALayout,
                                                                             BLayout,
                                                                             CLayout,
                                                                             ADataType,
                                                                             BDataType,
                                                                             CDataType,
                                                                             AccDataType,
                                                                             AElementOp,
                                                                             BElementOp,
                                                                             CElementOp>;

#include "run_gemm_example.inc"

int main(int argc, char* argv[]) {
    bool res = run_gemm_streamk_example(argc, argv);
    std::cout << res << std::endl;
    return 0; 
}