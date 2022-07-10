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
#else 
#ifdef __APPLE__
//------------------------------------------------
// Includes y definiciones específicos en macOS
#include <OpenGL/gl3.h>
#include <GLFW/glfw3.h>  
#else 
// ------------------------------------------------
// Emitir error por sistema operativo no soportado
#error "No puedo determinar el sistema operativo, o no esta soportado"
#endif
#endif

constexpr int numErrors = 5 ;

const GLenum errCodes[numErrors] =
{
   GL_NO_ERROR ,
   GL_INVALID_ENUM ,
   GL_INVALID_VALUE ,
   GL_INVALID_OPERATION ,
   //GL_INVALID_FRAMEBUFFER_OPERATION ,  // REVISAR (pq no está definido con glfw?)
   GL_OUT_OF_MEMORY
} ;

const char * errDescr[numErrors] =
{
   "Error when trying to report an error: no error has been recorded",
   "An unacceptable value is specified for an enumerated argument",
   "A numeric argument is out of range",
   "The specified operation is not allowed in the current state",
   //"The command is trying to render to or read from the framebuffer while the currently bound framebuffer is not framebuffer complete (i.e. the return value from 'glCheckFramebufferStatus' is not GL_FRAMEBUFFER_COMPLETE)",
   "There is not enough memory left to execute the command"
} ;


const char * errCodeString[numErrors] =
{
   "GL_NO_ERROR",
   "GL_INVALID_ENUM",
   "GL_INVALID_VALUE",
   "GL_INVALID_OPERATION",
   //"GL_INVALID_FRAMEBUFFER_OPERATION",
   "GL_OUT_OF_MEMORY"
} ;

// ---------------------------------------------------------------------
// devuelve descripción de error dado el código de error opengl

std::string ErrorDescr( GLenum errorCode )
{
   int iErr = -1 ;
   for ( unsigned i = 0 ; i < numErrors ; i++ )
   {  if ( errCodes[i] == errorCode)
      {  iErr = i ;
         break ;
      }
   }
   if ( iErr == -1 )
      return std::string("Error when trying to report an error: error code is not a valid error code for 'glGetError'") ;
   return std::string( errDescr[iErr] ) ;
}

std::string ErrorCodeString( GLenum errorCode )
{
   int iErr = -1 ;
   for ( unsigned i = 0 ; i < numErrors ; i++ )
   {  if ( errCodes[i] == errorCode)
      {  iErr = i ;
         break ;
      }
   }
   if ( iErr == -1 )
      return std::string("** invalid error code **") ;
   return std::string( errCodeString[iErr] ) ;
}


// ---------------------------------------------------------------------------------------------
// Constantes y variables globales 

constexpr GLuint
    ind_atrib_posiciones = 0,      // índice del atributo de vértice con su posiciones (debe ser el índice 0, siempre)
    ind_atrib_colores    = 1 ;     // índice del atributo de vértice con su color RGB
bool 
    redibujar_ventana   = true ,   // puesto a true por los gestores de eventos cuando cambia el modelo y hay que regenerar la vista 
    terminar_programa   = false ;  // puesto a true en los gestores de eventos cuando hay que terminar el programa
GLFWwindow *                      
    ventana_glfw        = nullptr; // puntero a la ventana GLFW
int 
    ancho_actual        = 512 ,    // ancho actual del framebuffer, en pixels
    alto_actual         = 512 ;    // alto actual del framebuffer, en pixels
GLint 
    loc_mat_modelview,             // localizador o identificador (location) de la matriz 'u_modelview'  en los shaders
    loc_mat_proyeccion ;           // localizador o identificador (location) de la matriz 'u_proyeccion' en los shaders
GLenum 
    id_vao_ind = 0 ,                   // identificador de VAO (vertex array object)
    id_vao_no_ind = 0 ;


constexpr GLfloat mat_ident[] =    // matriz 4x4 identidad (para fijar valor inicial de las matrices)
    {   1.0, 0.0, 0.0, 0.0, 
        0.0, 1.0, 0.0, 0.0, 
        0.0, 0.0, 1.0, 0.0, 
        0.0, 0.0, 0.0, 1.0, 
    } ;

// ---------------------------------------------------------------------------------------------
// Fuentes para el vertex shader y el fragment shader 

// Cadena con el código fuente para el vertex shader sencillo, se invoca una vez por cada vértice. 
// su función es escribir en 'gl_Position' las posiciones del vértice 
// además, puede escribir otros atributos del vértice en variables 'out' (tambíen llamadas 'varying')
// (en este caso escribe el color en 'var_color')

