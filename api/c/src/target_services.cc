#include "crsio2.h"

using namespace crsio2;

sandbox_target_services_t sandbox_get_target_services() {
  return TargetServices::GetInstance();
}

sandbox_error_t sandbox_target_services_init(sandbox_target_services_t target) {
  return target->Init().TakeRaw();
}

void sandbox_target_services_lower(sandbox_target_services_t target) {
  target->Lower();
}

