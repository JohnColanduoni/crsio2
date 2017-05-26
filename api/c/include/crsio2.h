#ifndef CRSIO2_H_
#define CRSIO2_H_

#ifdef __cplusplus
namespace crsio2 {
  class BaseBrokerServices {
    public:
      virtual ~BaseBrokerServices() {}

    BaseBrokerServices(const BaseBrokerServices&) = delete;
    BaseBrokerServices& operator=(const BaseBrokerServices&) = delete;

    protected:
      BaseBrokerServices() {}
  };

  class BaseTargetServices {
    public:
      virtual ~BaseTargetServices() {}

    BaseTargetServices(const BaseTargetServices&) = delete;
    BaseTargetServices& operator=(const BaseTargetServices&) = delete;

    protected:
      BaseTargetServices() {}
  };
}
#endif

#ifdef _WIN32
  #include <crsio2/windows.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef struct _sandbox_broker_services_t* sandbox_broker_services_t;
typedef struct _sandbox_target_services_t* sandbox_target_services_t;
#else
typedef crsio2::BrokerServices* sandbox_broker_services_t;
typedef crsio2::TargetServices* sandbox_target_services_t;
#endif

sandbox_broker_services_t sandbox_get_broker_services();

sandbox_target_services_t sandbox_get_target_services();


#ifdef __cplusplus
}
#endif

#endif
