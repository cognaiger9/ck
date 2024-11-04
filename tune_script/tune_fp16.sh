#!/bin/bash
# Path to the source file where placeholders are defined
SOURCE_FILE="../example/01_gemm/gemm_fp16.cpp"

# Temporary file to store modified code
TEMP_FILE="../example/01_gemm/temp_gemm_fp16_4.cpp"

# Block size = 256 ()
PRE=(1 2)
ASP=(2 4 8)
ADB=(2 4 8)
BOOL1=(0 1)
BS=(1 3)
BSP=(2 4)
BDP=(2 4)
BOOL2=(0 1)
CMPS=(1 2)
CSPS=(2 4 8)
BK1=(2 4)

# (M, N, K) = (8192, 6144, 1536) then (MPB, NPB) = (64, 64), (128, 128), (256, 128) valid on MI250, (256, 256) + (128, 256) + (64, 256) wrong results, other invalid mfma instructions
# (M, N, K) = (8192, 1536, 6144) then (64, 64) + (128, 128) + (256, 128); (128, 256) + (256, 256) + (64, 256) wrong
# (M, N, K) = (8192, 1536, 1536) then (64, 64) + (128, 128) + (256, 128)
MPB=(128)
NPB=(256)
KPB=(32 64)
RES=(2)  # can be (1 2 4) but fix now for quick res
MXPW=(2 4)
NXPW=(2 4)

# Loop through each combination of BS, MPB, NPB, and KPB
#for bs in "${BS[@]}"; do
#    for mpb in "${MPB[@]}"; do
#        for npb in "${NPB[@]}"; do
#            for kpb in "${KPB[@]}"; do
#                # Substitute the placeholders in the source file
#                sed "s/BS/$bs/g; s/MPB/$mpb/g; s/NPB/$npb/g; s/KPB/$kpb/g" "$SOURCE_FILE" > "$TEMP_FILE"
#                
#                # Optional: Add code here to compile or execute the modified file
#                # Example:
#                # g++ -o gemm_tuner "$TEMP_FILE" && ./gemm_tuner
#                make -j24 temp_gemm_fp16
#                ./temp_gemm_fp16 1 1 1 8192 6144 1536 1536 6144 6144 >> output_fp16.txt
#                echo "Ran with BS=$bs, MPB=$mpb, NPB=$npb, KPB=$kpb"
#            done
#        done
#    done
#done

for mpb in "${MPB[@]}"; do
    for npb in "${NPB[@]}"; do
        for kpb in "${KPB[@]}"; do
            for mxpw in "${MXPW[@]}"; do
                for nxpw in "${NXPW[@]}"; do
                    for res1 in "${RES[@]}"; do
                        res2=$((4/res1))
                        mpxdl=$((mpb/mxpw/res1))
                        npxdl=$((npb/nxpw/res1))
                        for pre in "${PRE[@]}"; do
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
                                                                    sed "s/MPB/$mpb/g; s/NPB/$npb/g; s/KPB/$kpb/g; s/MXPW/$mxpw/g; s/NXPW/$nxpw/g; s/MPXDL/$mpxdl/g; s/NPXDL/$npxdl/g; s/PRE/$pre/g; s/ASP/$asp/g; s/ADB/$adb/g; s/BOOL1/$bool1/g; s/BS1/$bs/g; s/BSP/$bsp/g; s/BDP/$bdp/g; s/BOOL2/$bool2/g; s/CMPS/$cmps/g; s/CSPS/$csps/g; s/BK11/$bk1/g" "$SOURCE_FILE" > "$TEMP_FILE"

                                                                    make -j4 temp_gemm_fp16_4
                                                                    ./bin/temp_gemm_fp16_4 1 0 1 8192 1536 6144 6144 1536 1536 >> output_fp16_4_2.txt
                                                                    rm ./bin/temp_gemm_fp16_4
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
done
