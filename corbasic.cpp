/*
 *  File to make DLL which allows C++ to interop with
 *  C#. It works in C++, but is not working in C#
 *  for some oddball reason...
 */

#include <windows.h>
#include <cor.h>
#include <CorDebug.h>
#include <MetaHost.h>
#include <stdio.h>

union CorBasicEvent;

enum CorBasicEventType {
  CBEV_NEWPROC,
  CBEV_BREAK,
  CBEV_BREAKPOINT,
  CBEV_DATABREAKPOINT,
};

struct CorBasicNewProcessEvent {
  int type;
  ICorDebugProcess * process;
};

struct CorBasicBreakEvent {
  int type;
  ICorDebugAppDomain * appDomain;
  ICorDebugThread * thread;
};

struct CorBasicBreakpointEvent {
  int type;
  ICorDebugAppDomain * appDomain;
  ICorDebugThread * thread;
  ICorDebugBreakpoint * breakpoint;
};

union CorBasicEvent {
  int type;
  CorBasicBreakpointEvent breakpoint;
  CorBasicBreakEvent breakev;
  CorBasicNewProcessEvent newprocess;
};

/*
 *  Event queue with single writer and
 *  single reader. Writer is debugger callbacks
 *  and reader is the debugger code which
 *  wants the next event to poll.
 */
#define CORBASIC_EVQUEUE_LEN 32
struct CorBasicEventQueue {
  CorBasicEvent queue[CORBASIC_EVQUEUE_LEN];
  int consumer;
  int producer;
  HANDLE conssema;
  HANDLE prodsema;
};

CorBasicEventQueue * NewCBEVQueue() {
  HANDLE csema;
  HANDLE psema;
  CorBasicEventQueue * queue = 0;

  csema = CreateSemaphoreA(0, 0, CORBASIC_EVQUEUE_LEN, 0);
  if(csema) {
    psema = CreateSemaphoreA(0, CORBASIC_EVQUEUE_LEN, CORBASIC_EVQUEUE_LEN, 0);
    if(psema) {
      queue = new CorBasicEventQueue;
      queue -> consumer = 0;
      queue -> producer = 0;
      queue -> conssema = csema;
      queue -> prodsema = psema;
    } else
        CloseHandle(csema);
  }

  return queue;
}

void FreeCBEVQueue(CorBasicEventQueue * queue) {
  delete queue;
}

void AddEvent(CorBasicEventQueue * queue, CorBasicEvent * ev) {
  WaitForSingleObject(queue -> prodsema, INFINITE);
  queue -> queue[queue -> producer] = *ev;
  queue -> producer += 1;
  if(queue -> producer >= CORBASIC_EVQUEUE_LEN)
    queue -> producer = 0;
  ReleaseSemaphore(queue -> conssema, 1, 0);
}

void RemoveEvent(CorBasicEventQueue * queue, CorBasicEvent * ev) {
  DWORD waitstatus = WaitForSingleObject(queue -> conssema, INFINITE);
  *ev = queue -> queue[queue -> consumer];
  queue -> consumer += 1;
  if(queue -> consumer >= CORBASIC_EVQUEUE_LEN)
    queue -> consumer = 0;
  ReleaseSemaphore(queue -> prodsema, 1, 0);
}

int PollEvent(CorBasicEventQueue * queue, CorBasicEvent * ev) {
  DWORD waitstatus = WaitForSingleObject(queue -> conssema, 0);
  int evexists = (waitstatus == WAIT_OBJECT_0);
  if(evexists) {
    *ev = queue -> queue[queue -> consumer];
    queue -> consumer += 1;
    if(queue -> consumer >= CORBASIC_EVQUEUE_LEN)
      queue -> consumer = 0;
    ReleaseSemaphore(queue -> prodsema, 1, 0);
  }
  return evexists;
}

struct CorBasicDebugger {
  ICorDebug * debugger;
  ICorDebugProcess * process;
  CorBasicEventQueue * evqueue;
};

/*
 * NOTE: From MSDN, all callbacks in this interface are
 *       serialized and called in the same thread with
 *       the process in a synchronized state.
 */
class CorBasicManagedCallback : ICorDebugManagedCallback2 , ICorDebugManagedCallback {
  private:
  ULONG refcnt;
  CorBasicEventQueue * queue;

