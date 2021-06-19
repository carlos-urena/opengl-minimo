.SUFFIXES:
.PHONY: x, clean


x: bin/main_exe
	./bin/main_exe

bin/main_exe: ../../src/main.cpp
	clang++ -DMACOS -std=c++11 -Wall  -Wno-deprecated-declarations -lglfw -framework OpenGL -o bin/main_exe  ../../src/main.cpp

clean:
	rm -rf bin/*_exe