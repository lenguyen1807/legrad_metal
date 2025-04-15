#pragma once

#include <memory>
#include <mutex>

#include "Metal/MTLCommandQueue.hpp"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLLibrary.hpp"
#include "backend/metal_dispatcher.h"
#include "core/allocator.h"
#include "internal/pattern.h"

class MetalMgr : public internal::Singleton<MetalMgr>
{
public:
  MetalMgr();  // can throw
  ~MetalMgr();

  const core::MetalAllocator& allocator() const { return *allocator_; }
  core::MetalAllocator& allocator() { return *allocator_; }

  MTL::Device* device() const { return device_; }
  MTL::Library* library() const { return library_; }

  const metal::KernelDispatcher& dispatcher() const { return *dispatcher_; }
  metal::KernelDispatcher& dispatcher() { return *dispatcher_; }

private:
  MTL::Device* device_;
  MTL::CommandQueue* cmd_queue_;
  MTL::Library* library_;
  std::mutex mtx_;

  // allocator for buffer
  std::unique_ptr<core::MetalAllocator> allocator_;
  std::unique_ptr<metal::KernelDispatcher> dispatcher_;
};