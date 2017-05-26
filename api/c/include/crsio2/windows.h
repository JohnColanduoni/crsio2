#ifndef CRSIO2_WINDOWS_H_
#define CRSIO2_WINDOWS_H_

#ifdef __cplusplus
namespace crsio2 {
  class BrokerServices : BaseBrokerServices {
    public:
      static BrokerServices* GetInstance();
  };

  class TargetServices : BaseTargetServices {
    public:
      static TargetServices* GetInstance();
  };
}
#endif

#endif
