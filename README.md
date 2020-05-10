# Séléné

**Séléné** is a lightweight and versatile automation tool, driven by end user customized **Lua scripts** and acting on **MQTT data**.

Dependencies
------------

  -	**PAHOc** can be found on [its website](https://eclipse.org/paho/clients/c/).

And if you want to use related plugin (mostly to create smart dashboards) :
  -	**Curses** : the well known text based semi graphics interface
  -	**OLED** : support of SSD1306, SH1106, SSD1327 or SSD1308 small OLED displays. You have to use my [own fork of ArduiPi_OLED](https://github.com/destroyedlolo/ArduiPi_OLED) or derivated as containing lot of additional features compared to its baseline.
  - **libdrm**, **libkms** and **Cairo** and dependancies : needed by *DRMCAIRO* plugin, a graphical framework to build dashboard without having to install X itself.


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
