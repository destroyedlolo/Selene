This file contains some help related to linux' framebuffer settings

Screen resolution
=====

List of available resolution
----

```
# cat /sys/class/graphics/fb1/modes
U:720x480p-196
U:720x480p-60
D:1920x1080p-50
D:1280x720p-50
D:1920x1080i-50
D:1920x1080i-60
D:720x576p-50
V:1280x1024p-75
V:1024x768p-70
V:1024x768p-60
V:800x600p-75
V:800x600p-72
V:800x600p-60
V:800x600p-56
V:640x480p-75
V:640x480p-72
V:640x480p-60
D:800x480p-60
```

With :
  * **U** : undefined
  * **D** : detailed from EDID's timing descriptors
  * **V** : from VESA

Set resolution
----

```
echo "D:800x480p-60" > /sys/class/graphics/fb1/mode
```

