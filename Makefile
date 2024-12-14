# global Makefile that is calling sub directories ones

gotoall: all

# Clean previous builds sequels
clean:
	-rm -f lib/Selene/*.so
	-rm -f lib/*.so.2
	-rm -f src/*/*.o

# Build everything
all:
	$(MAKE) -C src/SelPlugins/Curses
	$(MAKE) -C src/libSelene
	$(MAKE) -C src/SeleneCore
	$(MAKE) -C src/SelLog
	$(MAKE) -C src/SelLua
	$(MAKE) -C src/SelScripting
	$(MAKE) -C src/SelElasticStorage
	$(MAKE) -C src/SelMultitasking
	$(MAKE) -C src/SelSharedFunction
	$(MAKE) -C src/SelSharedRef
	$(MAKE) -C src/SelSharedVar
	$(MAKE) -C src/SelMQTT
	$(MAKE) -C src/SelError
	$(MAKE) -C src/SelTimer
	$(MAKE) -C src/SelFIFO
	$(MAKE) -C src/SelEvent
	$(MAKE) -C src/SelCollection
	$(MAKE) -C src/SelAverageCollection
	$(MAKE) -C src/SelTimedCollection
	$(MAKE) -C src/SelTimedWindowCollection
	$(MAKE) -C src/Selene