const char * const fuente_vs = R"glsl(
    #version 330 core
    
    // Parámetros uniform (variables de entrada iguales para todos los vértices en cada primitiva)

    uniform mat4 u_mat_modelview;  // variable uniform: matriz de transformación de posiciones
    uniform mat4 u_mat_proyeccion; // variable uniform: matriz de proyección

    // Atributos de vértice (variables de entrada distintos para cada vértice)
    // (las posiciones de posición siempre deben estar en la 'location' 0)

    layout( location = 0 ) in vec3 atrib_posicion ; // atributo 0: posición del vértice
    layout( location = 1 ) in vec3 atrib_color ;    // atributo 1: color RGB del vértice
    
    // Variables 'varying' de salida (se calculan para cada vértice y se 
    // entregan interpoladas al fragment shader)
    
    out vec3  var_color ; // color RGB del vértice (el mismo que proporciona la aplic.)

    // función principal que se ejecuta una vez por vértice
    void main() 
    {
        // copiamos color recibido en el color de salida, tal cual
        var_color = atrib_color ;

        // calcular las posiciones del vértice en posiciones de mundo y escribimos 'gl_Position'
        // (se calcula multiplicando las cordenadas por la matrices 'modelview' y 'projection')
        gl_Position = u_mat_proyeccion * u_mat_modelview * vec4( atrib_posicion, 1);  
    }
)glsl";

// Cadena con el código fuente para el fragment shader sencillo: se invoca una vez por cada pixel.
// su función es escribir en 'gl_FragColor' el color del pixel 

const char * const fuente_fs = R"glsl(
    #version 330 core
    
    // datos de entrada ('in'), valores interpolados, a partir de las variables 
    // 'varying' calculadas como salida del vertex shader (se deben corresponder en nombre y tipo):

    in vec3 var_color ; // color interpolado en el pixel.

    // dato de salida (color del pixel)
    layout(location = 0) out vec4 out_color_fragmento ; // color que se calcula como resultado final de este shader en 'main'
    
    // función principal que se ejecuta una vez por cada pixel en el cual se 
    // proyecta una primitiva, calcula 'out_color_fragmento'

    void main() 
    {
        out_color_fragmento = vec4( var_color, 1 ) ; // el color del pixel es el color interpolado
    }
)glsl";


// ------------------------------------------------------------------------------------------------------
// activa una tabla de una atributo de vértice con flotantes 

void ActivarTablaFloats( GLuint ind_atrib, GLuint num_verts, GLuint num_vals_tupla, const GLvoid * datos )
{
    assert( 0 < ind_atrib && ind_atrib < 2);
    GLenum  id_vbo ;
    glGenBuffers( 1, &id_vbo );               // crea VBO verts.
    glBindBuffer( GL_ARRAY_BUFFER, id_vbo );  // activa VBO verts.                            
    glBufferData( GL_ARRAY_BUFFER, num_vals_tupla*num_verts*sizeof(float), datos, GL_STATIC_DRAW ); // copia
    glVertexAttribPointer( ind_atrib, num_vals_tupla, GL_FLOAT, GL_FALSE, 0, datos );  // indica puntero a array de posiciones
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glEnableVertexAttribArray( ind_atrib ); // habilita uso de array de posiciones
    assert( glGetError() == GL_NO_ERROR );
}
// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo diferido,
// no indexado, con 'glDrawArrays'

