// Ejemplo mínimo de código OpenGL, usa OpenGL 2.1 + GLSL 1.2
//
// Ver referencia de la API aquí:
// ver: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/
//

#include <cassert>  // para 'assert' (verificación de condiciones lógicas)
#include <cstring>  // para 'strlen' (al compilar shaders)
#include <iostream> 

#ifdef __linux__
// -----------------------------------------------
// Includes y definiciones específicos en Linux
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h> 
#else 
#ifdef __APPLE__
//------------------------------------------------
// Includes y definiciones específicos en macOS
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLFW/glfw3.h>  
#define glGenVertexArrays  glGenVertexArraysAPPLE
#define glBindVertexArray  glBindVertexArrayAPPLE
#else 
// ------------------------------------------------
// Emitir error por sistema operativo no soportado
#error "No puedo determinar el sistema operativo, o no esta soportado"
#endif
#endif


// ---------------------------------------------------------------------------------------------
// Constantes y variables globales 

bool 
    redibujar_ventana   = true ,   // puesto a true por los gestores de eventos cuando cambia el modelo y hay que regenerar la vista 
    terminar_programa   = false ;  // puesto a true en los gestores de eventos cuando hay que terminar el programa
GLFWwindow *                      
    ventana_glfw        = nullptr; // puntero a la ventana GLFW
int 
    ancho_actual        = 512 ,    // ancho actual del framebuffer, en pixels
    alto_actual         = 512 ;    // alto actual del framebuffer, en pixels

GLint 
    loc_modelview,                 // localizador o identificador (location) de la matriz 'u_modelview'  en los shaders
    loc_proyeccion ;               // localizador o identificador (location) de la matriz 'u_proyeccion' en los shaders

GLenum 
    id_vao_no_ind = 0 ,                   // identificador de VAO para MD, secuencia no indexada 
    id_vao_ind    = 0 ;                   // identificador de VAO para MD, secuencia indexada 


const GLfloat mat_ident[] =        // matriz 4x4 identidad (para fijar valor inicial de la matriz 'modelview')
    {   1.0, 0.0, 0.0, 0.0, 
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
    
    uniform mat4 u_modelview;  // variable uniform: matriz de transformación de coordenadas
    uniform mat4 u_proyeccion; // variable uniform: matriz de proyección
    varying vec4 v_color ;     // variable de salida: color del vértice

    void main() 
    {
        v_color     = gl_Color ;   // color vértice = color enviado por la aplic.
        gl_Position = u_proyeccion * u_modelview * gl_Vertex;  // transforma coords
    }
)glsl";

// fuente para el fragment shader sencillo: se invoca una vez por cada pixel.
// su función es escribir en 'gl_FragColor' el color del pixel 

const char * const fuente_fs = R"glsl(
    #version 120
    
    varying vec4 v_color ; // variable de entrada: color interpolado en el pixel.
    
    void main() 
    {
        gl_FragColor = v_color ; // el color del pixel es el color interpolado
    }
)glsl";

// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo diferido,
// no indexado, con 'glDrawArrays'

void DibujarTrianguloMD_NoInd( ) 
{
     assert( glGetError() == GL_NO_ERROR );

    // número de vértices que se van a dibujar
    constexpr unsigned
        num_verts = 3 ;


     // tablas de coordenadas y colores de vértices (coordenadas en 2D, con Z=0) 
    const GLfloat 
        coordenadas[ num_verts*2 ] = {  -0.8, -0.8,      +0.8, -0.8,     0.0, 0.8      },
        colores    [ num_verts*3 ] = {  1.0, 0.0, 0.0,   0.0, 1.0, 0.0,  0.0, 0.0, 1.0 } ;

    // la primera vez, crear e inicializar el VAO
    if ( id_vao_no_ind == 0 )
    {
        // crear y activar el VAO
        glGenVertexArrays( 1, &id_vao_no_ind ); // crear VAO
        glBindVertexArray( id_vao_no_ind );     // activa VAO
     
        // desabilitar atributos que no vamos a usar
        glDisableClientState( GL_NORMAL_ARRAY );   // no usaremos array de normales
        glDisableClientState( GL_TEXTURE_COORD_ARRAY ); // no usaremos array de coordenadas de textura

        // crear el VBO de coordenadas,  fijar puntero y lo habilita 
        GLenum  id_vbo_coordenadas ;
        glGenBuffers( 1, &id_vbo_coordenadas );               // crea VBO verts.
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_coordenadas );  // activa VBO verts.                            
        glBufferData( GL_ARRAY_BUFFER, 2*num_verts*sizeof(float), coordenadas, GL_STATIC_DRAW ); // copia
        glVertexPointer( 2, GL_FLOAT, 0, 0 );  // indica puntero a array de coordenadas
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glEnableClientState( GL_VERTEX_ARRAY ); // habilita uso de array de coordenadas

        // crear el VBO de colores, fijar puntero y habilitar
        GLenum id_vbo_colores  ;
        glGenBuffers( 1, &id_vbo_colores );  // crea VBO colores
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_colores );   // activa VBO colores                           
        glBufferData( GL_ARRAY_BUFFER, 3*num_verts*sizeof(float), colores, GL_STATIC_DRAW ); // copia
        glColorPointer( 3, GL_FLOAT, 0, 0 );    // indica puntero a array de colores
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glEnableClientState( GL_COLOR_ARRAY );  // habilita uso de array de colores

    }
    else
        glBindVertexArray( id_vao_no_ind );

    // dibujar y desactivar el VAO
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_POLYGON, 0, num_verts );
    glBindVertexArray( 0 );

    assert( glGetError() == GL_NO_ERROR );
}


// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo diferido,
// indexado, con 'glDrawElements'

void DibujarTrianguloMD_Ind( ) 
{
     assert( glGetError() == GL_NO_ERROR );

    // número de vértices e índices que se van a dibujar
    constexpr unsigned
        num_verts = 3,
        num_inds  = 3 ;


     // tablas de coordenadas y colores de vértices (coordenadas en 2D, con Z=0) 
    const GLfloat 
        coordenadas[ num_verts*2 ] = {  -0.8, -0.8,      +0.8, -0.8,     0.0, 0.8      },
        colores    [ num_verts*3 ] = {  1.0, 0.0, 0.0,   0.0, 1.0, 0.0,  0.0, 0.0, 1.0 } ;
    const GLuint 
        indices[ num_inds ] = { 0,1,2 };

    // la primera vez, crear e inicializar el VAO
    if ( id_vao_ind == 0 )
    {
        // crear y activar el VAO
        glGenVertexArrays( 1, &id_vao_ind ); // crear VAO
        glBindVertexArray( id_vao_ind );     // activa VAO
     
        // desabilitar atributos que no vamos a usar
        glDisableClientState( GL_NORMAL_ARRAY );   // no usaremos array de normales
        glDisableClientState( GL_TEXTURE_COORD_ARRAY ); // no usaremos array de coordenadas de textura

        
        // crear el VBO de coordenadas,  activar, fijar puntero y habilitar 
        GLenum  id_vbo_coordenadas ;
        glGenBuffers( 1, &id_vbo_coordenadas );               // crea VBO verts.
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_coordenadas );  // activa VBO verts.                            
        glBufferData( GL_ARRAY_BUFFER, 2*num_verts*sizeof(float), coordenadas, GL_STATIC_DRAW ); // copia
        glVertexPointer( 2, GL_FLOAT, 0, 0 );  // indica puntero a array de coordenadas
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glEnableClientState( GL_VERTEX_ARRAY ); // habilita uso de array del coordenadas designado con 'glVertexPointer'

        // crear el VBO de colores, activar, fijar puntero y habilitar
        GLenum id_vbo_colores  ;
        glGenBuffers( 1, &id_vbo_colores );  // crea VBO colores
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_colores );   // activa VBO colores                           
        glBufferData( GL_ARRAY_BUFFER, 3*num_verts*sizeof(float), colores, GL_STATIC_DRAW ); // copia
        glColorPointer( 3, GL_FLOAT, 0, 0 );    // indica puntero a array de colores
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glEnableClientState( GL_COLOR_ARRAY );  // habilita uso de array de colores designado con 'glColorPointer'

        // crear el VBO de índices, popular VBO (queda activado)
        GLenum id_vbo_indices ;
        glGenBuffers( 1, &id_vbo_indices ); // crea VBO indices 
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_vbo_indices );// activa VBO de índices y lo deja activado en el target 'GL_ELEMENT_ARRAY_BUFFER', dentro del VBO
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, num_inds*sizeof(unsigned), indices, GL_STATIC_DRAW ); // copia indices desde  RAM hacia GPU        
    }
    else
        glBindVertexArray( id_vao_ind );

    // dibujar y desactivar el VAO
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawElements( GL_POLYGON, num_inds, GL_UNSIGNED_INT, 0 );
    glBindVertexArray( 0 );

    assert( glGetError() == GL_NO_ERROR );
}
// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo inmediato,
// no indexado (con 'glDrawArrays')

