// Ejemplo mínimo de código OpenGL, usa OpenGL 2.1, cauce fijo
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
    redibujar_ventana   = true ,  // puesto a true por los gestores de eventos cuando cambia el modelo y hay que regenerar la vista 
    terminar_programa   = false ;  // puesto a true en los gestores de eventos cuando hay que terminar el programa
GLFWwindow *                      
    ventana_glfw        = nullptr; // puntero a la ventana GLFW
int 
    ancho_actual        = 512 ,      // ancho actual del framebuffer, en pixels
    alto_actual         = 512 ;      // alto actual del framebuffer, en pixels
GLenum 
    id_vao_no_ind = 0 ,                   // identificador de VAO para MD, secuencia no indexada 
    id_vao_ind    = 0 ;                   // identificador de VAO para MD, secuencia indexada 

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
    
    assert( glGetError() == GL_NO_ERROR );

    // establece la zona visible (toda la ventana)
    glViewport( 0, 0, ancho_actual, alto_actual ); 

    // limpiar la ventana
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

    // fijar la matriz de proyección igual a la matriz identidad
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity() ;

    // fijar matriz de transformación de coordenadas 'modelview' igual a la matriz identidad
    // (dejar activo el modo )
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    
    // Dibujar un triángulo en modo diferido con la matriz 'modelview' actual.
    DibujarTrianguloMD_NoInd();
    //DibujarTrianguloMD_Ind();

    // Cambiar la matriz de transformación de coordenadas (matriz 'u_modelview')
    constexpr float incremento_z = -0.1 ;
    const GLfloat mat_despl[] =     // matriz 4x4 desplazamiento: 0.2 en X y en Y, -0.1 en Z
    {   1.0, 0.0, 0.0, 0.2, 
        0.0, 1.0, 0.0, 0.2, 
        0.0, 0.0, 1.0, incremento_z, 
        0.0, 0.0, 0.0, 1.0, 
    } ;
    glMatrixMode( GL_MODELVIEW );
    glMultTransposeMatrixf( mat_despl );

    // Dibujar triángulo (desplazado), en modo inmediato.
    //DibujarTrianguloMI_NoInd();
    DibujarTrianguloMD_Ind();

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

void InicializaOpenGL()
{
    using namespace std ;

    cout  << "Datos de versión e implementación de OpenGL" << endl
         << "    implementación de : " << glGetString(GL_VENDOR)  << endl
         << "    hardware          : " << glGetString(GL_RENDERER) << endl
         << "    version de OpenGL : " << glGetString(GL_VERSION) << endl
         << "    version de GLSL   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl ;

    // Fijar puntero a funciones de OpenGL versión 2.0 y posteriores
    InicializaGLEW();

    // habilitar comprobación de profundidad
    glEnable( GL_DEPTH_TEST );

    // fijar ancho de líneas
    glLineWidth( 1.7 );

    // color para 'glClear' (blanco, 100% opaco)
    glClearColor( 1.0, 1.0, 1.0, 0.0 );
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