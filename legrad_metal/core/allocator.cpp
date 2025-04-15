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
Allocator::Allocator()
    : total_bytes_(0)
{
}

Allocator::~Allocator()
{
  free_cached();
}

void Allocator::free_cached()
{
  for (const auto& [size, ptr] : available_pool_) {
    ptr->release();
  }
  available_pool_.clear();
}

Buffer Allocator::alloc(size_t nbytes)
{
  std::lock_guard<std::mutex> lock(mtx_);

  MTL::Buffer* ptr = nullptr;

  if (nbytes != 0) {
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
        free_cached();
        LEGRAD_LOG_WARN("All caches from Allocator are deleted", 0);
        ptr = allocate_and_throw(nbytes);
      }
      allocation_map_.insert({ptr, nbytes});
      total_bytes_ += nbytes;
    }
  }

  LEGRAD_LOG_WARN("Allocator create buffer with 0 size", 0);

  return Buffer(ptr, nbytes);
}

MTL::Buffer* Allocator::allocate_and_throw(size_t nbytes)
{
  auto device = MetalMgr::instance().device();
  MTL::Buffer* ptr = device->newBuffer(nbytes, MTL::ResourceStorageModeShared);

  if (ptr == nullptr) {
    LEGRAD_LOG_ERR("Cannot allocate buffer with size {}", nbytes);
    throw std::bad_alloc();
  }

  return ptr;
}

void Allocator::free(Buffer* buf)
{
  std::lock_guard<std::mutex> loc(mtx_);

  if (buf->ptr() == nullptr) {
    LEGRAD_LOG_WARN("Cannot free a buffer with null data", 0);
    return;
  }

  auto iter = allocation_map_.find(buf->ptr());

  if (LEGRAD_UNLIKELY(iter == allocation_map_.end())) {
    // this case is rarely happened
    LEGRAD_LOG_WARN("This buffer ({}) is not allocated by this allocator",
                    (void*)buf);
    return;
  }

  size_t size = iter->second;

  // Delete memory from current map
  allocation_map_.erase(buf->ptr());
  // Return memory to pool
  available_pool_.insert({size, buf->ptr()});
}
};  // namespace core