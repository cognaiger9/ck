#!/bin/bash
export ROCR_VISIBLE_DEVICES=2

# Fixed path
OUT_TEXT_DIR="../tunning_output"
# Path to the source file where placeholders are defined
SOURCE_FILE="../example/01_gemm/streamk.cpp"

# Variable
M=8192
N=1536
K=6144
TYPE="fp16"
ALGO="streamk"
PARTITION=1
GPU="mi300"

# Temporary file to store modified code
EXE_FILE="streamk_${PARTITION}"
TEMP_FILE="../example/01_gemm/${EXE_FILE}.cpp"
OUTPUT_FILE="${OUT_TEXT_DIR}/${TYPE}_${M}x${N}x${K}_${GPU}_${PARTITION}_${ALGO}.txt"

# Block size = 256 ()
ASP=(8 4 2)
ADB=(8 4 2)
BOOL1=(1 0)
BS=(1)
BSP=(2 4)
BDP=(8 2 4)
BOOL2=(1 0)
CMPS=(1)
CSPS=(8 4 2)
BK1=(2 4)

MPB=(128 256)
NPB=(128 256)
mpxdl=32
npxdl=32

for mpb in "${MPB[@]}"; do
    for npb in "${NPB[@]}"; do
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
                                                    # Substitute the placeholders in the source file
                                                    sed "s/MPB/$mpb/g; s/NPB/$npb/g; s/MXPW/$mxpw/g; s/NXPW/$nxpw/g; s/MPXDL/$mpxdl/g; s/NPXDL/$npxdl/g; s/ASP/$asp/g; s/ADB/$adb/g; s/BOOL1/$bool1/g; s/BS1/$bs/g; s/BSP/$bsp/g; s/BDP/$bdp/g; s/BOOL2/$bool2/g; s/CMPS/$cmps/g; s/CSPS/$csps/g" "$SOURCE_FILE" > "$TEMP_FILE"
                                                    
                                                    if ! make -j16 -C ../build ${EXE_FILE}; then
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