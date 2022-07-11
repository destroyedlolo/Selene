# This script will regenerate documentation


echo "Selene own library"
echo "------------------"

ldoc -c docs/ldoc/Selene.ld src/SeleneLibrary

echo
echo "Curses plug-in"
echo "--------------"

ldoc -c docs/ldoc/Curses.ld src/SelPlugins/Curses

echo
echo "DirectFB plug-in"
echo "----------------"

ldoc -c docs/ldoc/DirectFB.ld src/SelPlugins/DirectFB

echo
echo "OLed plug-in"
echo "------------"

ldoc -c docs/ldoc/OLed.ld src/SelPlugins/OLED

