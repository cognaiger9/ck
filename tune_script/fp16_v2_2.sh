#!/bin/bash
export ROCR_VISIBLE_DEVICES=5

# Fixed path
OUT_TEXT_DIR="../tunning_output"
# Path to the source file where placeholders are defined
SOURCE_FILE="../example/01_gemm/fp16_v2.cpp"

# Variable
M=216000
N=9216
K=1152
TYPE="fp16"
ALGO="v2"
PARTITION=2
GPU="mi300"

# Temporary file to store modified code
EXE_FILE="fp16_v2_${PARTITION}"
TEMP_FILE="../example/01_gemm/${EXE_FILE}.cpp"
OUTPUT_FILE="${OUT_TEXT_DIR}/${TYPE}_${M}x${N}x${K}_${GPU}_${ALGO}_${PARTITION}.txt"

# Block size = 256 ()
PRE=(2 1)
ASP=(8 4)
ADB=(8 4)
BOOL1=(0 1)
BS=(1 2)
BSP=(8 4 2)
BDP=(4 2)
BOOL2=(0 1)
CMPS=(1 2)
CSPS=(8 4)
BK1=(4)

MPB=(128)
NPB=(256 128)
KPB=(32 64)
mpxdl=32
npxdl=32

for mpb in "${MPB[@]}"; do
    for npb in "${NPB[@]}"; do
        for kpb in "${KPB[@]}"; do
            for pre in "${PRE[@]}"; do
                mxpw=$((mpb/mpxdl/2))
                nxpw=$((npb/npxdl/2))
                for asp in "${ASP[@]}"; do
                    for adb in "${ADB[@]}"; do
                        for bool1 in "${BOOL1[@]}"; do
                            for bs in "${BS[@]}"; do
                                for bsp in "${BSP[@]}"; do
                                    for bdp in "${BDP[@]}"; do
                                        for bool2 in "${BOOL2[@]}"; do
                                            for cmps in "${CMPS[@]}"; do
                                                for csps in "${CSPS[@]}"; do
                                                    for bk1 in "${BK1[@]}"; do
                                                        # Substitute the placeholders in the source file
                                                        sed "s/PRE/$pre/g; s/MPB/$mpb/g; s/NPB/$npb/g; s/KPB/$kpb/g; s/MXPW/$mxpw/g; s/NXPW/$nxpw/g; s/MPXDL/$mpxdl/g; s/NPXDL/$npxdl/g; s/ASP/$asp/g; s/ADB/$adb/g; s/BOOL1/$bool1/g; s/BS1/$bs/g; s/BSP/$bsp/g; s/BDP/$bdp/g; s/BOOL2/$bool2/g; s/CMPS/$cmps/g; s/CSPS/$csps/g; s/BK11/$bk1/g" "$SOURCE_FILE" > "$TEMP_FILE"
                                                        if ! make -j24 -C ../build ${EXE_FILE}; then
                                                            echo "Error: make failed for ${EXE_FILE}" >&2
                                                        fi
                                                        # Run executable
                                                        # row row row
                                                        ../build/bin/$EXE_FILE 1 2 1 ${M} ${N} ${K} ${K} ${N} ${N} >> "${OUTPUT_FILE}"
                                                        rm ../build/bin/$EXE_FILE
                                                    done
                                                done
                                            done
                                        done
                                    done
                                done
                            done
                        done
                    done
                done
            done
        done
    done
done