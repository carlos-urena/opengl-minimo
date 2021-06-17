// Ejemplo mínimo de código OpenGL, usa OpenGL 2.1 + GLSL 1.2
//
// Ver referencia de la API aquí:
// ver: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/
//
// Para ver las variables predefinidas en GLSL, ver aquí:
// https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)

#include <cstring>  // para 'strlen' (al compilar shaders)
#include <iostream> 

#ifdef MACOS
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLFW/glfw3.h>  
#else 
#pragma error "no está definido MACOS ...."
#endif



// Variables globales 

bool 
    redibujar_ventana   = true ,  // puesto a true por los gestores de eventos cuando cambia el modelo y hay que regenerar la vista 
    terminar_programa   = false ;  // puesto a true en los gestores de eventos cuando hay que terminar el programa
GLFWwindow *                      
    ventana_glfw        = nullptr; // puntero a la ventana GLFW
int 
    ancho_actual        = 512 ,      // ancho actual de la ventana
    alto_actual         = 512 ;      // alto actual de la ventana

GLint 
    loc_matriz_transf_coords  ;

// fuente para el vertex shader sencillo
const char * const fuente_vs = 
    R"glsl(
        #version 120
        uniform mat4 matriz_transf_coords; 
        void main() 
        {
            gl_FrontColor = gl_Color ; 
            gl_Position   = matriz_transf_coords *  gl_Vertex;     
        }
    )glsl";

// fuente para el fragment shader sencillo
const char * const fuente_fs = 
    R"glsl(
        #version 120
        void main() 
        {
            gl_FragColor = gl_Color ; // color interpolado
        }
    )glsl";


// Funciones 

void VisualizarFrame( ) 
{ 
    static int cuenta = 0 ;
    using namespace std ;
    cout << "VisualizarFrame, cuenta == " << ++cuenta << endl ;

    assert( glGetError() == GL_NO_ERROR );

    glViewport( 0, 0, ancho_actual, alto_actual ); // establecer zona visible
    glClearColor( 1.0, 1.0, 1.0, 0.0 ); // color para 'glClear' (blanco, 100% opaco)
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // limpiar la ventana

    assert( glGetError() == GL_NO_ERROR );

    
    glBegin( GL_LINES );
        glColor4f( 1.0, 0.1, 0.1, 1.0 ); glVertex3f( 0.0, 0.0, 0.0 );
        glColor4f( 0.0, 1.0, 0.1, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 );
    glEnd() ; 

    assert( glGetError() == GL_NO_ERROR );

    glfwSwapBuffers( ventana_glfw );
    
}
// ---------------------------------------------------------------------------------------------

void FGE_CambioTamano( GLFWwindow* ventana, int nuevo_ancho, int nuevo_alto )
{ 
    ancho_actual      = nuevo_ancho ;
    alto_actual       = nuevo_alto ;
    redibujar_ventana = true ; // fuerza a redibujar la ventana

    using namespace std ;
    cout << "Nuevas dimensiones del viewport == " << nuevo_ancho << " x " << nuevo_alto << endl ;
}
// ---------------------------------------------------------------------------------------------

void FGE_PulsarLevantarTecla( GLFWwindow* ventana, int key, int scancode, int action, int mods ) 
{ 
    // si se pulsa la tecla 'ESC', acabar el programa
    if ( key == GLFW_KEY_ESCAPE )
        terminar_programa = true ;
}
// ---------------------------------------------------------------------------------------------

void FGE_PulsarLevantarBotonRaton( GLFWwindow* ventana, int button, int action, int mods ) 
{ 
    // nada, por ahora 
    
}
void FGE_MovimientoRaton( GLFWwindow* ventana, double xpos, double ypos )
{
    // nada, por ahora 
}
// ---------------------------------------------------------------------------------------------

void FGE_Scroll( GLFWwindow* ventana, double xoffset, double yoffset ) 
{
    // nada, por ahora 
}
// ---------------------------------------------------------------------------------------------

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

void Inicializa_GLFW( int argc, char * argv[] ) 
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
    ventana_glfw = glfwCreateWindow( ancho_actual, alto_actual, "IG ejemplo mínimo", nullptr, nullptr ); 
    glfwMakeContextCurrent( ventana_glfw ); // necesario para OpenGL

    // definir cuales son las funciones gestoras de eventos (con nombres 'FGE_....')
    glfwSetWindowSizeCallback ( ventana_glfw, FGE_CambioTamano );
    glfwSetKeyCallback        ( ventana_glfw, FGE_PulsarLevantarTecla );
    glfwSetMouseButtonCallback( ventana_glfw, FGE_PulsarLevantarBotonRaton);
    glfwSetCursorPosCallback  ( ventana_glfw, FGE_MovimientoRaton );
    glfwSetScrollCallback     ( ventana_glfw, FGE_Scroll );
}
// ---------------------------------------------------------------------------------------------

void Compilar_Shaders( ) 
{ 
    using namespace std ;

    assert( glGetError() == GL_NO_ERROR );

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

    // leer el identificador ("location") del parémtro uniform "matriz_transf_coord"
    loc_matriz_transf_coords = glGetUniformLocation( id_prog, "matriz_transf_coords" );
    if ( loc_matriz_transf_coords == -1 )
    {
        cout << "Error: no encuentro variable uniform 'matriz_transf_coords'" << endl ;
        exit(1);
    }
    const GLfloat ident[] = 
        {   1.0, 0.0, 0.0, 0.0, 
            0.0, 1.0, 0.0, 0.0, 
            0.0, 0.0, 1.0, 0.0, 
            0.0, 0.0, 0.0, 1.0, 
        } ;
    glUniformMatrix4fv( loc_matriz_transf_coords, 1, GL_FALSE, ident );

    assert( glGetError() == GL_NO_ERROR );
    cout << "fragment/vertex shader en uso, ok" << endl ;

    //glUseProgram( 0 );
}
// ---------------------------------------------------------------------------------------------

void Inicializa_OpenGL()
{
    Compilar_Shaders();

    glLineWidth( 8.3 );
}
// ---------------------------------------------------------------------------------------------

void BucleEventos_GLFW() 
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
    cout << "Programa mínimo de OpenGL" << endl ;

    Inicializa_GLFW( argc, argv ) ; // Crea una ventana
    Inicializa_OpenGL() ;           // Compila vertex y fragment shaders. Enlaza y activa programa.
    BucleEventos_GLFW() ;           // Esperar eventos y procesarlos
    glfwTerminate();                // Terminar GLFW (cierra la ventana)

    cout << "Programa terminado normalmente." << endl ;
}