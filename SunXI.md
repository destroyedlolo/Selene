If compiled on SunXI architecture (Allwinner chips) where the compiler can find **sunxi_disp_ioctl.h** you will got additionals **DCCard** features :

* **Open()** and **OpenFB()** will guess the screen number based on the file provided (i.e /dev/fb0 means we are on screen 0)
* New methodes **On()** and **Off()** to turn the screen on and off

This file *may be* available on your kernel tree, otherwise, you can get if from this [link](https://github.com/ssvb/xf86-video-fbturbo/blob/master/src/sunxi_disp_ioctl.h).
