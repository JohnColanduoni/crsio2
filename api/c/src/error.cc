#include "crsio2.h"

const char* sandbox_error_description(sandbox_error_t error) {
  return error->description();
}

void sandbox_error_release(sandbox_error_t error) {
  delete error;
}

