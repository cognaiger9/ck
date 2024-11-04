#!/bin/bash

# Get values for Mperblock and Nperblock from arguments
MPB=$1
NPB=$2

# Path to the source file where placeholders are defined
SOURCE_FILE="./gemm_tuner.cpp"

# Temporary file to store modified code
TEMP_FILE="temp_gemm_tuner.cpp"

# Replace placeholders with actual values
sed "s/MPB/$MPB/g; s/NPB/$NPB/g" "$SOURCE_FILE" > "$TEMP_FILE"

# Move modified code back to the original file (optional)
# mv "$TEMP_FILE" "$SOURCE_FILE"

# Run cmake and build
bash cmake.sh
cmake temp_gemm_tuner.cpp
./temp_gemm_tuner 1 1 1 8192 6144 1536 1536 6144 6144

# Remove temporary file
#rm "$TEMP_FILE"