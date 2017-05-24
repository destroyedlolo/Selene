# Séléné

**Séléné** is a lightweight and versatile automation tool, driven by end user customized **Lua scripts** and acting on **MQTT data**.
Optionnal pluggins "**DirectFB**" or "**Curses**" can be used to create graphical applications :

  - **DirectFB** to create hires graphical applications using Linux Framebuffer (in other words, whiteout needing heavy X stack)
  - **Curses** to create character driven lowres application than can be used from a simple textual shell interface.

----
My typicals use cases, for my own smart home :

  - [Smart home dashboard](https://github.com/destroyedlolo/HomeDashboard/tree/master/SeleniteDFB) on a recycled to Linux bricked tablet (Séléné + DirectFB pluggin)
    - updates in real time HiRes graphical interface based on received MQTT messages
    - historizes and graphs thoses data
  - [smart home control](https://github.com/destroyedlolo/HomeDashboard/tree/master/SeleniteCurses) using text interface (Séléné + Curses pluggin)
    - displays in real time received MQTT data
    - interactive text based interface to send commands and orders thru MQTT messages to other smarthome components
  - [smart home automation](https://github.com/destroyedlolo/Majordome) "*Majordome*" (Séléné)
    - creates some scenario based on MQTT realtime data, provided consignes and external events.

---

Dependencies
------------

  - For **DirectFB** and **Curses**, please have a look on your distribution packages management system.
  -	**PAHOc** can be found on [its website](https://eclipse.org/paho/clients/c/).

---

Unfortunately, I don't have times for a decent documentation. But you may find some informations in :
- *Doc* file that is only an dirty draft of what could evolve to a decent documentation :)
- all comprehensive examples files in *Selenites*, *SelenitesCurses* and *SelenitesDFB*