  public:
  CorBasicManagedCallback(CorBasicEventQueue * q) {
    refcnt = 0;
    queue = q;
  }
  
  /* IUnknown boilerplate */
  STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppv) {
    if(!ppv)
      return E_INVALIDARG;
    *ppv = 0;
    if(riid == IID_IUnknown || riid == IID_ICorDebugManagedCallback2 || riid == IID_ICorDebugManagedCallback) {
      *ppv = (LPVOID)this;
      AddRef();
      return NOERROR;
    }
    return E_NOINTERFACE;
  }
  STDMETHODIMP_(ULONG) AddRef() {
    InterlockedIncrement(&refcnt);
    return refcnt;
  }
  STDMETHODIMP_(ULONG) Release() {
    ULONG localcnt = InterlockedDecrement(&refcnt);
    if(!refcnt)
      delete this;
    return localcnt;
  }

  STDMETHODIMP Break(ICorDebugAppDomain * appDomain, ICorDebugThread * thread) {
    printf("HI from Break\n");
    return S_OK;
  }

  STDMETHODIMP Breakpoint(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugBreakpoint * breakpoint) {
    printf("HI from Breakpoint\n");
    return S_OK;
  }

  STDMETHODIMP BreakpointSetError(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugBreakpoint * breakpoint, DWORD dwError) {
    printf("HI from BreakpointSetError\n");
    return S_OK;
  }

  STDMETHODIMP ControlCTrap(ICorDebugProcess * process) {
    printf("HI from ControlCTrap\n");
    return S_OK;
  }

  STDMETHODIMP CreateAppDomain(ICorDebugProcess * process, ICorDebugAppDomain * appDomain) {
    printf("HI from CreateAppDomain\n");
    appDomain -> Attach();
    return S_OK;
  }

  STDMETHODIMP CreateProcess(ICorDebugProcess * process) {
    CorBasicEvent ev;
    printf("HI from CreateProcess\n");
    ev.type = CBEV_NEWPROC;
    ev.newprocess.process = process;
    AddEvent(queue, &ev);
    return S_OK;
  }

  STDMETHODIMP CreateThread(ICorDebugAppDomain * appDomain, ICorDebugThread * thread) {
    printf("HI from CreateThread\n");
    return S_OK;
  }

  STDMETHODIMP DebuggerError(ICorDebugProcess * process, HRESULT errorHR, DWORD errorCode) {
    printf("HI from DebuggerError\n");
    return S_OK;
  }

  STDMETHODIMP EditAndContinueRemap(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugFunction * function, BOOL fAccurate) {
    printf("HI from EditAndContinueRemap\n");
    return S_OK;
  }

  STDMETHODIMP EvalComplete(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugEval * eval) {
    printf("HI from EvalComplete\n");
    return S_OK;
  }

  STDMETHODIMP EvalException(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugEval * eval) {
    printf("HI from EvalException\n");
    return S_OK;
  }

  STDMETHODIMP Exception(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, BOOL unhandled) {
    printf("HI from Exception\n");
    return S_OK;
  }

  STDMETHODIMP ExitAppDomain(ICorDebugProcess * process, ICorDebugAppDomain * appDomain) {
    printf("HI from ExitAppDomain\n");
    return S_OK;
  }

  STDMETHODIMP ExitProcess(ICorDebugProcess * process) {
    printf("HI from ExitProcess\n");
    return S_OK;
  }

  STDMETHODIMP ExitThread(ICorDebugAppDomain * appDomain, ICorDebugThread * thread) {
    printf("HI from ExitThread\n");
    return S_OK;
  }

  STDMETHODIMP LoadAssembly(ICorDebugAppDomain * appDomain, ICorDebugAssembly * assembly) {
    printf("HI from LoadAssembly\n");
    return S_OK;
  }

  STDMETHODIMP LoadClass(ICorDebugAppDomain * appDomain, ICorDebugClass * c) {
    printf("HI from LoadClass\n");
    return S_OK;
  }

  STDMETHODIMP LoadModule(ICorDebugAppDomain * appDomain, ICorDebugModule * module) {
    printf("HI from LoadModule\n");
    return S_OK;
  }

  STDMETHODIMP LogMessage(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, LONG lLevel, WCHAR * logSwitchName, WCHAR * message) {
    printf("HI from LogMessage\n");
    return S_OK;
  }

  STDMETHODIMP LogSwitch(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, LONG lLevel, ULONG ulReason, WCHAR * logSwitchName, WCHAR * parentName) {
    printf("HI from LogSwitch\n");
    return S_OK;
  }

  STDMETHODIMP NameChange(ICorDebugAppDomain * appDomain, ICorDebugThread * thread) {
    printf("HI from NameChange\n");
    return S_OK;
  }

  STDMETHODIMP StepComplete(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugStepper * stepper, CorDebugStepReason reason) {
    printf("HI from StepComplete\n");
    return S_OK;
  }

  STDMETHODIMP UnloadAssembly(ICorDebugAppDomain * appDomain, ICorDebugAssembly * assembly) {
    printf("HI from UnloadAssembly\n");
    return S_OK;
  }

  STDMETHODIMP UnloadClass(ICorDebugAppDomain * appDomain, ICorDebugClass * c) {
    printf("HI from UnloadClass\n");
    return S_OK;
  }

  STDMETHODIMP UnloadModule(ICorDebugAppDomain * appDomain, ICorDebugModule * module) {
    printf("HI from UnloadModule\n");
    return S_OK;
  }

  STDMETHODIMP UpdateModuleSymbols(ICorDebugAppDomain * appDomain, ICorDebugModule * module, IStream * symbolStream) {
    printf("HI from UpdateModuleSymbols\n");
    return S_OK;
  }

  STDMETHODIMP ChangeConnection(ICorDebugProcess * process, CONNID dwConnectionId) {
    printf("HI from ChangeConnection\n");
    return S_OK;
  }

  STDMETHODIMP CreateConnection(ICorDebugProcess * process, CONNID dwConnectionId, WCHAR * connName) {
    printf("HI from CreateConnection\n");
    return S_OK;
  }

  STDMETHODIMP DestroyConnection(ICorDebugProcess * process, CONNID dwConnectionId) {
    printf("HI from DestroyConnection\n");
    return S_OK;
  }

  STDMETHODIMP Exception(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugFrame * frame, ULONG32 offset, CorDebugExceptionCallbackType dwEventType, DWORD dwFlags) {
    printf("HI from Exception2\n");
    return S_OK;
  }

  STDMETHODIMP ExceptionUnwind(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, CorDebugExceptionUnwindCallbackType dwEventType, DWORD dwFlags) {
    printf("HI from ExceptionUnwind\n");
    return S_OK;
  }

  STDMETHODIMP FunctionRemapComplete(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugFunction * function) {
    printf("HI from FunctionRemapComplete\n");
    return S_OK;
  }

  STDMETHODIMP FunctionRemapOpportunity(ICorDebugAppDomain * appDomain, ICorDebugThread * thread, ICorDebugFunction * oldFunction, ICorDebugFunction * newFunction, ULONG32 oldOffset) {
    printf("HI from FunctionRemapOpportunity\n");
    return S_OK;
  }

  STDMETHODIMP MDANotification(ICorDebugController * controller, ICorDebugThread * thread, ICorDebugMDA * mda) {
    printf("HI from MDANotification\n");
    return S_OK;
  }
};

