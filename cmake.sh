cmake                                                                                             \
-D CMAKE_PREFIX_PATH=/opt/rocm                                                                    \
-D CMAKE_CXX_COMPILER=/opt/rocm/bin/hipcc                                                         \
-D CMAKE_BUILD_TYPE=Release                                                                       \
-D GPU_TARGETS="gfx90a"                                                                    \
..
