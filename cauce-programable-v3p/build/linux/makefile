.SUFFIXES:
.PHONY: x, clean

ejecutable:=main_linux_exe

x: $(ejecutable)
	./$<

$(ejecutable): ../../src/main.cpp
	g++  -std=c++11  ../../src/main.cpp -o $(ejecutable) -lGLEW -lGLU -lGL -lglfw 

clean:
	rm -rf $(ejecutable)