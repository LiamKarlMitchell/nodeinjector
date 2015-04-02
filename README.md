Building
===================
To build this node.js module you will need to use node-gyp https://github.com/TooTallNate/node-gyp
This module was developed using c++ 2010 if you want to build this module to inject processes with a 64 bit architecture
you will need to install the 64 bit version of node.js and download windows SDK http://msdn.microsoft.com/en-ca/windows/bb980924.aspx

If building for node-webkit look at their guide https://github.com/nwjs/nw-gyp

I had to do this

```
nw-gyp configure --target=v0.12.0 --msvs_version=2012
nw-gyp build --arch=ia32
```

Compiled on Windows 7 x64 as a 32bit binary.


What can this be used for?
===================
node dll injector can be used to inject dlls into programs currently running on your computer and run code in the targeted program
It can inject both 32 bit and 64 processes but requires that both the dll and module be also compiled in 64 bit or else it will not 
accept the dll. So... if you want to inject either a 32 bit program or a 64 program it will require you to use either 64-bit nodejs 
and a 64 bit compiled version of this module or a 32-bit version of node.js and the module.

How do I use this?
===================
For an example see example.js

Basically we first get the process ID of the running program like so:

		var injector = require("./build/release/injector");
		var procname = "ac_client.exe"; 
		//get a process id returns false on failure
		var procid = injector.getprocID(procname);



After we have gotten the process ID we inject the process by process id


	/*injector.inject will return false on failure it does not know if the program will actually 
	run it will return if it injected the dll but it can't tell if the injected dll will actually run */
	var inj = injector.inject(procid, "C:\dlls\test.dll");

LICENSE
===================
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
