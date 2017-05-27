#include "crsio2.h"
#include <sandbox/win/src/sandbox.h>

namespace crsio2 {
  class TargetServicesImpl : public TargetServices {
    public:
      TargetServicesImpl(sandbox::TargetServices*);

      Result Init() override;
      void Lower() override;

    private:
      sandbox::TargetServices* inner_;
  };
}
