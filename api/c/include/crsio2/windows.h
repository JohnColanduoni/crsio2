#ifndef CRSIO2_WINDOWS_H_
#define CRSIO2_WINDOWS_H_

#ifdef __cplusplus

#include <windows.h>

namespace crsio2 {
  class Policy;

  enum TokenLevel {
    USER_LOCKDOWN = 0,
    USER_RESTRICTED,
    USER_LIMITED,
    USER_INTERACTIVE,
    USER_NON_ADMIN,
    USER_RESTRICTED_SAME_ACCESS,
    USER_UNPROTECTED,
    USER_LAST
  };

  enum JobLevel {
    JOB_LOCKDOWN = 0,
    JOB_RESTRICTED,
    JOB_LIMITED_USER,
    JOB_INTERACTIVE,
    JOB_UNPROTECTED,
    JOB_NONE
  };

  enum IntegrityLevel {
    INTEGRITY_LEVEL_SYSTEM,
    INTEGRITY_LEVEL_HIGH,
    INTEGRITY_LEVEL_MEDIUM,
    INTEGRITY_LEVEL_MEDIUM_LOW,
    INTEGRITY_LEVEL_LOW,
    INTEGRITY_LEVEL_BELOW_LOW,
    INTEGRITY_LEVEL_UNTRUSTED,
    INTEGRITY_LEVEL_LAST
  };


  class BrokerServices : public BaseBrokerServices {
    public:
      static BrokerServices* GetInstance();
  };

  class TargetServices : public BaseTargetServices {
    public:
      static TargetServices* GetInstance();
  };

  class Policy : public BasePolicy {
    public:
      virtual Result SetTokenLevel(TokenLevel initial, TokenLevel lockdown) = 0;

      virtual Result SetJobLevel(JobLevel level) = 0;

      virtual Result SetIntegrityLevel(IntegrityLevel level) = 0;
      virtual Result SetDelayedIntegrityLevel(IntegrityLevel level) = 0;

      virtual Result SetAlternateDesktop(bool alternate_winstation) = 0;

      virtual Result SetLowBox(const wchar_t *sid) = 0;

      virtual Result SetStdoutHandle(void* handle) = 0;
      virtual Result SetStderrHandle(void* handle) = 0;

      virtual Result AddHandleToShare(void* handle) = 0;

      enum SubSystem {
        SUBSYS_FILES,             
        SUBSYS_NAMED_PIPES,       
        SUBSYS_PROCESS,           
        SUBSYS_REGISTRY,          
        SUBSYS_SYNC,              
        SUBSYS_WIN32K_LOCKDOWN    
      };

      enum Semantics {
        FILES_ALLOW_ANY,       
        FILES_ALLOW_READONLY,  
        FILES_ALLOW_QUERY,     
        FILES_ALLOW_DIR_ANY,   
        NAMEDPIPES_ALLOW_ANY,  
        PROCESS_MIN_EXEC,      
        PROCESS_ALL_EXEC,      
        EVENTS_ALLOW_ANY,      
        EVENTS_ALLOW_READONLY, 
        REG_ALLOW_READONLY,    
        REG_ALLOW_ANY,         
        FAKE_USER_GDI_INIT,    
        IMPLEMENT_OPM_APIS     
      };

      virtual Result AddRule(SubSystem subsystem, Semantics semantics, const wchar_t* pattern) = 0;
  };

  class TargetProcess : public BaseTargetProcess {
    public:
      virtual PROCESS_INFORMATION* GetProcessInformation() = 0;
  };
}
#else
  typedef enum _sandbox_token_level_t {
    USER_LOCKDOWN = 0,
    USER_RESTRICTED,
    USER_LIMITED,
    USER_INTERACTIVE,
    USER_NON_ADMIN,
    USER_RESTRICTED_SAME_ACCESS,
    USER_UNPROTECTED,
    USER_LAST
  } sandbox_token_level_t;

  typedef enum _sandbox_job_level_t {
    JOB_LOCKDOWN = 0,
    JOB_RESTRICTED,
    JOB_LIMITED_USER,
    JOB_INTERACTIVE,
    JOB_UNPROTECTED,
    JOB_NONE
  } sandbox_job_level_t;

  typedef enum _sandbox_integrity_level_t {
    INTEGRITY_LEVEL_SYSTEM,
    INTEGRITY_LEVEL_HIGH,
    INTEGRITY_LEVEL_MEDIUM,
    INTEGRITY_LEVEL_MEDIUM_LOW,
    INTEGRITY_LEVEL_LOW,
    INTEGRITY_LEVEL_BELOW_LOW,
    INTEGRITY_LEVEL_UNTRUSTED,
    INTEGRITY_LEVEL_LAST
  } sandbox_integrity_level_t;

  typedef enum _sandbox_rule_subsystem_t {
    SUBSYS_FILES,             
    SUBSYS_NAMED_PIPES,       
    SUBSYS_PROCESS,           
    SUBSYS_REGISTRY,          
    SUBSYS_SYNC,              
    SUBSYS_WIN32K_LOCKDOWN    
  } sandbox_rule_subsystem_t;

  typedef enum _sandbox_rule_semantics_t {
    FILES_ALLOW_ANY,       
    FILES_ALLOW_READONLY,  
    FILES_ALLOW_QUERY,     
    FILES_ALLOW_DIR_ANY,   
    NAMEDPIPES_ALLOW_ANY,  
    PROCESS_MIN_EXEC,      
    PROCESS_ALL_EXEC,      
    EVENTS_ALLOW_ANY,      
    EVENTS_ALLOW_READONLY, 
    REG_ALLOW_READONLY,    
    REG_ALLOW_ANY,         
    FAKE_USER_GDI_INIT,    
    IMPLEMENT_OPM_APIS     
  } sandbox_rule_semantics_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
typedef crsio2::TokenLevel sandbox_token_level_t;
typedef crsio2::JobLevel sandbox_job_level_t;
typedef crsio2::IntegrityLevel sandbox_integrity_level_t;
typedef crsio2::Policy::SubSystem sandbox_rule_subsystem_t;
typedef crsio2::Policy::Semantics sandbox_rule_semantics_t;
#endif

sandbox_error_t sandbox_policy_set_token_level(sandbox_policy_t policy, sandbox_token_level_t initial, sandbox_token_level_t lockdown);
sandbox_error_t sandbox_policy_set_job_level(sandbox_policy_t policy, sandbox_job_level_t level);
sandbox_error_t sandbox_policy_set_integrity_level(sandbox_policy_t policy, sandbox_integrity_level_t level);
sandbox_error_t sandbox_policy_set_delayed_integrity_level(sandbox_policy_t policy, sandbox_integrity_level_t level);
sandbox_error_t sandbox_policy_set_alternate_desktop(sandbox_policy_t policy, bool alternate_winstation);
sandbox_error_t sandbox_policy_set_low_box(sandbox_policy_t policy, const wchar_t* sid);
sandbox_error_t sandbox_policy_set_stdout_handle(sandbox_policy_t policy, void* handle);
sandbox_error_t sandbox_policy_set_stderr_handle(sandbox_policy_t policy, void* handle);
sandbox_error_t sandbox_policy_add_handle_to_share(sandbox_policy_t policy, void* handle);
sandbox_error_t sandbox_policy_add_rule(sandbox_policy_t policy, sandbox_rule_subsystem_t subsystem, sandbox_rule_semantics_t semantics, const wchar_t* pattern);

void* sandbox_target_process_get_process_information(sandbox_target_process_t process);

#ifdef __cplusplus
}
#endif

#endif
