# Séléné

**Séléné** is a lightweight and versatile framework to build various event driven applications based on **Lua scripts** for their high level.

**Séléné** provides :
* **tasks list** management : heavy tasks or tasks needing to run in sequence are queued and launched when main application thread is available (have a look on **SelShared** objects)
* **Asynchroneous tasks** : tasks can detach from their mother process. Unlike Lua's coroutine, they are working totally independently, without having to manage concurrent access to their own context.
* **data exchange** between tasks are managed using shared variables or data queues (have a look on **SelShared** and **Collection** objects)
* Tasks are waked-up by various types of **events** : **timers** (absolute and relative times), **MQTT** messages arrivals, **files’ events**, **Unix events**, … 
automation tool, driven by end user customized **Lua scripts** and acting on **MQTT data**.

Thanks to plug-ins, **Séléné** allows to easily create events driven dashboard :
* **Text based** ones using the industry standard *Curses library*
* Graphicals ones :
	* Tiny **OLED** displays
	* Linux **Framebuffers** so without having to install and manage obese X layer  

Dependencies
------------

  -	**PAHOc** can be found on [its website](https://eclipse.org/paho/clients/c/).

And if you want to use related plugin (mostly to create smart dashboards) :
  -	**Curses** : the well known text based semi graphics interface
  -	**OLED** : support of SSD1306, SH1106, SSD1327 or SSD1308 small OLED displays. You have to use my [own fork of ArduiPi_OLED](https://github.com/destroyedlolo/ArduiPi_OLED) or derivated as containing lot of additional features compared to its baseline.
  - **libdrm**, **libkms** and **Cairo** and dependancies : needed by *DRMCAIRO* plugin, a graphical framework to build dashboard without having to install X itself.
Notez-bien : **DRMCairo** has now a fallback in case DRM/KMS is not working, using directly the *FrameBuffer*. For the moment, I didn't created a dedicated target without libdrm/libkms even if not used.


Deprecation
-----------

  -	**DirectFB** : X free lightweight graphical framework. After a long and sad story, it seems it definitively lost its official support and being deprecated on most of distribution. It's seem the most updated version is [on this fork](https://github.com/darrengarvey/directfb). DirectFB got less and less support in Linux distribution, source compilation is problematic, I'll not work anymore on this plugin. By the way, it's working pretty well as it is. Replacement is **DRMCairo**

Installation
------------

  1. install dependencies
  2. extract Séléné sources "somewhere" (a.k.a in a temporary directory)
  3. optionally, if you want to change compilation option
    1. install [LFMakeMaker](https://github.com/destroyedlolo/LFMakeMaker)
	2. customise `remake.sh` as per your needs : activate module by settings *USE_DIRECTFB* (deprecated), *USE_DRMCAIRO*, *USE_CURSES* and *USE_OLED*. Optionally, change plugging directory with *PLUGIN_DIR* (not recommended).
	3. execute `remake.sh` to update Makefiles.
  4. `make`
  5. execute `install.sh`
  6. as **root**, run `ldconfig`

---

Unfortunately, I don't have times for a decent documentation. But you may find some informations in :
- all comprehensive examples files in *Selenites????* subdirectories.
- probably in source code as well.

All good wishes are welcome if someone wants to get on with the job.
