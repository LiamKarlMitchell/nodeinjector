/* 
	node.js dll injector
    Copyright (C) 2014 undido

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#include <node.h>
#include <v8.h>
#include <Windows.h>
#include <string>
#include <cstdio>
#include <tlhelp32.h>
#include <iostream>

using namespace v8;

void EnableDebugPriv()
{
    HANDLE hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkp;

    OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
    LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL);

    CloseHandle(hToken); 
}

bool injectDLL(DWORD dwProcessId, LPCSTR lpszDLLPath)
{
  HANDLE  hProcess, hThread;
  LPVOID  lpBaseAddr, lpFuncAddr;
  DWORD   dwMemSize, dwExitCode;
  BOOL    bSuccess = false;
  HMODULE hUserDLL;

  if((hProcess = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_OPERATION
    |PROCESS_VM_WRITE|PROCESS_VM_READ|THREAD_QUERY_INFORMATION, FALSE, dwProcessId)))
  {   
    dwMemSize = lstrlenA(lpszDLLPath) + 1;
    if((lpBaseAddr = VirtualAllocEx(hProcess, NULL, dwMemSize, MEM_COMMIT, PAGE_READWRITE)))
    {     
      if(WriteProcessMemory(hProcess, lpBaseAddr, lpszDLLPath, dwMemSize, NULL))
      { 
        hUserDLL = LoadLibrary(TEXT("kernel32.dll"));
        if (hUserDLL == NULL) {
          hUserDLL = LoadLibrary(TEXT("kernelbase.dll"));
        }
        if(hUserDLL)
        {         
          if((lpFuncAddr = GetProcAddress(hUserDLL, "LoadLibraryA")))
          {           
            if((hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpFuncAddr, lpBaseAddr, 0, NULL)))
            {
              WaitForSingleObject(hThread, INFINITE);
              if(GetExitCodeThread(hThread, &dwExitCode)) {
                bSuccess = (dwExitCode != 0) ? true : false;
              }
              CloseHandle(hThread);
            }
          }
          FreeLibrary(hUserDLL);
        }
      }
      VirtualFreeEx(hProcess, lpBaseAddr, 0, MEM_RELEASE);
    }
    CloseHandle(hProcess);
  }
  return bSuccess;
}

void injectProc(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

 if (args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments. Expects Process ID and string DLL path.")));
    return;
  }

  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments. Process ID should be a number.")));
    return;
  }

  EnableDebugPriv();
  //v8 to std::string
  String::Utf8Value procid4(args[0]->ToString());
  std::string procid3 = std::string(*procid4);   
  //string to int
  int procid2 = atoi(procid3.c_str());
  //int to dword...
  DWORD PID = (DWORD)procid2;
	
  String::Utf8Value dllName3(args[1]->ToString());
  std::string dllName2 = std::string(*dllName3);  
  const char * dll = dllName2.c_str();

  if(!PID) {
    return args.GetReturnValue().Set(false);
  }

  if (injectDLL(PID, dll)) {
    return args.GetReturnValue().Set(true);
  }

  return args.GetReturnValue().Set(false);
}

void getprocID(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

   if (args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments. Expects string 'Process Name'.")));
    return;
  }

  v8::String::Utf8Value procn(args[0]->ToString());
  std::string y2 = std::string(*procn);  
	const char * procname = y2.c_str();
	
	EnableDebugPriv();

  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

  if (Process32First(snapshot, &entry) == TRUE)
  {
      while (Process32Next(snapshot, &entry) == TRUE)
      {
          if (stricmp(entry.szExeFile, procname) == 0)
          {
             CloseHandle(snapshot);
		     return args.GetReturnValue().Set(false);
          }
      }
  }
  CloseHandle(snapshot);
  return args.GetReturnValue().Set(false);
}

void executeInject(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  
 if (args.Length() < 2) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments. Expects string 'executablefile & arguments' and string 'dllpath'.")));
    return;
  }

  std::string launchString(*v8::String::Utf8Value(args[0]->ToString()));
  std::string dllPath(*v8::String::Utf8Value(args[1]->ToString()));

  EnableDebugPriv();

  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  PROCESS_INFORMATION processInfo;
  STARTUPINFO info={sizeof(info)};

  if(CreateProcessA(NULL,(LPSTR)launchString.c_str(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &info, &processInfo))
  {
	  if (injectDLL(processInfo.dwProcessId, dllPath.c_str()  )) {
      if (ResumeThread((HANDLE)processInfo.hThread) != -1) {
		   return args.GetReturnValue().Set(true);
	     }
    }

    // If we get here then there was a problem injecting the dll.
    // Kill process to prevent leaving a suspended process hanging around.
    TerminateProcess(processInfo.hProcess,1);
  }

  return args.GetReturnValue().Set(false);
}


void init(Handle<Object> exports) {
	NODE_SET_METHOD(exports, "getprocID", getprocID);
	NODE_SET_METHOD(exports, "inject", injectProc);
	NODE_SET_METHOD(exports, "executeInject", executeInject);
}

NODE_MODULE(injector, init)