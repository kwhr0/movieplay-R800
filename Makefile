all:
	c++ *.cpp -std=c++17 -O3 -o emu800 -L/opt/homebrew/lib -lSDL2
	make -C c
