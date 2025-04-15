#pragma once

#include <mutex>
#include <unordered_map>

#include "backend/metal_function.h"
#include "core/dtype.h"
#include "utils/funcs.h"

namespace metal
{
using DispatcherKey = std::pair<Ops, core::TypeInfo>;
using DispatcherType =
    std::unordered_map<DispatcherKey, metal::Kernel*, utils::HashPairKey>;

class KernelDispatcher
{
public:
  ~KernelDispatcher();
  Kernel* dispatch(metal::Ops op, core::TypeInfo info);

private:
  DispatcherType dispatcher_;
  std::mutex mtx_;
};
};  // namespace metal