# This script will regenerate documentation


echo "Selene own library"
echo "------------------"

ldoc -c docs/Selene.ld src/SeleneLibrary

echo
echo "Curses plug-in"
echo "--------------"

ldoc -c docs/Curses.ld src/SelPlugins/Curses

echo
echo "DirectFB plug-in"
echo "----------------"

ldoc -c docs/DirectFB.ld src/SelPlugins/DirectFB

