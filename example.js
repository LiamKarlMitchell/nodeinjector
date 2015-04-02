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

// With both methods ensure your target process and dll exist.
// And that your dll process and module are all compiled with the correct architecture type.

// Method 1: You want to inject into an already running process that is not protected?

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
  
  
// Method 2: Starting a process suspended with arguments, injecting and resuming the process.
// Note: Has no way to set working directory but you can just use process.chdir, if you want use var old = process.cwd(); to store the current dir.
//       assign the dir you want, call the method then return the working directory.
//       exceptions may be thrown if you have invalid input.
  // Remember if a file path has \ then use \\
  try {
      process.chdir("E:\\Games\\ALT1Games\\TwelveSky1");
	  var injector = require("./build/release/injector");
	  if (injector.executeInject("TwelveSky.exe alt1games.twelvesky1:/test/test/0/18/0/2/1024/768","E:\\Games\\ALT1Games\\TwelveSky1\\TSX_Client.dll")) {
		console.log('Process started and injected.');
	  } else {
		console.log('Unable to execute and inject dll.');
	  }
  } catch (err) {
    console.error('Exception Thrown!');
    console.error(err);
  }