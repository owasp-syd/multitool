export CC     := gcc
export CFLAGS := -m32 -g

.PHONY: build clean libdisasm

build: multitool
clean:; rm -f *.o *.gch; $(MAKE) -C libdis clean
multitool: multitool.o bgmtree.o libdisasm; gcc -m32 -o $@ *.o libdis/*.o
multitool.o: multitool.c; gcc -m32 -I $(PWD)/libos -c $< -o $@
bgmtree.o: bgmtree.c; gcc -m32 -c $< -o $@
libdisasm:; $(MAKE) -C libdis build
