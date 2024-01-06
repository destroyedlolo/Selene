# global Makefile that is calling sub directories ones

gotoall: all

# Clean previous builds sequels
clean:
	-rm Selene
	-rm *.so
	-rm src/*.o
	-rm src/libSeleneLibrary/*.o

# Build everything
all:
	$(MAKE) -C src/libSelene
	$(MAKE) -C src/SeleneCore
	$(MAKE) -C src/
