#include "crsio2.h"

using namespace crsio2;

sandbox_target_services_t sandbox_get_target_services() {
  return TargetServices::GetInstance();
}
