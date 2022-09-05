Types of shared functions
=========================

Tasks
-----

Tasks are lauched by the main thread and are stored by "reference" in **FUNCREFLOOKTBL** table.
Tasks can access to any global objects.


SelSharedFunc
-------------

Shared functions are passed among threads by its source code.
As running in different Lua's State, they don't know about other objects (including other functions).
They can use **SelShared** variables as well as task list to interact with other threads or triggers other tasks.



Initialize functions
====================

  - **initG_\*()** : functions that initialise **ONCE** internal stuffs (repositories, semaphores, ...)
  - **init_\*()** : functions to expose Lua objects
