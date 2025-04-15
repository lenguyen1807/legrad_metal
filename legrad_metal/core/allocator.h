#pragma once

#include <cstddef>
#include <map>

#include "Metal/MTLBuffer.hpp"

namespace core
{
// forward declaration
class Buffer;

class Allocator
{
public:
  Allocator();
  ~Allocator();

  Buffer alloc(size_t nbytes);
  void free(Buffer* buf);
  size_t size() const { return allocation_map_.size(); }
  size_t nbytes() const { return total_bytes_; }

private:
  void free_cached();
  MTL::Buffer* allocate_and_throw(size_t nbytes);

private:
  std::mutex mtx_;
  std::multimap<size_t, MTL::Buffer*> available_pool_;
  std::multimap<MTL::Buffer*, size_t> allocation_map_;

  // for debug
  size_t total_bytes_;
};
};  // namespace core