#include "Foundation/NSAutoreleasePool.hpp"
#include "Metal/MTLCommandBuffer.hpp"
#include "Metal/MTLComputeCommandEncoder.hpp"
#include "metal_function.h"
#include "metal_mgr.h"

namespace metal
{
Kernel::Kernel()
    : func(nullptr)
    , pipeline(nullptr)
{
}

void Kernel1D::run(MTL::CommandQueue* queue,
                   Tensor inputs[],
                   Tensor outputs[],
                   size_t input_size,
                   size_t output_size,
                   KernelContext context)
{
  // create memory pool for auto clean resources
  NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

  // setup
  MTL::CommandBuffer* command_buffer = queue->commandBuffer();
  assert(command_buffer != nullptr);

  MTL::ComputeCommandEncoder* compute_encoder =
      command_buffer->computeCommandEncoder();
  assert(compute_encoder != nullptr);

  compute_encoder->setComputePipelineState(pipeline);

  // // set inputs to compute encoder
  // for (size_t i = 0; i < input_size; i++) {
  //   compute_encoder->setBuffer(inputs[i].buffer(), 0, i);
  // }

  // // set outputs to compute encoder
  // for (size_t i = input_size; i < input_size + output_size; i++) {
  //   compute_encoder->setBuffer(outputs[i].buffer(), 0, i);
  // }

  // finally set the additional context
  if (context.data != nullptr) {
    // we create a temp buffer
    // and this temp buffer should take the resource (data)
    auto device = MetalMgr::instance().device();

    // first we create a deleter for this buffer
    void (^deallocator)(void*, NS::UInteger) = ^(void* ptr, NS::UInteger size) {
      context.deleter(ptr, static_cast<int>(size));
    };

    // then create the buffer
    MTL::Buffer* ctx_buf =
        device->newBuffer(context.data, context.length, 0, deallocator);

    // pass buffer as context to encoder
    compute_encoder->setBuffer(ctx_buf, 0, input_size + output_size);

    // note that this buffer should be deleted by auto release pool
  }

  // https://developer.apple.com/documentation/metal/performing-calculations-on-a-gpu?language=objc
  // we assume that all tensor has same size
  // int arr_length = inputs[0].numel();
  // auto grid_size = MTL::Size::Make(arr_length, 1, 1);
  // int max_thread_size = pipeline->maxTotalThreadsPerThreadgroup();
  // if (max_thread_size > arr_length) {
  //   max_thread_size = arr_length;
  // }
  // auto thread_group_size = MTL::Size::Make(max_thread_size, 1, 1);
  // compute_encoder->dispatchThreadgroups(grid_size, thread_group_size);

  // End the encoding setup
  compute_encoder->endEncoding();

  // Start the kernel
  command_buffer->commit();
  command_buffer->waitUntilCompleted();

  // Clean resources
  pool->release();
}
};  // namespace metal