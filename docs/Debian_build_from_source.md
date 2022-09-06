# Debian installation from source
=======

This file describes installation on an **Armbian** system and, consequently, may be applicable to any **Debian** derived distribution.

## Important notes
---

1. As the date of writing, Debian disabled **libkms** so DRM only is not available. You have to enable framebuffer fallback to be able to use **DRMCairo** plug-in.

    DRMC_WITH_FB=1

1. I wasn't able to make Selene working with **Bullseye** : it seems it's due to the way Debian compiled GCC 10 (it's not a problem with GCC itself as it's working without trouble on Gentoo).

## Dependencies installation
---

* `apt install libpaho-mqtt-dev`
   or manually from source [paho/c](https://www.eclipse.org/paho/index.php?page=clients/c/index.php) if not packaged (**Buster**)
* `apt-get install libcairo2-dev libdrm-dev libfreetype6-dev`
* `apt-get install lua5.1 liblua5.1-0-dev` (I didn't tested with other Lua's version on Armbian but Séléné itself works with most recent Lua versions,5.2, 5.3, 5.4 ...)

## compilation
---

Proceed with Séléné installation as described in the main README.
