#include "crsio2.h"

#include "error.h"

using namespace crsio2;

const char* ChromiumError::description() {
  switch(this->code_) {
    case sandbox::SBOX_ALL_OK:
      return "ok";
    case sandbox::SBOX_ERROR_GENERIC:
      FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        this->win32_err_,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&this->win32_err_string_,
        0,
        nullptr
      );
      return this->win32_err_string_;
    case sandbox::SBOX_ERROR_BAD_PARAMS:
      return "bad parameters";
    case sandbox::SBOX_ERROR_UNSUPPORTED:
      return "unsupported";
/*
  SBOX_ERROR_NO_SPACE = 4,
  // The ipc service requested does not exist.
  SBOX_ERROR_INVALID_IPC = 5,
  // The ipc service did not complete.
  SBOX_ERROR_FAILED_IPC = 6,
  // The requested handle was not found.
  SBOX_ERROR_NO_HANDLE = 7,
  // This function was not expected to be called at this time.
  SBOX_ERROR_UNEXPECTED_CALL = 8,
  // WaitForAllTargets is already called.
  SBOX_ERROR_WAIT_ALREADY_CALLED = 9,
  // A channel error prevented DoCall from executing.
  SBOX_ERROR_CHANNEL_ERROR = 10,
  // Failed to create the alternate desktop.
  SBOX_ERROR_CANNOT_CREATE_DESKTOP = 11,
  // Failed to create the alternate window station.
  SBOX_ERROR_CANNOT_CREATE_WINSTATION = 12,
  // Failed to switch back to the interactive window station.
  SBOX_ERROR_FAILED_TO_SWITCH_BACK_WINSTATION = 13,
  // The supplied AppContainer is not valid.
  SBOX_ERROR_INVALID_APP_CONTAINER = 14,
  // The supplied capability is not valid.
  SBOX_ERROR_INVALID_CAPABILITY = 15,
  // There is a failure initializing the AppContainer.
  SBOX_ERROR_CANNOT_INIT_APPCONTAINER = 16,
  // Initializing or updating ProcThreadAttributes failed.
  SBOX_ERROR_PROC_THREAD_ATTRIBUTES = 17,
  // Error in creating process.
  SBOX_ERROR_CREATE_PROCESS = 18,
  // Failure calling delegate PreSpawnTarget.
  SBOX_ERROR_DELEGATE_PRE_SPAWN = 19,
  // Could not assign process to job object.
  SBOX_ERROR_ASSIGN_PROCESS_TO_JOB_OBJECT = 20,
  // Could not assign process to job object.
  SBOX_ERROR_SET_THREAD_TOKEN = 21,
  // Could not get thread context of new process.
  SBOX_ERROR_GET_THREAD_CONTEXT = 22,
  // Could not duplicate target info of new process.
  SBOX_ERROR_DUPLICATE_TARGET_INFO = 23,
  // Could not set low box token.
  SBOX_ERROR_SET_LOW_BOX_TOKEN = 24,
  // Could not create file mapping for IPC dispatcher.
  SBOX_ERROR_CREATE_FILE_MAPPING = 25,
  // Could not duplicate shared section into target process for IPC dispatcher.
  SBOX_ERROR_DUPLICATE_SHARED_SECTION = 26,
  // Could not map view of shared memory in broker.
  SBOX_ERROR_MAP_VIEW_OF_SHARED_SECTION = 27,
  // Could not apply ASLR mitigations to target process.
  SBOX_ERROR_APPLY_ASLR_MITIGATIONS = 28,
  // Could not setup one of the required interception services.
  SBOX_ERROR_SETUP_BASIC_INTERCEPTIONS = 29,
  // Could not setup basic interceptions.
  SBOX_ERROR_SETUP_INTERCEPTION_SERVICE = 30,
  // Could not initialize interceptions. This usually means 3rd party software
  // is stomping on our hooks, or can sometimes mean the syscall format has
  // changed.
  SBOX_ERROR_INITIALIZE_INTERCEPTIONS = 31,
  // Could not setup the imports for ntdll in target process.
  SBOX_ERROR_SETUP_NTDLL_IMPORTS = 32,
  // Could not setup the handle closer in target process.
  SBOX_ERROR_SETUP_HANDLE_CLOSER = 33,
  // Cannot get the current Window Station.
  SBOX_ERROR_CANNOT_GET_WINSTATION = 34,
  // Cannot query the security attributes of the current Window Station.
  SBOX_ERROR_CANNOT_QUERY_WINSTATION_SECURITY = 35,
  // Cannot get the current Desktop.
  SBOX_ERROR_CANNOT_GET_DESKTOP = 36,
  // Cannot query the security attributes of the current Desktop.
  SBOX_ERROR_CANNOT_QUERY_DESKTOP_SECURITY = 37,
  // Cannot setup the interception manager config buffer.
  SBOX_ERROR_CANNOT_SETUP_INTERCEPTION_CONFIG_BUFFER = 38,
  // Cannot copy data to the child process.
  SBOX_ERROR_CANNOT_COPY_DATA_TO_CHILD = 39,
  // Cannot setup the interception thunk.
  SBOX_ERROR_CANNOT_SETUP_INTERCEPTION_THUNK = 40,
  // Cannot resolve the interception thunk.
  SBOX_ERROR_CANNOT_RESOLVE_INTERCEPTION_THUNK = 41,
  // Cannot write interception thunk to child process.
  SBOX_ERROR_CANNOT_WRITE_INTERCEPTION_THUNK = 42,
  // Cannot find the base address of the new process.
  SBOX_ERROR_CANNOT_FIND_BASE_ADDRESS = 43,
  // Placeholder for last item of the enum.
  SBOX_ERROR_LAST*/
    default:
      return "unknown";
  }
}