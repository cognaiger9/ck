#!/bin/bash
# Path to the source file where placeholders are defined
SOURCE_FILE="../example/01_gemm/gemm_fp16.cpp"

# Temporary file to store modified code
TEMP_FILE="../example/01_gemm/temp_gemm_fp16.cpp"

# Arrays for various parameters
PRE=(1 2 3 4 5 6 7 8 9 10)
BS=(64 128 256 512)
MPB=(128 256 512)
NPB=(128 256 512)
KPB=(32 64 128 256)

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

for pre in "${PRE[@]}"; do
    # Substitute the placeholders in the source file
    sed "s/PRE/$pre/g" "$SOURCE_FILE" > "$TEMP_FILE"

    # Optional: Add code here to compile or execute the modified file
    # Example:
    # g++ -o gemm_tuner "$TEMP_FILE" && ./gemm_tuner
    make -j24 temp_gemm_fp16
    ./temp_gemm_fp16 1 1 1 8192 6144 1536 1536 6144 6144 >> output_fp16.txt
    rm ./temp_gemm_fp16
done
