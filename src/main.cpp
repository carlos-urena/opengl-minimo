// Ejemplo mínimo de código OpenGL, usa OpenGL 2.1 + GLSL 1.2
//
// Ver referencia de la API aquí:
// ver: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/
//

#include <cassert>  // para 'assert' (verificación de condiciones lógicas)
#include <cstring>  // para 'strlen' (al compilar shaders)
#include <iostream> 

#ifdef LINUX 
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h> 
#else 
#ifdef MACOS
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLFW/glfw3.h>  

#define glGenVertexArrays  glGenVertexArraysAPPLE
#define glBindVertexArray  glBindVertexArrayAPPLE
#else 
#pragma error "no está definido el sistema operativo"
#endif
#endif


// ---------------------------------------------------------------------------------------------
// Constantes y variables globales 

bool 
    redibujar_ventana   = true ,  // puesto a true por los gestores de eventos cuando cambia el modelo y hay que regenerar la vista 
    terminar_programa   = false ;  // puesto a true en los gestores de eventos cuando hay que terminar el programa
GLFWwindow *                      
    ventana_glfw        = nullptr; // puntero a la ventana GLFW
int 
    ancho_actual        = 512 ,      // ancho actual del framebuffer, en pixels
    alto_actual         = 512 ;      // alto actual del framebuffer, en pixels

GLint 
    loc_matriz_transf_coords  ;

GLenum 
    id_vao = 0 ; // identificador de VAO (vertex array object)


const GLfloat mat_ident[] =     // matriz 4x4 identidad
    {   1.0, 0.0, 0.0, 0.0, 
        0.0, 1.0, 0.0, 0.0, 
        0.0, 0.0, 1.0, 0.0, 
        0.0, 0.0, 0.0, 1.0, 
    } ;

const GLfloat mat_despl[] =     // matriz 4x4 desplazamiento
    {   1.0, 0.0, 0.0, 0.3, 
        0.0, 1.0, 0.0, 0.0, 
        0.0, 0.0, 1.0, 0.0, 
        0.0, 0.0, 0.0, 1.0, 
    } ;

// ---------------------------------------------------------------------------------------------
// Fuentes para el vertex shader y el fragment shader 

// fuente para el vertex shader sencillo, se invoca una vez por cada vértice. 
// su función es escribir en 'gl_Position' las coordenadas del vértice 
// además, puede escribir otros atributos del vértice en variables 'varying'
// (en este caso escribe el color en 'v_color')

const char * const fuente_vs = R"glsl(
    #version 120
    
    uniform mat4 u_matriz_transf_coords; // variable uniform: matriz de transformación de coordenadas
    varying vec4 v_color ; // variable de salida: color del vértice

    void main() 
    {
        v_color     = gl_Color ; // el color del vértice es el enviado por la aplicación.
        gl_Position = u_matriz_transf_coords *  gl_Vertex;  // transforma coordenadas   
    }
)glsl";

// fuente para el fragment shader sencillo: se invoca una vez por cada pixel.
// su función es escribir en 'gl_FragColor' el color del pixel 

const char * const fuente_fs = R"glsl(
    #version 120
    
    varying vec4 v_color ; // variable de entrada: color interpolado.
    
    void main() 
    {
        gl_FragColor = v_color ; // el color del pixel es el color interpolado
    }
)glsl";

// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo diferido

