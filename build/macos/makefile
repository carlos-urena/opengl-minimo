.SUFFIXES:
.PHONY: x, clean


x: bin/main_exe
	./bin/main_exe

bin/main_exe: ../../src/main.cpp
	clang++ -DMACOS -std=c++11 -framework OpenGL -Wno-deprecated-declarations -lglfw -o bin/main_exe  ../../src/main.cpp

clean:
	rm -rf bin/*_exe