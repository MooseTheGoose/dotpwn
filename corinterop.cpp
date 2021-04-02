/*
 *  A really basic unmanaged interop file to include
 *  in C# to make the whole COM interop process easier
 *  and less shim-y and excruciating.
 *
 *  Compile with 'cl corinterop.cpp CorGuids.lib MSCorEE.lib'
 */

#include <windows.h>
#include <cor.h>
#include <CorDebug.h>
#include <MetaHost.h>
#include <stdlib.h>
#include <stdio.h>

/* So much pain */
class CorInteropManagedCallback : ICorDebugManagedCallback2 , ICorDebugManagedCallback {
  private:
  ULONG refcnt;

  public:
  CorInteropManagedCallback() {
    refcnt = 0;
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
    printf("HI from CreateProcess\n");
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
    exit(0);
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

/* Actually, this one isn't so bad. */
class CorInteropUnmanagedCallback : ICorDebugUnmanagedCallback {
  private:
  ULONG refcnt;
  ICorDebugProcess ** pproc;

  public:
  CorInteropUnmanagedCallback(ICorDebugProcess ** pproc) {
    refcnt = 0;
    this -> pproc = pproc;
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

#define VERS_MAX_LEN 255

int wmain(int argc, wchar_t *argv[]) {
  HRESULT comStatus;
  ULONG len;
  ICLRMetaHost * host;
  ICLRRuntimeInfo * runtime;
  ICorDebug * debugger;
  ICorDebugProcess * proc;
  IEnumUnknown * runtimeEnum;
  wchar_t versBuffer[VERS_MAX_LEN + 1];
  wchar_t * vers = L"v4.0.30319";
  STARTUPINFOW si;
  PROCESS_INFORMATION pi;

  if(argc != 2) {
    fprintf(stderr, "Usage: %ls exename\n", argv[0]);
    return -1;
  }

  comStatus = CLRCreateInstance(CLSID_CLRMetaHost, IID_ICLRMetaHost, (LPVOID *)&host);
  if(comStatus != S_OK) {
    fprintf(stderr, "Failed to get CLR Metadata Host\n");
    exit(-1);
  }
  comStatus = host -> EnumerateInstalledRuntimes(&runtimeEnum);
  if(comStatus != S_OK) {
    fprintf(stderr, "Failed to enumerate CLRs\n");
    exit(-1);
  }

  runtime = 0;
  while(runtimeEnum -> Next(1, (IUnknown **)&runtime, &len) == S_OK) {
    DWORD maxLen = VERS_MAX_LEN;
    runtime -> GetVersionString(versBuffer, &maxLen);
    printf("%ls\n", versBuffer);
    if(wcscmp(versBuffer, vers) == 0)
      break;
    runtime -> Release();
    runtime = 0;
  }
  if(!runtime) {
    fprintf(stderr, "Failed to get %ls runtime\n", vers);
    exit(-1);
  }

  comStatus = runtime -> GetInterface(CLSID_CLRDebuggingLegacy, IID_ICorDebug, (LPVOID *)&debugger);
  if(comStatus != S_OK) {
    fprintf(stderr, "Failed to create debugger");
    exit(-1);
  }
  debugger -> Initialize();
  debugger -> SetManagedHandler((ICorDebugManagedCallback *)new CorInteropManagedCallback());
  debugger -> SetUnmanagedHandler((ICorDebugUnmanagedCallback *)new CorInteropUnmanagedCallback(&proc));  

  runtimeEnum -> Release();
  runtime -> Release();
  host -> Release();

  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(si);

  comStatus = debugger -> CreateProcess(0, argv[1], 0, 0, 0, CREATE_NEW_CONSOLE, 0, 0, &si, &pi, DEBUG_NO_SPECIAL_OPTIONS, &proc);
  if(comStatus != S_OK) {
    fprintf(stderr, "Failed to create process. Error code: %lx\n", comStatus);
    exit(-1);
  }

  /* Run this. Don't you wish VS debugger was this fast? */
  while(1) {
    proc -> Continue(0);
  }

  return 0;
}