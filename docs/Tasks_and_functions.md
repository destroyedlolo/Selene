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

**Typical usage :** 
 * tasks that can be delayed and having to interact with the global environment. For example, code to updated GUI when new data arrive.

SelSharedFunc
-------------

Shared functions are running in parallel threads, with a dedicated state. Consequently, **they can't access to any global objects, including other functions**. Use **SelShared** variables as well as tasks lists to interact with other threads.

Shared functions are passed among threads by its source code.

**Typical usage :** 
 * functions for **immediate** actions when an even arrives, 
 * long standing background processing,
 * queuing of incoming data. Don't forget there is no guaranty a *task* can be launched before the arrival of a new set of data : in race condition, data may be lost as only the last received data is accessible to a *task*. To avoid this situation, the solution is to create a *function* that will add incoming data in a queue/collection and then push a *task* in the waiting list to process them. This *task* has to loop among all queued data before exiting.
 * everything that needs to run in parallel.

Again [MQTT.sel](../Selenites/MQTT.sel) for a complete example.
