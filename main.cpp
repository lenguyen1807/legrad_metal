#include <iostream>

#include "legrad_metal/internal/view_pack.h"
#include "legrad_metal/metal_mgr.h"

int main()
{
  auto view = internal::view_pack();
  auto& mgr = MetalMgr::instance();
  std::cout << "Hello World\n";
  return 0;
}