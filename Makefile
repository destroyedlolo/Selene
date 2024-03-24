# global Makefile that is calling sub directories ones

gotoall: all

# Clean previous builds sequels
clean:
	-rm -f Selene src/testSelene/testSelene
	-rm -f lib/Selene/*.so
	-rm -f src/*/*.o

# Build everything
all:
	$(MAKE) -C src/libSelene
	$(MAKE) -C src/SeleneCore
	$(MAKE) -C src/SelLog
	$(MAKE) -C src/SelLua
	$(MAKE) -C src/SelScripting
	$(MAKE) -C src/SelElasticStorage
	$(MAKE) -C src/SelMultitasking
	$(MAKE) -C src/SelSharedVar
	$(MAKE) -C src/SelMQTT
	$(MAKE) -C src/SelError
	$(MAKE) -C src/SelTimer
	$(MAKE) -C src/SelFIFO
	$(MAKE) -C src/SelEvent
	$(MAKE) -C src/testSelene
	$(MAKE) -C src/Selene
