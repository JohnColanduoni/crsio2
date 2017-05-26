#include "crsio2.h"
#include <sandbox/win/src/sandbox_factory.h>

#include "broker_services.h"

#include <mutex>

using namespace crsio2;

BrokerServices* BrokerServices::GetInstance() {
  static std::once_flag instance_init;
  static BrokerServicesImpl* instance = nullptr;
  std::call_once(instance_init, []() {
      auto inner_instance = sandbox::SandboxFactory::GetBrokerServices();
      if(inner_instance == nullptr) {
        instance = nullptr;
      } else {
        instance = new BrokerServicesImpl(inner_instance);
      }
  });
  return instance;
}

BrokerServicesImpl::BrokerServicesImpl(sandbox::BrokerServices* inner) {
}

