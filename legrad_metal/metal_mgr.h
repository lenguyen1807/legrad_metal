#pragma once

#include <memory>
#include <mutex>

#include "Metal/MTLCommandQueue.hpp"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLLibrary.hpp"
#include "core/allocator.h"
#include "internal/pattern.h"

class MetalMgr : public internal::Singleton<MetalMgr>
{
public:
  MetalMgr();  // can throw
  ~MetalMgr();

  const core::Allocator& allocator() const { return *allocator_; }
  core::Allocator& allocator() { return *allocator_; }

  MTL::Device* device() const { return device_; }
  MTL::Library* library() const { return library_; }

private:
  MTL::Device* device_;
  MTL::CommandQueue* cmd_queue_;
  MTL::Library* library_;
  std::mutex mtx_;

  // allocator for buffer
  std::unique_ptr<core::Allocator> allocator_;
};