#include <stdexcept>

#include "backend/metal_function.h"
#include "core/dtype.h"
#include "macros/expr.h"
#include "macros/log.h"
#include "metal_dispatcher.h"
#include "metal_mgr.h"

namespace metal
{
KernelDispatcher::~KernelDispatcher()
{
  LEGRAD_LOG_TRACE("KernelDispatcher destructor called", 0);

  for (auto& [key, kernel] : dispatcher_) {
    // relese metal objects
    kernel->func->release();
    kernel->pipeline->release();
    // delete kernel
    delete kernel;
    kernel = nullptr;
  }
  dispatcher_.clear();
}

Kernel* KernelDispatcher::dispatch(metal::Ops op, core::TypeInfo info)
{
  auto& mgr = MetalMgr::instance();
  auto library = mgr.library();
  auto device = mgr.device();
  std::pair key = {op, info};

  // --- Fast path: Check if kernel already exists ---
  {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = dispatcher_.find(key);
    if (it != dispatcher_.end()) {
      return it->second;  // Kernel already loaded
    }
  }

  // clang-format off
  // --- Slow path: Kernel doesn't exist, need to create it ---
  #define LEGRAD_METAL_LOAD_KERNEL(name_prefix, type_token, kernel) \
    do { \
      NS::Error* error = nullptr; \
      const char* func_name_str = LEGRAD_STRINGIZE(LEGRAD_CONCAT_SNAKE(name_prefix, type_token)); \
      LEGRAD_LOG_DEBUG("Lazy loading kernel: {}", func_name_str);\
      /* Check kernel function */ \
      kernel->func = library->newFunction(NS::String::string(func_name_str, NS::StringEncoding::UTF8StringEncoding)); \
      LEGRAD_CHECK_AND_THROW(kernel->func != nullptr, std::runtime_error, "Failed to find Metal function during lazy load {}", func_name_str); \
      /* Check kernel pipeline */ \
      kernel->pipeline = device->newComputePipelineState(kernel->func, &error); \
      LEGRAD_CHECK_AND_THROW(error == nullptr, std::runtime_error, "Failed to create pipeline state during lazyload for function {} with error: {}", func_name_str, error->localizedDescription()->utf8String()); \
      { /* Load kernel */ \
          std::lock_guard<std::mutex> lock(mtx_); \
          /* Double check if another thread created it while we were working */ \
          auto it = dispatcher_.find(key); \
          if (it != dispatcher_.end()) { \
              /* Another thread won the race, return its kernel. unique_ptr handles cleanup. */ \
              return it->second; \
          } \
          dispatcher_.insert({key, kernel.get()}); \
          newly_created_kernel = kernel.release(); /* Transfer ownership to map */ \
          return newly_created_kernel; \
      } \
    } while(false)

  // clang-format on
  Kernel* newly_created_kernel = nullptr;
  std::unique_ptr<metal::Kernel1D> kernel_1d =
      std::make_unique<metal::Kernel1D>();

  switch (op) {
    case Ops::ADD: {
      switch (info) {
        case core::TypeInfo::Float32: {
          LEGRAD_METAL_LOAD_KERNEL(add, float, kernel_1d);
          break;
        }
        default:
          LEGRAD_THROW_ERROR(std::runtime_error,
                             "Unsupported type for {} kernel",
                             core::TypeInfoToString(info));
      }
      break;
    }
    case Ops::MUL: {
      switch (info) {
        case core::TypeInfo::Float32: {
          LEGRAD_METAL_LOAD_KERNEL(mul, float, kernel_1d);
          break;
        }
        default:
          LEGRAD_THROW_ERROR(std::runtime_error,
                             "Unsupported type for {} kernel",
                             core::TypeInfoToString(info));
      }
      break;  // Should not be reached
    }
    case Ops::RELU: {
      switch (info) {
        case core::TypeInfo::Float32:
          LEGRAD_METAL_LOAD_KERNEL(relu, float, kernel_1d);
          break;
        default:
          LEGRAD_THROW_ERROR(std::runtime_error,
                             "Unsupported type for {} kernel",
                             core::TypeInfoToString(info));
      }
      break;  // Should not be reached
    }
    default:
      LEGRAD_THROW_ERROR(std::runtime_error,
                         "Unsupported op for {} kernel loading",
                         metal::OpsToString(op));
  }

    // clang-format off
  #undef METAL_LOAD_KERNEL

  // This point should ideally not be reached if a kernel was created or an error thrown
  LEGRAD_THROW_ERROR(std::logic_error, "Kernel creation logic failed unexpectedly", 0);
}
};  // namespace metal