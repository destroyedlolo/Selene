# Séléné

**Séléné** is a lightweight and versatile framework to build **Lua** event driven application.

**Séléné** provides :

* **tasks list** management : tasks needing to run in sequence or which doesn't need to be real-time are queued. They will be launched when main application thread is idle
* **Asynchroneous tasks** : tasks can detach from their mother process. Unlike Lua's coroutine, they are working totally independently, without having to manage concurrent access to their own context. Especially suitable for realtime actions.
* **data exchange** between tasks are managed using shared variables or data queues (have a look on **SelShared** and **Collection** objects). Notez-bien : due to Lua's limitation, *detached tasks* can't access to objects (including functions) declared in the main thread, Shared objects and collection handle data sharing as well as locking to avoid race condition and concurrent access.
* Tasks are waked-up by various types of **events** : 
	* **timers** (absolute and relative times)
	* **MQTT** messages arrivals. Consequently, an external application can trigger a tasks by sending an MQTT message. Séléné provides some APIs to expose MQTT payload to Lua scripts and can send new messages.
	* **files’ events**
	* **Unix events**
	* … 

Thanks to plug-ins, **Séléné** allows to easily create events driven dashboard :
* **Text based** ones using the industry standard *Curses library*
* Graphicals ones :
	* Tiny **OLED** displays
	* Linux **Framebuffers** so without having to install and manage obese X layer  

Dependencies
------------

  -	**PAHOc** can be found on [its website](https://eclipse.org/paho/clients/c/). Mandatory to handle MQTT layer.

And if you want to use related plugin (mostly to create smart dashboards) :
  -	**Curses** : the well known text based semi graphics interface
  -	**OLED** : support of SSD1306, SH1106, SSD1327 or SSD1308 small OLED displays. You have to use my [own fork of ArduiPi_OLED](https://github.com/destroyedlolo/ArduiPi_OLED) or derivated as containing lot of additional features compared to its baseline.
  - **libdrm**, **libkms** and **Cairo** and dependancies : needed by *DRMCAIRO* plugin, a graphical framework to build dashboard without having to install X itself.
Notez-bien : **DRMCairo** has now a fallback in case DRM/KMS is not working, using directly the *FrameBuffer*.

My systems are mostly under **Linux/Gentoo**, but one of my *SBC* is running **Armbian**, I wrote a special [installation note](docs/Devian_Installation.md) for **Debian** and derived.


Deprecation
-----------

  -	**DirectFB** : X free lightweight graphical framework. After a long and sad story, it seems it definitively lost its official support and being deprecated on most of distributions. It's seem the most updated version is [on this fork](https://github.com/darrengarvey/directfb). 
  
  DirectFB got less and less support in Linux distributions, source compilation is problematic, I'll not work anymore on this plugin. By the way, it's working pretty well as it is.
Replacement is **DRMCairo**.

Installation
------------

  1. install dependencies
  1. extract Séléné sources "somewhere" (a.k.a in a temporary directory)
  1. optionally, if you want to change compilation option
     1. install [LFMakeMaker](https://github.com/destroyedlolo/LFMakeMaker)
     1. customise `remake.sh` as per your needs : activate module by settings *USE_DIRECTFB* (deprecated), *USE_DRMCAIRO*, *USE_CURSES* and *USE_OLED*. Optionally, change plugging directory with *PLUGIN_DIR* (not recommended).
     1. execute `remake.sh` to update Makefiles.
  1. `make`
  1. execute `install.sh`
  1. as **root**, run `ldconfig`

---

Even if I don't have time for a descent fully featured documentation, you may find some informations in :
- **Doc subdirectory** especially regarding Lua's exposed API
- all comprehensive examples files in **Selenites????** subdirectories.
- probably in source code as well.

All good wishes are welcome if someone wants to get on with the job.
