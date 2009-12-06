
all: build

build:
	cd src; make
	cd c_src; make

clean:
	cd src; make clean
	cd c_src; make clean



