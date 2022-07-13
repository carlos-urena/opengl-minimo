.SUFFIXES:
.PHONY: x, clean

ejecutable:=main_macos_exe

x: $(ejecutable)
	./$<

$(ejecutable): ../../src/main.cpp
	clang++  -std=c++11 -Wall -Wfatal-errors -Wno-deprecated-declarations -lglfw -framework OpenGL -o $(ejecutable)  ../../src/main.cpp

clean:
	rm -rf *_exe