void DibujarTrianguloMD( ) 
{
     assert( glGetError() == GL_NO_ERROR );

    // número de vértices que se van a dibujar
    constexpr unsigned
        num_verts = 3 ;

    if ( id_vao == 0 )
    {
        // tablas de coordenadas y colores de vértices (coordenadas en 2D, con Z=0) 
        const GLfloat 
            coordenadas[ num_verts*2 ] = {  -0.8, -0.8,      +0.8, -0.8,     0.0, 0.8      },
            colores    [ num_verts*3 ] = {  1.0, 0.0, 0.0,   0.0, 1.0, 0.0,  0.0, 0.0, 1.0 } ;
        
        // crear y activar el VAO
        glGenVertexArrays( 1, &id_vao ); // crear VAO
        glBindVertexArray( id_vao );     // activa VAO

        // crear el VBO de coordenadas, y fijar puntero a la tabla de coordenadas
        GLenum  id_vbo_coordenadas ;
        glGenBuffers( 1, &id_vbo_coordenadas );               // crea VBO verts.
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_coordenadas );  // activa VBO verts.                            
        glBufferData( GL_ARRAY_BUFFER, 2*num_verts*sizeof(float), coordenadas, GL_STATIC_DRAW ); // copia
        glVertexPointer( 2, GL_FLOAT, 0, 0 );  // indica puntero a array de coordenadas
        glEnableClientState( GL_VERTEX_ARRAY ); // habilita uso de array de coordenadas

        // crear el VBO de colores, y fijar puntero a la tabla de colores
        GLenum id_vbo_colores  ;
        glGenBuffers( 1, &id_vbo_colores );  // crea VBO colores
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_colores );   // activa VBO colores                           
        glBufferData( GL_ARRAY_BUFFER, 3*num_verts*sizeof(float), colores, GL_STATIC_DRAW ); // copia
        glColorPointer( 3, GL_FLOAT, 0, 0 );    // indica puntero a array de colores
        glEnableClientState( GL_COLOR_ARRAY );  // habilita uso de array de colores
    }
    else
        glBindVertexArray( id_vao );

    // dibujar y desactivar el VAO
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glShadeModel( GL_SMOOTH );
    glDrawArrays( GL_POLYGON, 0, num_verts );
    glBindVertexArray( 0 );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo inmediato

void DibujarTrianguloMI( ) 
{
     assert( glGetError() == GL_NO_ERROR );

    // número de vértices que se van a dibujar
    constexpr unsigned
        num_verts = 3 ;

    const GLfloat 
        coordenadas[ num_verts*2 ] = {  -0.6, -0.6,      +0.6, -0.6,     0.0, 0.6      },
        colores    [ num_verts*3 ] = {  1.0, 1.0, 0.0,   1.0, 0.0, 1.0,  0.0, 1.0, 1.0 } ;

    // activar el VAO por defecto (en memoria de la app), deshabilitar punteros que no se usan (por si acaso)
    glBindVertexArray( 0 ) ;
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    // establecer y habilitar punteros
    glVertexPointer( 2, GL_FLOAT, 0, coordenadas );  // indica puntero a array de coordenadas
    glEnableClientState( GL_VERTEX_ARRAY );          // habilita uso del puntero al array de coordenadas
    glColorPointer( 3, GL_FLOAT, 0, colores );       // indica puntero a array de colores
    glEnableClientState( GL_COLOR_ARRAY );           // habilita uso del puntero al array de colores
    
    // configurar el modo y dibujar
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_POLYGON, 0, num_verts );

    // deshabilitar punteros (queda habilitado el VAO 0)
    glDisableClientState( GL_VERTEX_ARRAY );
    glDisableClientState( GL_COLOR_ARRAY );

    assert( glGetError() == GL_NO_ERROR );
}

// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar el contenido en la ventana

void VisualizarFrame( ) 
{ 
    using namespace std ;
    
    assert( glGetError() == GL_NO_ERROR );

    // establece la zona visible (toda la ventana)
    glViewport( 0, 0, ancho_actual, alto_actual ); 

    // establece la matriz de transformación de coordenadas, 
    // la hace igual a la matriz identidad
    glUniformMatrix4fv( loc_matriz_transf_coords, 1, GL_TRUE, mat_ident );
    
    // limpiar la ventana
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

    // Dibujar un triángulo en modo diferido 
    DibujarTrianguloMD();

    // Cambiar la matriz de transformación de coordenadas y volver a dibujar
    glUniformMatrix4fv( loc_matriz_transf_coords, 1, GL_TRUE, mat_despl );
    DibujarTrianguloMI();

    // esperar a que termine 'glDrawArrays' y entonces presentar el framebuffer actualizado
    glfwSwapBuffers( ventana_glfw );
    
}
// ---------------------------------------------------------------------------------------------
// función que se invoca cada vez que cambia el número de pixels del framebuffer 

