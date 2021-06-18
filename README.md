# opengl-minimo

Un ejemplo de un código fuente OpenGL (ver 2.1), con funcionalidad mínima, en C++ (versión 2011). 
Usa [GLFW 3](https://github.com/glfw/glfw) para eventos y ventanas.
Incluye _makefile_ para macos y linux. 

## Requisitos y compilación en Linux 

En linux es necesario tener instalado el compilador de C++ de GNU o del proyecto LLVM. 
Además se deben instalar los _packages_ 'libglew-dev' y 'glfw3-dev', se puede hacer con:

```
sudo apt install libglew-dev
sudo apt install glfw3-dev
```

Para compilar y ejecutar, hay que situarse en la carpeta `build/linux` y escribir `make`

## Requisistos y compilación en macOS

En ordenadores macOS hay que tener instalada la herramienta de desarrollo de **XCode**. 
Además, se debe instalar **cmake**, disponible en esta Web: 

Y la librerí GLFW, 

