#ifndef CRSIO2_H_
#define CRSIO2_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
  #include <wchar.h>

  typedef const wchar_t* sandbox_file_name_t;
  typedef const wchar_t* sandbox_command_line_t;
#endif

#ifdef __cplusplus

#include <memory>
#include <string>

namespace crsio2 {
    class Error {
      public:
        virtual ~Error() {}

      Error(const Error&) = delete;
      Error& operator=(const Error&) = delete;

      virtual const char* description() = 0;

      protected:
        Error() {}
    };

  class Result {
    public:
      ~Result() {
        delete value_;
      }

      Result(Result&&) = default;

      bool IsOk() {
        return value_ == nullptr;
      }
      Error* TakeRaw() {
        auto value = value_;
        value_ = nullptr;
        return value;
      }

      Result(const Result&) = delete;
      Result& operator=(const Error&) = delete;

      static Result Ok() {
        return Result(nullptr);
      }

      static Result Err(Error* error) {
        return Result(error);
      }

    private:
      Result(Error* value) :
        value_(value)
      {}

      Error* value_;
  };

  class Policy;
  class BrokerServices;
  class TargetServices;
  class TargetProcess;

  class BasePolicy {
    public:
      virtual ~BasePolicy() {}

    BasePolicy(const BasePolicy&) = delete;
    BasePolicy& operator=(const BasePolicy&) = delete;

    protected:
      BasePolicy() {}
  };

  class BaseBrokerServices {
    public:
      virtual ~BaseBrokerServices() {}

      virtual Result Init() = 0;
      virtual std::unique_ptr<Policy> CreatePolicy() = 0;
      virtual Result SpawnTarget(
          sandbox_file_name_t executable_path,
          sandbox_command_line_t command_line,
          Policy* policy,
          std::unique_ptr<TargetProcess>& process_out
      ) = 0;
      virtual Result WaitForAllTargets() = 0;

      BaseBrokerServices(const BaseBrokerServices&) = delete;
      BaseBrokerServices& operator=(const BaseBrokerServices&) = delete;

    protected:
      BaseBrokerServices() {}
  };

  class BaseTargetServices {
    public:
      virtual ~BaseTargetServices() {}

      virtual Result Init() = 0;
      virtual void Lower() = 0;

      BaseTargetServices(const BaseTargetServices&) = delete;
      BaseTargetServices& operator=(const BaseTargetServices&) = delete;

    protected:
      BaseTargetServices() {}
  };

  class BaseTargetProcess {
    public:
      virtual ~BaseTargetProcess() {}

      virtual uint32_t GetProcessId() = 0;

      virtual Result Resume() = 0;

      BaseTargetProcess(const BaseTargetProcess&) = delete;
      BaseTargetProcess& operator=(const BaseTargetProcess&) = delete;

    protected:
      BaseTargetProcess() {}
  };
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef struct _sandbox_error_t* sandbox_error_t;

typedef struct _sandbox_broker_services_t* sandbox_broker_services_t;
typedef struct _sandbox_policy_t* sandbox_policy_t;
typedef struct _sandbox_target_process_t* sandbox_target_process_t;

typedef struct _sandbox_target_services_t* sandbox_target_services_t;
#else
typedef crsio2::Error* sandbox_error_t;

typedef crsio2::BrokerServices* sandbox_broker_services_t;
typedef crsio2::Policy* sandbox_policy_t;
typedef crsio2::TargetProcess* sandbox_target_process_t;

typedef crsio2::TargetServices* sandbox_target_services_t;
#endif

const char* sandbox_error_description(sandbox_error_t error);
void sandbox_error_release(sandbox_error_t);

sandbox_broker_services_t sandbox_get_broker_services();
sandbox_error_t sandbox_broker_services_init(sandbox_broker_services_t);
sandbox_policy_t sandbox_create_policy(sandbox_broker_services_t);
sandbox_error_t sandbox_spawn_target(
    sandbox_broker_services_t broker,
    sandbox_file_name_t executable_path,
    sandbox_command_line_t command_line,
    sandbox_policy_t policy,
    sandbox_target_process_t* process_out
);
sandbox_error_t sandbox_wait_for_all_targets(sandbox_broker_services_t);

void sandbox_policy_release(sandbox_policy_t);

void sandbox_target_process_release(sandbox_target_process_t);
uint32_t sandbox_target_process_get_id(sandbox_target_process_t);
sandbox_error_t sandbox_target_process_resume(sandbox_target_process_t);

sandbox_target_services_t sandbox_get_target_services();
sandbox_error_t sandbox_target_services_init(sandbox_target_services_t);
void sandbox_target_services_lower(sandbox_target_services_t);

#ifdef __cplusplus
}
#endif

#ifdef _WIN32
  #include <crsio2/windows.h>
#endif



#endif
