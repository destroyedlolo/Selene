# global Makefile that is calling sub directories ones

gotoall: all

# Clean previous builds sequels
clean:
	-rm -f Selene src/testSelene/testSelene
	-rm -f *.so
	-rm -f src/*.o
	-rm -f src/libSelene/*.o

# Build everything
all:
	$(MAKE) -C src/libSelene
	$(MAKE) -C src/SeleneCore
	$(MAKE) -C src/SelLog
	$(MAKE) -C src/SelLua
	$(MAKE) -C src/SelScripting
	$(MAKE) -C src/SelElasticStorage
	$(MAKE) -C src/SelMultitasking
	$(MAKE) -C src/SelSharedFunction
	$(MAKE) -C src/SelMQTT
	$(MAKE) -C src/testSelene
	$(MAKE) -C src/Selene