void DibujarTrianguloMI_NoInd( ) 
{
     assert( glGetError() == GL_NO_ERROR );

    // número de vértices que se van a dibujar
    constexpr unsigned
        num_verts = 3 ;

    // coordenadas y colores de los vértices
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
// función que se encarga de visualizar un triángulo relleno en modo inmediato,
// indexado (con 'glDrawElements')

void DibujarTrianguloMI_Ind( ) 
{
     assert( glGetError() == GL_NO_ERROR );

    // número de vértices e índices que se van a dibujar
    constexpr unsigned
        num_verts = 3 ,
        num_inds  = 3 ;

    // coordenadas y colores de los vértices
    const GLfloat 
        coordenadas[ num_verts*2 ] = {  -0.6, -0.6,      +0.6, -0.6,     0.0, 0.6      },
        colores    [ num_verts*3 ] = {  1.0, 1.0, 0.0,   1.0, 0.0, 1.0,  0.0, 1.0, 1.0 } ;
    const GLuint 
        indices[ num_inds ] = { 0,1,2 };

    // activar el VAO por defecto (en memoria de la app), deshabilitar punteros que no se usan (por si acaso)
    glBindVertexArray( 0 ) ;
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState( GL_NORMAL_ARRAY );

    // establecer y habilitar punteros
    glVertexPointer( 2, GL_FLOAT, 0, coordenadas );  // indica puntero a array de coordenadas
    glEnableClientState( GL_VERTEX_ARRAY );          // habilita uso del puntero al array de coordenadas
    glColorPointer( 3, GL_FLOAT, 0, colores );       // indica puntero a array de colores
    glEnableClientState( GL_COLOR_ARRAY );           // habilita uso del puntero al array de colores
    
    // configurar el modo y dibujar, al dibujar especificar donde está el array de índices
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawElements( GL_POLYGON, num_inds, GL_UNSIGNED_INT, indices );

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
    
    // comprobar y limpiar variable interna de error
    assert( glGetError() == GL_NO_ERROR );

    // establece la zona visible (toda la ventana)
    glViewport( 0, 0, ancho_actual, alto_actual ); 

    // fija la matriz de transformación de coordenadas de los shaders ('u_modelview'), 
    // (la hace igual a la matriz identidad)
    glUniformMatrix4fv( loc_modelview, 1, GL_TRUE, mat_ident );

    // fija la matriz de proyeccion 'modelview' de los shaders ('u_proyeccion')
    // (la hace igual a la matriz identidad)
    glUniformMatrix4fv( loc_proyeccion, 1, GL_TRUE, mat_ident );
    
    // limpiar la ventana
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

    // habilitar EPO por Z-buffer (test de profundidad)
    glEnable( GL_DEPTH_TEST );

    // Dibujar un triángulo en modo diferido 
    DibujarTrianguloMD_NoInd();
    //DibujarTrianguloMD_Ind();

    // Cambiar la matriz de transformación de coordenadas (matriz 'u_modelview')
    constexpr float incremento_z = -0.1 ;
    const GLfloat mat_despl[] =     // matriz 4x4 desplazamiento
    {   1.0, 0.0, 0.0, 0.2,         // (0,2 en X y en Y, -0.1 en Z (más cerca))
        0.0, 1.0, 0.0, 0.2, 
        0.0, 0.0, 1.0, incremento_z, 
        0.0, 0.0, 0.0, 1.0, 
    } ;
    glUniformMatrix4fv( loc_modelview, 1, GL_TRUE, mat_despl );

    // dibujar triángulo (desplazado) en modo inmediato.
    //DibujarTrianguloMI_NoInd(); // no indexado 
    //DibujarTrianguloMI_Ind();     // indexado 
    DibujarTrianguloMD_Ind();     // indexado 

    // comprobar y limpiar variable interna de error
    assert( glGetError() == GL_NO_ERROR );

    // esperar a que termine 'glDrawArrays' y entonces presentar el framebuffer actualizado
    glfwSwapBuffers( ventana_glfw );
    
}
// ---------------------------------------------------------------------------------------------
// función que se invoca cada vez que cambia el número de pixels del framebuffer 
// (cada vez que se redimensiona la ventana)

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
#ifndef __APPLE__
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

    // leer el identificador ("location") del parámetro uniform "u_modelview"
    // poner esa matriz con un valor igual a la matriz identidad.
    loc_modelview = glGetUniformLocation( id_prog, "u_modelview" );
    if ( loc_modelview == -1 )
    {   cout << "Error: no encuentro variable uniform 'u_modelview'" << endl ;
        exit(1);
    }

    // leer el identificador ("location") del parámetro uniform "u_proyeccion"
    // poner esa matriz con un valor igual a la matriz identidad.
    loc_proyeccion = glGetUniformLocation( id_prog, "u_proyeccion" );
    if ( loc_modelview == -1 )
    {   cout << "Error: no encuentro variable uniform 'u_proyeccion'" << endl ;
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

    glLineWidth( 1.7 );
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