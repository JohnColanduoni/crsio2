#include "crsio2.h"

using namespace crsio2;

sandbox_broker_services_t sandbox_get_broker_services() {
  return BrokerServices::GetInstance();
}
