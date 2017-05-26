#include "crsio2.h"
#include <sandbox/win/src/sandbox.h>

namespace crsio2 {
  class BrokerServicesImpl : public BrokerServices {
    public:
      BrokerServicesImpl(sandbox::BrokerServices*);
  };
}