/*
 *  NOTE: From MSDN, the unmanaged callbacks are special only
 *        for the fact that callbacks may be nested
 *        if managed state is polled. Otherwise, still called
 *        from the same thread.
 */
class CorBasicUnmanagedCallback : ICorDebugUnmanagedCallback {
  private:
  ULONG refcnt;
  CorBasicEventQueue * queue;

  public:
  CorBasicUnmanagedCallback(CorBasicEventQueue * q) {
    refcnt = 0;
    queue = q;
  }
  
  /* IUnknown boilerplate */
  STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppv) {
    if(!ppv)
      return E_INVALIDARG;
    *ppv = 0;
    if(riid == IID_IUnknown || riid == IID_ICorDebugUnmanagedCallback) {
      *ppv = (LPVOID)this;
      AddRef();
      return NOERROR;
    }
    return E_NOINTERFACE;
  }
  STDMETHODIMP_(ULONG) AddRef() {
    InterlockedIncrement(&refcnt);
    return refcnt;
  }
  STDMETHODIMP_(ULONG) Release() {
    ULONG localcnt = InterlockedDecrement(&refcnt);
    if(!refcnt)
      delete this;
    return localcnt;
  }

  STDMETHODIMP DebugEvent(DEBUG_EVENT * debugEvent, BOOL fOutOfBand) {
    printf("HI from DebugEvent\n");
    
    return S_OK;
  }
};


