# Debian installation note
=======

This file describes installation on an **Armbian** system and, consequently, may be applicable to any **Debian** derived distribution.

## Important note
---

As the date of writing, Debian disabled **libkms** so DRM only is not available. You have to enable framebuffer fallback to be able to use **DRMCairo** plug-in.

    DRMC_WITH_FB=1

## Dependencies installation
---

* apt install libpaho-mqtt-dev 
   or manually from source [paho/c](https://www.eclipse.org/paho/index.php?page=clients/c/index.php) if not packaged like in Buster
* apt-get install libcairo2-dev libdrm-dev libfreetype6-dev
* apt-get install lua5.1 liblua5.1-0-dev

## compilation
---

Proceed with Séléné installation as described in the main README.
