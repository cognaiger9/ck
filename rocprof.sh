M=216000
N=4608
K=2304
LAYOUT=RRR
TYPE=fp16
if [[ "$TYPE" == "int8" && "$LAYOUT" == "RCR" ]]; then
    rocprofv2 -d rocprof -o ${M}_${N}_${K}_${TYPE}_${LAYOUT} ./build/bin/example_gemm_xdl_int8 1 1 1 ${M} ${N} ${K} ${K} ${K} ${N}
elif [[ "$TYPE" == "int8" && "$LAYOUT" == "RRR" ]]; then
    rocprofv2 -d rocprof -o ${M}_${N}_${K}_${TYPE}_${LAYOUT} ./build/bin/gemm_int8_rrr 1 1 1 ${M} ${N} ${K} ${K} ${N} ${N}
elif [[ "$TYPE" == "fp16" && "$LAYOUT" == "RRR" ]]; then
    rocprofv2 -d rocprof -o ${M}_${N}_${K}_${TYPE}_${LAYOUT} ./build/bin/example_gemm_xdl_fp16_v2 1 2 1 ${M} ${N} ${K} ${K} ${N} ${N}
elif [[ "$TYPE" == "fp16" && "$LAYOUT" == "RCR" ]]; then
    rocprofv2 -d rocprof -o ${M}_${N}_${K}_${TYPE}_${LAYOUT} ./build/bin/gemm_fp16_rcr 1 2 1 ${M} ${N} ${K} ${K} ${K} ${N}
elif [[ "$TYPE" == "fp8" && "$LAYOUT" == "RCR" ]]; then
    rocprofv2 -d rocprof -o ${M}_${N}_${K}_${TYPE}_${LAYOUT} ./build/bin/example_gemm_xdl_fp8 1 2 1 ${M} ${N} ${K} ${K} ${K} ${N}
elif [[ "$TYPE" == "fp8" && "$LAYOUT" == "RRR" ]]; then
    rocprofv2 -d rocprof -o ${M}_${N}_${K}_${TYPE}_${LAYOUT} ./build/bin/gemm_fp8_rrr 1 2 1 ${M} ${N} ${K} ${K} ${N} ${N} 
fi
