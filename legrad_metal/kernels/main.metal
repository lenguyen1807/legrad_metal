#include <metal_stdlib>

using namespace metal;

// https://stackoverflow.com/questions/56687496/how-to-make-templated-compute-kernels-in-metal
template<typename T>
kernel void add(
        device const T * src0,
        device const T * src1,
        device T * dst,
        uint id [[thread_position_in_grid]]) {
    dst[id] = src0[id] + src1[id];
}

template [[ host_name("add_float") ]] 
kernel void add(device const float*, device const float*, device float*, uint);

template<typename T>
kernel void mul(
        device const T * src0,
        device const T * src1,
        device T * dst,
        uint id [[thread_position_in_grid]]) {
    dst[id] = src0[id] * src1[id];
}

template [[ host_name("mul_float") ]] 
kernel void mul(device const float*, device const float*, device float*, uint);