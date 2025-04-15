#pragma once

#include <cstddef>
#include <cstdlib>

#include "Metal/MTLBuffer.hpp"
#include "core/allocator.h"
#include "metal_mgr.h"

namespace core
{
class Buffer
{
public:
  Buffer() = default;

  // Buffer will manage this resources
  Buffer(MTL::Buffer* buffer, size_t nbytes)
      : ptr_(buffer)
      , size_(nbytes)
  {
  }

  ~Buffer()
  {
    // call allocator to delete this buffer
    auto& allocator = MetalMgr::instance().allocator();
    allocator.free(this);

    // set to default value
    ptr_ = nullptr;
    size_ = 0;
  }

  // We want to avoid Buffer copy
  // This will create some unknown behaviours
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  // But we can move Buffer
  Buffer(Buffer&& buf)
      : ptr_(buf.ptr_)
      , size_(buf.size_)
  {
    buf.size_ = 0;
    buf.ptr_ = nullptr;
  }

  void* host_ptr() { return ptr_->contents(); }
  const void* host_ptr() const { return ptr_->contents(); }

  MTL::Buffer* ptr() { return ptr_; }
  const MTL::Buffer* ptr() const { return ptr_; }

  size_t size() const { return size_; }

private:
  MTL::Buffer* ptr_ = nullptr;
  size_t size_ = 0;
};
};  // namespace core