#include "crsio2.h"
#include <sandbox/win/src/sandbox_factory.h>

#include "target_services.h"

#include <mutex>

using namespace crsio2;

TargetServices* TargetServices::GetInstance() {
  static std::once_flag instance_init;
  static TargetServicesImpl* instance = nullptr;
  std::call_once(instance_init, []() {
      auto inner_instance = sandbox::SandboxFactory::GetTargetServices();
      if(inner_instance == nullptr) {
        instance = nullptr;
      } else {
        instance = new TargetServicesImpl(inner_instance);
      }
  });
  return instance;
}

TargetServicesImpl::TargetServicesImpl(sandbox::TargetServices* inner) {
}
