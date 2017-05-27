#include "crsio2.h"

void sandbox_error_release(sandbox_error_t error) {
  delete error;
}