int wstr_startswith(const wchar_t * widestr, const wchar_t * prefix) {
  while(*widestr && *prefix && *widestr == *prefix)
    { prefix++; widestr++; }
  return !*prefix;
}

#define MAX_VERSION_LEN 63
ICLRRuntimeInfo * GetRuntimeFromVersion(const wchar_t * runtimeVersion) {
  ICLRMetaHost * host;
  ICLRRuntimeInfo * runtime;
  IEnumUnknown * runtimeEnum;
  ULONG len;
  HRESULT comStatus;
  wchar_t versionBuffer[MAX_VERSION_LEN + 1];

  host = 0;
  runtime = 0;
  runtimeEnum = 0;

  if(wcsnlen(runtimeVersion, MAX_VERSION_LEN + 1) < MAX_VERSION_LEN
     && CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID *)&host) == S_OK
     && host -> EnumerateInstalledRuntimes(&runtimeEnum) == S_OK) {
    while((comStatus = runtimeEnum -> Next(1, (IUnknown **)&runtime, &len)) == S_OK) {
      len = MAX_VERSION_LEN;
      runtime -> GetVersionString(versionBuffer, &len);
      if(wstr_startswith(versionBuffer, runtimeVersion))
        break;
      runtime -> Release();
      runtime = 0;
    }
    if(comStatus != S_OK)
      runtime = 0;
  }

  if(host)
    host -> Release();
  if(runtimeEnum)
    runtimeEnum -> Release();

  return runtime;
}

ICorDebug * CreateDebuggerFromRuntime(ICLRRuntimeInfo * runtime, CorBasicEventQueue * queue) {
  HRESULT comStatus;
  ULONG len;
  ICorDebug * debugger;
  ICorDebug * temp;

  debugger = 0;
  if(runtime -> GetInterface(CLSID_CLRDebuggingLegacy, IID_ICorDebug, (LPVOID *)&temp) == S_OK) {
    
    debugger = temp;
    debugger -> Initialize();
    debugger -> SetManagedHandler((ICorDebugManagedCallback *)new CorBasicManagedCallback(queue));
    debugger -> SetUnmanagedHandler((ICorDebugUnmanagedCallback *)new CorBasicUnmanagedCallback(queue));
  }
  
  return debugger;
}

#define MAX_COMMAND_LEN 32767
ICorDebugProcess * CreateDebugProcess(ICorDebug * debugger, const wchar_t * commandLine) {
  ICorDebugProcess * process;
  ICorDebugProcess * temp;
  wchar_t commandBuffer[MAX_COMMAND_LEN + 1];
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  process = 0;
  if(wcsnlen(commandLine, MAX_COMMAND_LEN + 1) < MAX_COMMAND_LEN) {
    wcsncpy(commandBuffer, commandLine, MAX_COMMAND_LEN);
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    if(debugger -> CreateProcess(0, commandBuffer, 0, 0, 0, CREATE_NEW_CONSOLE, 0, 0, &si, &pi, DEBUG_NO_SPECIAL_OPTIONS, &temp) == S_OK) {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      process = temp;
    }
  }

  return process;
}

int main() {
  ICLRRuntimeInfo * runtime = GetRuntimeFromVersion(L"v4.0");
  CorBasicEventQueue * queue = NewCBEVQueue();
  CorBasicEvent ev;
  ICorDebug * debugger = CreateDebuggerFromRuntime(runtime, queue);
  ICorDebugProcess * process = CreateDebugProcess(debugger, L"HelloWorld.exe");

  HANDLE debuggerHandle, duppedHandle;
  if(process -> GetHandle(&debuggerHandle) != S_OK) {
    fprintf(stderr, "Get handle failed\n");
    return -1;  
  }
  if(!DuplicateHandle(GetCurrentProcess(), debuggerHandle, GetCurrentProcess(), &duppedHandle, 0, 0, DUPLICATE_SAME_ACCESS)) {
    fprintf(stderr, "Duplicate failed: %d\n", GetLastError());
    return -1;
  }


  while(1) {
    DWORD waitstatus;
    process -> Continue(0);

    waitstatus = WaitForSingleObject(duppedHandle, 0);
    if(waitstatus == WAIT_OBJECT_0)
      break;
  }
  return 0;
}