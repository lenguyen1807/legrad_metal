#include <cassert>
#include <cstddef>
#include <exception>

#include "Metal/MTLBuffer.hpp"
#include "allocator.h"
#include "buffer.h"
#include "macros/expr.h"
#include "macros/log.h"
#include "metal_mgr.h"

namespace core
{
MetalAllocator::MetalAllocator()
    : total_bytes_(0)
{
}

MetalAllocator::~MetalAllocator()
{
  LEGRAD_LOG_TRACE("Allocator destruct called", 0);
  free_cached();
}

void MetalAllocator::free_cached()
{
  for (const auto& [size, ptr] : available_pool_) {
    LEGRAD_LOG_TRACE("Release buffer with pointer {} and size {}",
                     ptr->contents(), size);
    ptr->release();
  }
  available_pool_.clear();
}

MetalBuffer MetalAllocator::alloc(size_t nbytes)
{
  std::lock_guard<std::mutex> lock(mtx_);

  MTL::Buffer* ptr = nullptr;

  if (nbytes == 0) {
    LEGRAD_LOG_WARN("Allocator create buffer with 0 size", 0);
  } else {
    // We find if memory pool already has the memory with size we want
    auto it = available_pool_.find(nbytes);

    if (it != available_pool_.end()) {
      ptr = it->second;
      // If we can find memory from pool
      // We must make sure it is not null
      LEGRAD_ASSERT(ptr != nullptr, "Data from Allocator pool cannot be null",
                    0);
      // Then delete from current cache
      available_pool_.erase(it);
    } else {
      // If we cant find the memory, allocate new one
      try {
        ptr = allocate_and_throw(nbytes);
      } catch (const std::exception& e) {
        // But before we try to do anything else
        // There maybe an out of memory issue because of caching
        // so we need to free all cache
        LEGRAD_LOG_WARN("Canot allocate buffer, try to delete cache first", 0);
        free_cached();
        LEGRAD_LOG_WARN("All caches from Allocator are deleted", 0);
        ptr = allocate_and_throw(nbytes);
      }
      allocation_map_.insert({ptr, nbytes});
      total_bytes_ += nbytes;
    }
  }

  return MetalBuffer(ptr, nbytes);
}

MTL::Buffer* MetalAllocator::allocate_and_throw(size_t nbytes)
{
  auto device = MetalMgr::instance().device();
  MTL::Buffer* ptr = device->newBuffer(nbytes, MTL::ResourceStorageModeShared);

  if (ptr == nullptr) {
    LEGRAD_LOG_ERR("Cannot allocate buffer with size {}", nbytes);
    throw std::bad_alloc();
  }

  return ptr;
}

void MetalAllocator::free(MetalBuffer* buf)
{
  std::lock_guard<std::mutex> lock(mtx_);

  if (buf->ptr() == nullptr) {
    LEGRAD_LOG_WARN("Cannot free a buffer with null data", 0);
    return;
  }

  auto iter = allocation_map_.find(buf->ptr());

  if (LEGRAD_UNLIKELY(iter == allocation_map_.end())) {
    // this case is rarely happened
    LEGRAD_LOG_WARN("This buffer ({}) is not allocated by this allocator",
                    buf->ptr()->contents());
    return;
  }

  size_t size = iter->second;

  // Delete memory from current map
  allocation_map_.erase(buf->ptr());
  // Return memory to pool
  available_pool_.insert({size, buf->ptr()});
}
};  // namespace core