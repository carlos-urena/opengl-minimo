# opengl-minimo

Un ejemplo de un código fuente OpenGL (ver 2.1), con funcionalidad mínima, en C++ (versión 2011).
Usa [GLFW 3](https://github.com/glfw/glfw) para eventos y ventanas. Hay dos versiones:

* Una que únicamente usa el cauce fijo.
* Una que usa cauce programable (shaders básicos escritos en GLSL, versión 1.2.

Incluye _makefile_ para macos y linux.

## Requisitos en Linux

En linux es necesario tener instalado el compilador de C++ de GNU o del proyecto LLVM.
Además se deben instalar los _packages_ **libglew-dev** y **glfw3-dev**, se puede hacer con:

```
sudo apt install libglew-dev
sudo apt install libglfw3-dev
```

## Requisistos en macOS

En ordenadores macOS hay que tener instalada la herramienta de desarrollo de **XCode**.
Este herramienta de desarrollo incorpora el compilador de C++ del proyecto LLVM, así como el _framework_ de **OpenGL**.

### Descarga e instalación de GLFW sin _homebrew_

Usando la herramienta **homebrew** para mac, se puede instalar fácilmente la librería **GLFW**, con:

```
brew install glfw
```


### Descarga e instalación de GLFW sin _homebrew_

Si no se quiere usar **homebrew**, se puede descargar y compilar GLFW directamente. Para ello será necesario previamente descargar e instalar **cmake** para macOS ([https://cmake.org/download/](https://cmake.org/download/)). Si todo va bien, la orden `cmake` estará disponible.

Una vez que se tiene disponible la orden `cmake`, se puede descargar el archivo _zip_ de GLFW para macOS del sitio web de GLFW ([http://www.glfw.org/download.html](http://www.glfw.org/download.html)), después se abre el archivo en una carpeta nueva vacía, y al abrirlo se crea un subcarpeta de nombre `glfw-...` (los caracteres concretos en el lugar de los puntos suspensivos dependen de la versión, estos fuentes requieren la versión 3). Finalmente se puede compilar la librería de esta forma:

```
cd glfw-....
cmake -DBUILD_SHARED_LIBS=ON .
make
sudo make install
```

## Compilación y ejecución (Linux o macOS)

Para compilar y ejecutar, nos situamos en la carpeta `cauce-fijo/build/linux` o `cauce-fijo/build/macos` (según el sistema operativo), y hacemos `make`. Esto compila si es necesario, y luego ejecuta el programa, en la versión del cauce fijo. Igualmente se puede hacer para la versión del cauce programable, usando la carpeta `cauce-programable`.

## Requisitos y compilación en Windows

En Windows es posible compilar y ejecutar estos programas, para ello será necesario tener instalada la última versión (gratuita) de la herramienta de desarrollo **Visual Studio** de Microsoft. Además de esto, será necesario tener instalados los archivos de cabeceras (`.h`,`.hpp`) y librerías (DLLs) de OpenGL, GLFW (versión 3 o posteriores) y GLEW. Esto último se puede hacer con el gestor de paquetes **NuGet** (https://www.nuget.org/), que está incorporado a Visual Studio. Para más detalles se puede consultar las páginas de estos paquetes en el sitio web de NuGet:

* OpenGL: paquete `nupengl.core` (https://www.nuget.org/packages/nupengl.core/)
* GLEW: paquete `glew` (https://www.nuget.org/packages/glew/)
* GLFW: paquete `glfw` (https://www.nuget.org/packages/glfw/)

El código actual de este repositorio no está adaptado a Windows, para ello será necesario adaptar las directivas _include_, aunque no debe haber cambio alguno en las llamadas a OpenGL y el resto de sentencias del programa. En este respositorio no se proporcionan _makefiles_ ni archivos de proyecto para Visual Studio.

Hay que tener en cuenta que en este momento (sept. 2021) no es todavía posible usar WSL2 en Windows para ejecutar archivos ejecutables de Linux que accedan a la GPU o al sistema de ventanas. Sí es posible usar máquinas virtuales ejecutando Linux como invitado sobre Windows como anfitrión, pero se debe configurar el software de virtualización para asegurarnos que el SO invitado tiene acceso a la GPU.
