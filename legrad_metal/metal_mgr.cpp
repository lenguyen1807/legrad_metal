#include <cstddef>
#include <stdexcept>

#include "Foundation/NSBundle.hpp"
#include "Foundation/NSString.hpp"
#include "macros/log.h"
#include "metal_mgr.h"
#include "utils/funcs.h"

MetalMgr::MetalMgr()
{
  std::lock_guard<std::mutex> lock(mtx_);

  device_ = MTL::CreateSystemDefaultDevice();
  cmd_queue_ = device_->newCommandQueue();

  NS::Error* error = nullptr;

  // create library
  {
    // read kernel source
    auto src = utils::read_file(std::string(KERNEL_PATH) + "main.metal");
    NS::String* metal_src =
        NS::String::string(src.c_str(), NS::StringEncoding::UTF8StringEncoding);

    // compile library
    library_ = device_->newLibrary(metal_src, nullptr, &error);
    metal_src->release();

    // check errors
    if (error != nullptr) {
      const char* msg = error->localizedDescription()->utf8String();
      error->release();
      LEGRAD_THROW_ERROR(std::runtime_error,
                         "Failed to create Metal library with error: {}", msg);
    }
  }

  // initialize allocator, dispatcher, etc. after create device and library
  // sucessfully
  allocator_ = std::make_unique<core::MetalAllocator>();
  dispatcher_ = std::make_unique<metal::KernelDispatcher>();
}

MetalMgr::~MetalMgr()
{
  LEGRAD_LOG_TRACE("MetalMgr destructor called", 0);

  dispatcher_ = nullptr;
  allocator_ = nullptr;

  LEGRAD_LOG_TRACE("MetalMgr final delete (library, device, command queue)", 0);

  library_->release();
  cmd_queue_->release();
  device_->release();
}