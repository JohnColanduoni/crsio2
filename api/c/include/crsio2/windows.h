#ifndef CRSIO2_WINDOWS_H_
#define CRSIO2_WINDOWS_H_

#ifdef __cplusplus
namespace crsio2 {
  class Policy;

  class BrokerServices : public BaseBrokerServices {
    public:
      static BrokerServices* GetInstance();
  };

  class TargetServices : public BaseTargetServices {
    public:
      static TargetServices* GetInstance();
  };

  class Policy : public BasePolicy {
  };

  class TargetProcess : public BaseTargetProcess {

  };
}
#endif

#endif
