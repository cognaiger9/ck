#!/bin/bash
export ROCR_VISIBLE_DEVICES=2

# Fixed path
OUT_TEXT_DIR="../tunning_output"
# Path to the source file where placeholders are defined
SOURCE_FILE="../example/01_gemm/gemm_int8.cpp"

# Variable
M=216064
N=3456
K=1152
TYPE="int8"
PARTITION=6
GPU="mi300"

# Temporary file to store modified code
EXE_FILE="gemm_int8_${PARTITION}"
TEMP_FILE="../example/01_gemm/${EXE_FILE}.cpp"
OUTPUT_FILE="${OUT_TEXT_DIR}/${TYPE}_${M}x${N}x${K}_${GPU}_${PARTITION}.txt"

# Block size = 256 ()
ASP=(16 8 4)
ADB=(16 8 4)
BOOL1=(1 0)
BS=(2 3)
BSP=(8 4)
BDP=(8 4)
BOOL2=(1 0)
CMPS=(2)
CSPS=(16 8)
BK1=(16)

MPB=(256)
NPB=(128)
KPB=(64 32)
RES=(2)
MXPW=(4)
NXPW=(2)

for mpb in "${MPB[@]}"; do
    for npb in "${NPB[@]}"; do
        for kpb in "${KPB[@]}"; do
            for mxpw in "${MXPW[@]}"; do
                for nxpw in "${NXPW[@]}"; do
                    for res1 in "${RES[@]}"; do
                        res2=$((4/res1))
                        mpxdl=$((mpb/mxpw/res1))
                        npxdl=$((npb/nxpw/res2))
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
                                                                sed "s/MPB/$mpb/g; s/NPB/$npb/g; s/KPB/$kpb/g; s/MXPW/$mxpw/g; s/NXPW/$nxpw/g; s/MPXDL/$mpxdl/g; s/NPXDL/$npxdl/g; s/ASP/$asp/g; s/ADB/$adb/g; s/BOOL1/$bool1/g; s/BS1/$bs/g; s/BSP/$bsp/g; s/BDP/$bdp/g; s/BOOL2/$bool2/g; s/CMPS/$cmps/g; s/CSPS/$csps/g; s/BK11/$bk1/g" "$SOURCE_FILE" > "$TEMP_FILE"
                                                                
                                                                if ! make -j4 -C ../build ${EXE_FILE}; then
                                                                    echo "Error: make failed for ${EXE_FILE}" >&2
                                                                    exit 1
                                                                fi
                                                                
                                                                # Run executable
                                                                # row col row
                                                                ../build/bin/$EXE_FILE 1 0 1 ${M} ${N} ${K} ${K} ${K} ${N} >> "${OUTPUT_FILE}"
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
    done
done