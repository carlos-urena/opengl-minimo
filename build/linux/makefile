.SUFFIXES:
.PHONY: x, clean


x: bin/main_exe
	./bin/main_exe

bin/main_exe: ../../src/main.cpp
	g++ -DLINUX -std=c++11  ../../src/main.cpp -o bin/main_exe -lGLEW -lGLU -lGL -lglfw 

clean:
	rm -rf bin/*_exe