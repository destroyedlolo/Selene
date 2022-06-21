# This script will regenerate documentation


echo "Selene own library"
echo "------------------"

ldoc -v --project "Selene library" --title "Selene own library" --format markdown src/SeleneLibrary
