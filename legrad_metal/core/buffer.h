#pragma once

#include <cstddef>
#include <cstdlib>

#include "Metal/MTLBuffer.hpp"
#include "core/allocator.h"
#include "macros/log.h"
#include "metal_mgr.h"

namespace core
{
class MetalBuffer
{
public:
  // Buffer will manage this resources
  MetalBuffer(MTL::Buffer* buffer, size_t nbytes)
      : ptr_(buffer)
      , size_(nbytes)
  {
    LEGRAD_LOG_DEBUG("Create MetalBuffer with size: {} and pointer: {}", nbytes,
                     ptr_->contents());
  }

  ~MetalBuffer()
  {
    LEGRAD_LOG_DEBUG("Free MetalBuffer with size: {} and pointer: {}", size_,
                     ptr_->contents());
    free_buffer();
  }

  // We want to avoid Buffer copy
  // This will create some unknown behaviours
  MetalBuffer(const MetalBuffer&) = delete;
  MetalBuffer& operator=(const MetalBuffer&) = delete;

  // But we can move Buffer
  MetalBuffer(MetalBuffer&& buf) noexcept
      : ptr_(buf.ptr_)
      , size_(buf.size_)
  {
    buf.size_ = 0;
    buf.ptr_ = nullptr;
  }

  MetalBuffer& operator=(MetalBuffer&& buf) noexcept
  {
    if (this == &buf) {
      return *this;
    }

    // delete resource of current buffer first
    free_buffer();
    // move resource to current buffer
    ptr_ = buf.ptr_;
    size_ = buf.size_;
    // set other to default
    buf.ptr_ = nullptr;
    buf.size_ = 0;

    return *this;
  }

  MTL::Buffer* ptr() { return ptr_; }
  const MTL::Buffer* ptr() const { return ptr_; }

  size_t size() const { return size_; }

private:
  void free_buffer()
  {
    // call allocator to delete this buffer
    auto& allocator = MetalMgr::instance().allocator();
    allocator.free(this);

    // assign to null and zero
    ptr_ = nullptr;
    size_ = 0;
  }

private:
  MTL::Buffer* ptr_ = nullptr;
  size_t size_ = 0;
};
};  // namespace core