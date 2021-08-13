# builds
all: debug

debug:
	cd build/ && make config=debug

release:
	cd build/ && make config=release

# build system utils
update:
	git submodule foreach --recursive git fetch && git pull

premake:
	cd build/ && premake5 gmake2

clean:
	cd build/ && make clean
