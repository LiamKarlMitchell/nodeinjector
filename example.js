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
		var injector = require("./build/release/injector");
		var procname = "ac_client.exe"; 
		//get a process id returns false on failure
		var procid = injector.getprocID(procname);
		
if (procid != false){
	console.log("Found process '" + procname + "' with PID:" + procid);
	//inject dll into the process ID returns true on success returns false on failure
	var inj = injector.inject(procid, "C:\dlls\test.dll");
		if (inj){
			console.log("Injected process with dll");
		}
} else {
	console.log("Process not found!");
}
              