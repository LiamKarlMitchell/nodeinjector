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

Handle<Value> injectProc(const Arguments& args) {
HandleScope scope;
	//Declare the handle of the process.
HANDLE Process;

//Declare the memory we will be allocating
LPVOID Memory;

//Declare LoadLibrary
LPVOID LoadLibrary; 
 EnableDebugPriv();
	//v8 to std::string
    v8::String::Utf8Value procid4(args[0]->ToString());
    std::string procid3 = std::string(*procid4);   
	//string to int
	int procid2 = atoi(procid3.c_str());
	//int to dword...
	DWORD PID = (DWORD)procid2;
	
	v8::String::Utf8Value dllName3(args[1]->ToString());
    std::string dllName2 = std::string(*dllName3);  
	const char * dll = dllName2.c_str();


if(!PID) {
  return scope.Close(False());
   }
    
  //Open the process with read , write and execute priviledges
   Process = OpenProcess(PROCESS_CREATE_THREAD|PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_VM_OPERATION, FALSE, PID);
  
   //Get the address of LoadLibraryA
   LoadLibrary = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
  
   // Allocate space in the process for our DLL
   Memory = (LPVOID)VirtualAllocEx(Process, NULL, strlen(dll)+1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  
   // Write the string name of our DLL in the memory allocated
   WriteProcessMemory(Process, (LPVOID)Memory, (void *)dll, strlen(dll)+1, NULL);
  
   // Load our DLL, by forcing the process to call LoadLibrary("mydll.dll");
   CreateRemoteThread(Process, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibrary, (LPVOID)Memory, NULL, NULL);
  
   //Let the program regain control of itself
   CloseHandle(Process);


  //Lets free the memory we are not using anymore.
   VirtualFreeEx(Process , (LPVOID)Memory , 0, MEM_RELEASE);

  return scope.Close(True());
}

Handle<Value> getprocID(const Arguments& args) {
HandleScope scope;
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
			
			 return scope.Close(Integer::New(entry.th32ProcessID));
                 CloseHandle(snapshot);
            }
        }
    }
  return scope.Close(False());
    CloseHandle(snapshot);
}





void init(Handle<Object> exports) {
	  //get process id
	  	    exports->Set(String::NewSymbol("getprocID"),
      FunctionTemplate::New(getprocID)->GetFunction());
	  //inject process
	  	  	    exports->Set(String::NewSymbol("inject"),
      FunctionTemplate::New(injectProc)->GetFunction());
	 
}

NODE_MODULE(injector, init)