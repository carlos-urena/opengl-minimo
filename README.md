# opengl-minimo

Un ejemplo de un código fuente OpenGL (ver 2.1), con funcionalidad mínima, en C++ (versión 2011). 
Usa [GLFW 3](https://github.com/glfw/glfw) para eventos y ventanas. Usa shaders básicos escritos en GLSL (ver 1.2). 
Incluye _makefile_ para macos y linux. 

## Requisitos en Linux 

En linux es necesario tener instalado el compilador de C++ de GNU o del proyecto LLVM. 
Además se deben instalar los _packages_ **libglew-dev** y **glfw3-dev**, se puede hacer con:

```
sudo apt install libglew-dev
sudo apt install glfw3-dev
```

Para compilar y ejecutar, hay que situarse en la carpeta `build/linux` y escribir `make`

## Requisistos en macOS

En ordenadores macOS hay que tener instalada la herramienta de desarrollo de **XCode**. 
Este herramienta de desarrollo incorpora el compilador de C++ del proyecto LLVM, así como el _framework_ de **OpenGL**.
Usando la herramienta **homebrew** para mac, se puede instalar fácilmente la librería **GLFW**, con:

```
brew install glfw
```

Si no se quiere usar **homebrew**, se puede descargar **cmake** para macOS ([https://cmake.org/download/](https://cmake.org/download/)).

Una vez que se tiene disponible la orden `cmake`, se puede descargar el archivo _zip_ de GLFW para macOS del sitio web de GLFW ([ http://www.glfw.org/download.html](http://www.glfw.org/download.html)), después se abre el archivo en una carpeta nueva vacía, y al abrirlo se crea un subcarpeta de nombre `glfw-...` (los caracteres concretos en el lugar de los puntos suspensivos dependen de la versión, estos fuentes requieren la versión 3). Finalmente se puede compilar la librería de esta forma:

```
cd glfw-....
cmake -DBUILD_SHARED_LIBS=ON .
make
sudo make install
```

Para compilar y ejecutar, hay que situarse en la carpeta `build/macos` y escribir `make`


## Compilación y ejecución (Linux o macOS)

Para compilar y ejecutar, nos situamos en la carpeta `build/linux` o `build/macos` (según el sistema operativo), y hacemos `make`. Esto compila si es necesario, y luego ejecuta el programa.

