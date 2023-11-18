General installation procedure from source
==========================================

This file describes the *general* way to install Séléné from source.

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
  1. ensure `/usr/local/lib` is in your `/etc/ld.so.conf`
  1. as **root**, run `ldconfig`


