Types of shared functions
=========================

This document describes the differences between tasks and SelShared functions.

Tasks
-----

Tasks are lauched within the main thread and are stored by "reference" in **FUNCREFLOOKTBL** table.
Tasks can access to any global objects.

Typical waiting loop looks like :
```` lua
while true do
	local rt = table.pack( Selene.WaitFor() )
	for _,ret in ipairs(rt) do
		if type(ret) == 'function' then
			ret()
		end
	end
end
````

**Selene.WaitFor()** waits for MQTT events (as no other argument is provided) and return a list (table) of functions to run. The *for* loop launch them one by one.

See [MQTT.sel](../Selenites/MQTT.sel) for a complete example.

SelSharedFunc
-------------

Shared functions are passed among threads by its source code.
As running in different Lua's State, they don't know about other objects (including other functions).
They can use **SelShared** variables as well as task list to interact with other threads or triggers other tasks.



Initialize functions
====================

  - **initG_\*()** : functions that initialise **ONCE** internal stuffs (repositories, semaphores, ...)
  - **init_\*()** : functions to expose Lua objects