void DibujarTriangulo_MD_NoInd( ) 
{
     assert( glGetError() == GL_NO_ERROR );

    // número de vértices que se van a dibujar
    constexpr unsigned
        num_verts = 3 ;


     // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0) 
    const GLfloat 
        posiciones[ num_verts*2 ] = {  -0.8, -0.8,      +0.8, -0.8,     0.0, 0.8      },
        colores   [ num_verts*3 ] = {  1.0, 0.0, 0.0,   0.0, 1.0, 0.0,  0.0, 0.0, 1.0 } ;

    // la primera vez, crear e inicializar el VAO
    if ( id_vao_no_ind == 0 )
    {
        // crear y activar el VAO
        glGenVertexArrays( 1, &id_vao_no_ind ); // crear VAO
        glBindVertexArray( id_vao_no_ind );     // activa VAO
        assert( glGetError() == GL_NO_ERROR );

        // crear el VBO de posiciones,  fijar puntero y lo habilita 
        GLenum  id_vbo_posiciones ;
        glGenBuffers( 1, &id_vbo_posiciones );               // crea VBO verts.
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_posiciones );  // activa VBO verts.                            
        glBufferData( GL_ARRAY_BUFFER, 2*num_verts*sizeof(float), posiciones, GL_STATIC_DRAW ); // copia
        glVertexAttribPointer( ind_atrib_posiciones, 2, GL_FLOAT, GL_FALSE, 0, 0 );  // indica puntero a array de posiciones
        glEnableVertexAttribArray( ind_atrib_posiciones ); // habilita uso de array de posiciones
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
        assert( glGetError() == GL_NO_ERROR );

        // crear el VBO de colores, fijar puntero y habilitar
        GLenum id_vbo_colores  ;
        glGenBuffers( 1, &id_vbo_colores );  // crea VBO colores
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_colores );   // activa VBO colores                           
        glBufferData( GL_ARRAY_BUFFER, 3*num_verts*sizeof(float), colores, GL_STATIC_DRAW ); // copia
        glVertexAttribPointer(  ind_atrib_colores, 3, GL_FLOAT, GL_FALSE, 0, 0 );    // indica puntero a array de colores
        glEnableVertexAttribArray( ind_atrib_colores ) ;  // habilita uso de array de colores
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        assert( glGetError() == GL_NO_ERROR );
    }
    else
        glBindVertexArray( id_vao_no_ind );

    // dibujar y desactivar el VAO
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDrawArrays( GL_TRIANGLES, 0, num_verts );
    glBindVertexArray( 0 );

    assert( glGetError() == GL_NO_ERROR );
}


// ---------------------------------------------------------------------------------------------
// función que se encarga de visualizar un triángulo relleno en modo diferido,
// indexado, con 'glDrawElements'

void DibujarTriangulo_MD_Ind( ) 
{
     assert( glGetError() == GL_NO_ERROR );

    // número de vértices e índices que se van a dibujar
    constexpr unsigned
        num_verts = 3,
        num_inds  = 3 ;


     // tablas de posiciones y colores de vértices (posiciones en 2D, con Z=0) 
    const GLfloat 
        posiciones[ num_verts*2 ] = {  -0.8, -0.8,      +0.8, -0.8,     0.0, 0.8      },
        colores    [ num_verts*3 ] = {  1.0, 0.0, 0.0,   0.0, 1.0, 0.0,  0.0, 0.0, 1.0 } ;
    const GLuint 
        indices[ num_inds ] = { 0,1,2 };

    // la primera vez, crear e inicializar el VAO
    if ( id_vao_ind == 0 )
    {
        // crear y activar el VAO
        glGenVertexArrays( 1, &id_vao_ind ); // crear VAO
        glBindVertexArray( id_vao_ind );     // activa VAO
     
        // crear el VBO de posiciones,  fijar puntero y lo habilita 
        GLenum  id_vbo_posiciones ;
        glGenBuffers( 1, &id_vbo_posiciones );               // crea VBO verts.
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_posiciones );  // activa VBO verts.                            
        glBufferData( GL_ARRAY_BUFFER, 2*num_verts*sizeof(float), posiciones, GL_STATIC_DRAW ); // copia
        glVertexAttribPointer( ind_atrib_posiciones, 2, GL_FLOAT, GL_FALSE, 0, 0 );  // indica puntero a array de posiciones
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glEnableVertexAttribArray( ind_atrib_posiciones ); // habilita uso de array de posiciones
        assert( glGetError() == GL_NO_ERROR );

        // crear el VBO de colores, fijar puntero y habilitar
        GLenum id_vbo_colores  ;
        glGenBuffers( 1, &id_vbo_colores );  // crea VBO colores
        glBindBuffer( GL_ARRAY_BUFFER, id_vbo_colores );   // activa VBO colores                           
        glBufferData( GL_ARRAY_BUFFER, 3*num_verts*sizeof(float), colores, GL_STATIC_DRAW ); // copia
        glVertexAttribPointer(  ind_atrib_colores, 3, GL_FLOAT, GL_FALSE, 0, 0 );    // indica puntero a array de colores
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glEnableVertexAttribArray( ind_atrib_colores ) ;  // habilita uso de array de colores
        assert( glGetError() == GL_NO_ERROR );

        // crear el VBO de índices, popular VBO 
        // (queda activado en el 'target' GL_ELEMENT_ARRAY_BUFFER)
        GLenum id_vbo_indices ;
        glGenBuffers( 1, &id_vbo_indices ); // crea VBO indices 
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id_vbo_indices );// activa VBO de índices y lo deja activado en el target 'GL_ELEMENT_ARRAY_BUFFER', dentro del VBO
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, num_inds*sizeof(unsigned), indices, GL_STATIC_DRAW ); // copia indices desde  RAM hacia GPU        
    }
    else
        glBindVertexArray( id_vao_ind );

    assert( glGetError() == GL_NO_ERROR );

    // dibujar y desactivar el VAO
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    assert( glGetError() == GL_NO_ERROR );
    glDrawElements( GL_TRIANGLES, num_inds, GL_UNSIGNED_INT, 0 );
    assert( glGetError() == GL_NO_ERROR );
    glBindVertexArray( 0 );
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

    // fija la matriz de transformación de posiciones de los shaders ('u_modelview'), 
    // (la hace igual a la matriz identidad)
    glUniformMatrix4fv( loc_mat_modelview, 1, GL_TRUE, mat_ident );

    // fija la matriz de proyeccion 'modelview' de los shaders ('u_proyeccion')
    // (la hace igual a la matriz identidad)
    glUniformMatrix4fv( loc_mat_proyeccion, 1, GL_TRUE, mat_ident );
    
    // limpiar la ventana
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

    // habilitar EPO por Z-buffer (test de profundidad)
    glEnable( GL_DEPTH_TEST );

    // Dibujar un triángulo en modo diferido 
    DibujarTriangulo_MI_NoInd();

    // Cambiar la matriz de transformación de posiciones (matriz 'u_modelview')
    constexpr float incremento_z = -0.1 ;
    const GLfloat mat_despl[] =     // matriz 4x4 desplazamiento
    {   1.0, 0.0, 0.0, 0.2,         // (0,2 en X y en Y, -0.1 en Z (más cerca))
        0.0, 1.0, 0.0, 0.2, 
        0.0, 0.0, 1.0, incremento_z, 
        0.0, 0.0, 0.0, 1.0, 
    } ;
    glUniformMatrix4fv( loc_mat_modelview, 1, GL_TRUE, mat_despl );

    // dibujar triángulo (desplazado) en modo inmediato.
    DibujarTriangulo_MI_Ind();     // indexado 

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

    // especificar versión de OpenGL y parámetros de compatibilidad que se querrán
   // (pedimos opengl 330, tipo "core" (sin compatibilidad con versiones anteriores)
   glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
   glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
   glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
   glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    
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
// (deja activado el objeto programa)

