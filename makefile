all: compile run

build:; mkdir build
cmake: build; cd build && cmake ..
compile: cmake; make -C build
clean: cmake; make -C build clean
run:; build/htm
