#pragma once

#include <functional>

#include "Metal/MTLCommandQueue.hpp"
#include "Metal/MTLComputePipeline.hpp"
#include "Metal/MTLLibrary.hpp"
#include "internal/enum_impl.h"

// forward declaration
class Tensor;

namespace metal
{
constexpr int X_THREADS_PER_GROUP = 8;
constexpr int Y_THREADS_PER_GROUP = 8;

LEGRAD_ENUM(Ops, uint8_t, ADD, RELU, ADD, MUL, RELU, COUNT);

struct KernelContext
{
  void* data;
  int length;
  std::function<void(void*, int)> deleter;
};

struct Kernel
{
  MTL::Function* func;
  MTL::ComputePipelineState* pipeline;

  Kernel();
  virtual ~Kernel() {};

  virtual void run(MTL::CommandQueue* queue,
                   Tensor inputs[],
                   Tensor outputs[],
                   size_t input_size,
                   size_t output_size,
                   KernelContext context = KernelContext()) = 0;
};

struct Kernel1D : public Kernel
{
  ~Kernel1D() {};

  void run(MTL::CommandQueue* queue,
           Tensor inputs[],
           Tensor outputs[],
           size_t input_size,
           size_t output_size,
           KernelContext context = KernelContext()) override;
};
}  // namespace metal