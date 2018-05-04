
all:
	$(MAKE) -C src/SeleneLibrary
	$(MAKE) -C src/SelPlugins/Curses
	$(MAKE) -C src/SelPlugins/DirectFB
	$(MAKE) -C src