void FGE_CambioTamano( GLFWwindow* ventana, int nuevo_ancho, int nuevo_alto )
{ 
    ancho_actual      = nuevo_ancho ;
    alto_actual       = nuevo_alto ;
    redibujar_ventana = true ; // fuerza a redibujar la ventana
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cada vez que se pulse o levante una tecla. 

void FGE_PulsarLevantarTecla( GLFWwindow* ventana, int key, int scancode, int action, int mods ) 
{ 
    // si se pulsa la tecla 'ESC', acabar el programa
    if ( key == GLFW_KEY_ESCAPE )
        terminar_programa = true ;
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cada vez que se pulse o levante un botón del ratón

void FGE_PulsarLevantarBotonRaton( GLFWwindow* ventana, int button, int action, int mods ) 
{ 
    // nada, por ahora 
    
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cada vez que cambie la posición del puntero

void FGE_MovimientoRaton( GLFWwindow* ventana, double xpos, double ypos )
{
    // nada, por ahora 
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cada vez que mueva la rueda del ratón.

void FGE_Scroll( GLFWwindow* ventana, double xoffset, double yoffset ) 
{
    // nada, por ahora 
}
// ---------------------------------------------------------------------------------------------
// función que se invocará cuando se produzca un error de GLFW 

void ErrorGLFW ( int error, const char * description )
{
    using namespace std ;
    cerr 
        << "Error en GLFW. Programa terminado" << endl 
        << "Código de error : " << error << endl 
        << "Descripción     : " << description << endl ;
    exit(1);
}
// ---------------------------------------------------------------------------------------------
// código de inicialización de GLFW 

void InicializaGLFW( int argc, char * argv[] ) 
{ 
    using namespace std ;

    // intentar inicializar, terminar si no se puede
    if ( ! glfwInit() )
    {   
        cout << "Imposible inicializar GLFW. Termino." << endl ;
        exit(1) ; 
    }
    
    // especificar que función se llamará ante un error de GLFW
    glfwSetErrorCallback( ErrorGLFW );
    
    // crear la ventana (var. global ventana_glfw), activar el rendering context 
    ventana_glfw = glfwCreateWindow( 512, 512, "IG ejemplo mínimo", nullptr, nullptr ); 
    glfwMakeContextCurrent( ventana_glfw ); // necesario para OpenGL

    // leer y guardar las dimensiones del framebuffer en pixels
    glfwGetFramebufferSize( ventana_glfw, &ancho_actual, &alto_actual );

    // definir cuales son las funciones gestoras de eventos (con nombres 'FGE_....')
    glfwSetFramebufferSizeCallback( ventana_glfw, FGE_CambioTamano );
    glfwSetKeyCallback            ( ventana_glfw, FGE_PulsarLevantarTecla );
    glfwSetMouseButtonCallback    ( ventana_glfw, FGE_PulsarLevantarBotonRaton);
    glfwSetCursorPosCallback      ( ventana_glfw, FGE_MovimientoRaton );
    glfwSetScrollCallback         ( ventana_glfw, FGE_Scroll );
}

// ---------------------------------------------------------------------------------------------
// función para inicializar GLEW (necesario para las funciones de OpenGL ver 2.0 y posteriores)
// en macOS no es necesario (está vacía)

void InicializaGLEW()
{
#ifndef MACOS
    using namespace std ;
    GLenum codigoError = glewInit();
    if ( codigoError != GLEW_OK ) // comprobar posibles errores
    {
        cout << "Imposible inicializar ’GLEW’, mensaje recibido: " << endl 
             << (const char *)glewGetErrorString( codigoError ) << endl ;
        exit(1);
    }
    else
        cout << "Librería GLEW inicializada correctamente." << endl << flush ;

#endif
}
// ---------------------------------------------------------------------------------------------
// función que compila el vertex y el fragment shader

void CompilarShaders( ) 
{ 
    assert( glGetError() == GL_NO_ERROR );
    using namespace std ;

    // buffer para guardar el informe de errores al compilar o enlazar
    constexpr GLsizei long_buffer = 1024*16 ;
    GLchar            buffer[ long_buffer ] ;
    GLsizei           long_informe ;  // longitud del informe de errores

    // longitudes en bytes (caracteres) de los fuentes
    const GLint longitud_vs = strlen( fuente_vs ),
                longitud_fs = strlen( fuente_fs );

    // crear y compilar el vertex shader
    const GLuint id_vs = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( id_vs, 1, &fuente_vs,  &longitud_vs ) ;
    glCompileShader( id_vs ) ;

    // ver errores al compilar el vertex shader, si hay alguno, si hay abortar
    GLint estado_vs ; 
    glGetShaderiv( id_vs, GL_COMPILE_STATUS, &estado_vs );
    if ( estado_vs != GL_TRUE )
    {   cout << "Error al compilar el vertex shader. Aborto." << endl ;
        glGetShaderInfoLog( id_vs, long_buffer, &long_informe, buffer );
        cout << buffer << endl ;
        exit(1);
    }

    // crear y compilar el fragment shader
    const GLuint id_fs = glCreateShader( GL_FRAGMENT_SHADER ) ;
    glShaderSource( id_fs, 1, &fuente_fs, &longitud_fs ) ;
    glCompileShader( id_fs ) ;

    // ver errores al compilar el fragment shader, si hay alguno, si hay abortar
    GLint estado_fs ; 
    glGetShaderiv( id_fs, GL_COMPILE_STATUS, &estado_fs );
    if ( estado_fs != GL_TRUE )
    {   cout << "Error al compilar el fragment shader. Aborto." << endl ;
        glGetShaderInfoLog( id_fs, long_buffer, &long_informe, buffer );
        cout << buffer << endl ;
        exit(1);
    }

    // crear programa, asociar shaders al programa, enlazar.
    const GLuint id_prog = glCreateProgram() ;
    glAttachShader( id_prog, id_vs );
    glAttachShader( id_prog, id_fs );
    glLinkProgram( id_prog ) ;

    // ver errores al enlazar el fragment shader
    GLint estado_prog ;
    glGetProgramiv( id_prog, GL_LINK_STATUS, &estado_prog );
    if ( estado_prog != GL_TRUE )
    {   cout << "Error al enlazar el programa. Aborto." << endl ;
        glGetProgramInfoLog( id_prog, long_buffer, &long_informe, buffer );
        cout << buffer << endl ;
        exit(1);
    }

    assert( glGetError() == GL_NO_ERROR );

     

    // activar el programa
    glUseProgram( id_prog );

    // leer el identificador ("location") del parámetro uniform "u_matriz_transf_coord"
    // poner esa matriz con un valor igual a la matriz identidad.
    loc_matriz_transf_coords = glGetUniformLocation( id_prog, "u_matriz_transf_coords" );
    if ( loc_matriz_transf_coords == -1 )
    {   cout << "Error: no encuentro variable uniform 'matriz_transf_coords'" << endl ;
        exit(1);
    }
    
    

    assert( glGetError() == GL_NO_ERROR );
    cout << "fragment y vertex shaders creados correctamente." << endl ;
   
}
// ---------------------------------------------------------------------------------------------

void InicializaOpenGL()
{
    using namespace std ;

    cout  << "Datos de versión e implementación de OpenGL" << endl
         << "    implementación de : " << glGetString(GL_VENDOR)  << endl
         << "    hardware          : " << glGetString(GL_RENDERER) << endl
         << "    version de OpenGL : " << glGetString(GL_VERSION) << endl
         << "    version de GLSL   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl ;

    InicializaGLEW(); // Fijar puntero a funciones de OpenGL versión 2.0 y posteriores
    CompilarShaders();

    glLineWidth( 8.3 );
    glClearColor( 1.0, 1.0, 1.0, 0.0 ); // color para 'glClear' (blanco, 100% opaco)
}
// ---------------------------------------------------------------------------------------------

void BucleEventosGLFW() 
{ 
    while ( ! terminar_programa )
    {   if ( redibujar_ventana ) 
        {   VisualizarFrame(); 
            redibujar_ventana = false; // (evita que se redibuje continuamente)
        }
        glfwWaitEvents(); // esperar evento y llamar FGE (si hay alguna) 
        terminar_programa = terminar_programa || glfwWindowShouldClose( ventana_glfw ); 
    } 
}
// ---------------------------------------------------------------------------------------------

int main( int argc, char *argv[] )
{
    using namespace std ;
    cout << "Programa mínimo de OpenGL 2.1" << endl ;

    InicializaGLFW( argc, argv ); // Crea una ventana, fija funciones gestoras de eventos
    InicializaOpenGL() ;          // Compila vertex y fragment shaders. Enlaza y activa programa. Inicializa GLEW.
    BucleEventosGLFW() ;          // Esperar eventos y procesarlos hasta que 'terminar_programa == true'
    glfwTerminate();              // Terminar GLFW (cierra la ventana)

    cout << "Programa terminado normalmente." << endl ;
}