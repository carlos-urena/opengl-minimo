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
    redibujar_ventana   = false ,  // puesto a true por los gestores de eventos cuando cambia el modelo y hay que regenerar la vista 
    terminar_programa   = false ;  // puesto a true en los gestores de eventos cuando hay que terminar el programa
GLFWwindow *                      
    ventana_glfw        = nullptr; // puntero a la ventana GLFW
int 
    ancho_actual        = 0 ,      // ancho actual de la ventana
    alto_actual         = 0 ;      // alto actual de la ventana


// Funciones 

void VisualizarFrame( ) 
{ 
    glViewport( 0, 0, ancho_actual, alto_actual ); 
    
}

void FGE_CambioTamano( GLFWwindow* ventana, int nuevo_ancho, int nuevo_alto )
{ 
    ancho_actual = nuevo_ancho ;
    alto_actual  = nuevo_alto ;
}

void FGE_PulsarLevantarTecla( GLFWwindow* ventana, int key, int scancode, int action, int mods ) 
{ 
    // si se pulsa la tecla 'ESC', acabar el programa
    if ( key == GLFW_KEY_ESCAPE )
        terminar_programa = true ;
}

void FGE_PulsarLevantarBotonRaton( GLFWwindow* ventana, int button, int action, int mods ) 
{ 
    // nada, por ahora 
    
}
void FGE_MovimientoRaton( GLFWwindow* ventana, double xpos, double ypos )
{
    // nada, por ahora 
}

void FGE_Scroll( GLFWwindow* ventana, double xoffset, double yoffset ) 
{
    // nada, por ahora 
}

void ErrorGLFW ( int error, const char * description )
{
    using namespace std ;
    cerr 
        << "Error en GLFW. Programa terminado" << endl 
        << "Código de error : " << error << endl 
        << "Descripción     : " << description << endl ;
    exit(1);
}

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
    ventana_glfw = glfwCreateWindow( 512, 512, "IG ejemplo mínimo", nullptr, nullptr ); 
    glfwMakeContextCurrent( ventana_glfw ); // necesario para OpenGL

    // definir cuales son las funciones gestoras de eventos (con nombres 'FGE_....')
    glfwSetWindowSizeCallback ( ventana_glfw, FGE_CambioTamano );
    glfwSetKeyCallback        ( ventana_glfw, FGE_PulsarLevantarTecla );
    glfwSetMouseButtonCallback( ventana_glfw, FGE_PulsarLevantarBotonRaton);
    glfwSetCursorPosCallback  ( ventana_glfw, FGE_MovimientoRaton );
    glfwSetScrollCallback     ( ventana_glfw, FGE_Scroll );
}

void Inicializa_OpenGL( ) 
{ 
    const char * vertex_src = "" ;
    const char * shader_src = "" ;
}

void BucleEventos_GLFW() 
{ 
    redibujar_ventana = true ; // dibujar la ventana la primera vez 
    terminar_programa = false ; // activar para terminar (p.ej. con tecla ESC) 

    while ( ! terminar_programa )
    {
        if ( redibujar_ventana ) // si ha cambiado algo y es necesario redibujar 
        { 
            VisualizarFrame(); // dibujar la escena
            redibujar_ventana = false; // evitar que se redibuje continuamente 
        }
        glfwWaitEvents(); // esperar evento y llamar FGE (si hay alguna) 
        terminar_programa = terminar_programa || glfwWindowShouldClose( ventana_glfw ) ; // poner a 'true' la variable 'terminar_programa', si procede 
    } 
}

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