void CompilarEnlazarShaders( ) 
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
    {   
        cout << "Error al compilar el vertex shader. Aborto." << endl ;
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
    {   
        cout << "Error al compilar el fragment shader. Aborto." << endl ;
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
    {   
        cout << "Error al enlazar el programa. Aborto." << endl ;
        glGetProgramInfoLog( id_prog, long_buffer, &long_informe, buffer );
        cout << buffer << endl ;
        exit(1);
    }

    assert( glGetError() == GL_NO_ERROR );

    // activar el programa
    glUseProgram( id_prog );

    // leer el identificador ("location") del parámetro uniform "u_modelview"
    // poner esa matriz con un valor igual a la matriz identidad.
    loc_mat_modelview = glGetUniformLocation( id_prog, "u_mat_modelview" );
    if ( loc_mat_modelview == -1 )
    {   cout << "Error: no encuentro variable uniform 'u_mat_modelview'" << endl ;
        exit(1);
    }

    // leer el identificador ("location") del parámetro uniform "u_proyeccion"
    // poner esa matriz con un valor igual a la matriz identidad.
    loc_mat_proyeccion = glGetUniformLocation( id_prog, "u_mat_proyeccion" );
    if ( loc_mat_modelview == -1 )
    {   cout << "Error: no encuentro variable uniform 'u_mat_proyeccion'" << endl ;
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
    CompilarEnlazarShaders();
    assert( glGetError() == GL_NO_ERROR );
    glLineWidth( 1.0 );
    assert( glGetError() == GL_NO_ERROR );
    glClearColor( 1.0, 1.0, 1.0, 0.0 ); // color para 'glClear' (blanco, 100% opaco)
    assert( glGetError() == GL_NO_ERROR );
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
    cout << "Programa mínimo de OpenGL 3.3 o superior" << endl ;

    InicializaGLFW( argc, argv ); // Crea una ventana, fija funciones gestoras de eventos
    InicializaOpenGL() ;          // Compila vertex y fragment shaders. Enlaza y activa programa. Inicializa GLEW.
    BucleEventosGLFW() ;          // Esperar eventos y procesarlos hasta que 'terminar_programa == true'
    glfwTerminate();              // Terminar GLFW (cierra la ventana)

    cout << "Programa terminado normalmente." << endl ;
}