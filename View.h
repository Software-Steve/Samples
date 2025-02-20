/*
 * The View class is our way of encapsulating all the data and OpenGL rendering required for our projects.
 * Thus, this class is the one that will change most often from one program to another
 *
*/

#ifndef VIEW_H
#define VIEW_H
//Remember that glew will not include all the opengl functions, so you must include both glew and gl
//IF YOU DO THIS, REMEMBER TO INCLUDE GLEW BEFORE GL, OTHERWISE YOU WILL GET COMPILING ERRORS!
#include <GL/glew.h>
#include <GL/gl.h>
#include <string>
#include <map>
using namespace std;

/*
 * we are using glm, a mathematics library for OpenGL
 * glm has two advantages:
 * 1. It is based on the GLSL specification. That is, all the data structures, functions and algorithms it offers
 *    follow the same convention as GLSL shaders. Therefore the transition to GLSL will be very smooth.
 * 2. glm is completely header-based. Thus there are no libraries or dlls to link to. This makes it very portable.
 **/

//the include below is for the basic glm stuff. You will find this header until C:\QTFiles\glm, so include that path in the project file
#include <glm/glm.hpp>

#include "Scenegraph.h"
#include "Plane.h"

class View
{
    //we need this #define for convenience in some OpenGL 4 functions. As you can see it is just a typecast
#define BUFFER_OFFSET(offset) ((void *)(offset))

    //a structure that encapsulates information about our GLSL shaders.
    typedef struct {
        GLenum       type; //is it a vertex shader, a fragment shader, a geometry shader, a tesselation shader or none of the above?
        string  filename; //the file that stores this shader
        GLuint       shader; //the ID for this shader after it has been compiled
    } ShaderInfo;



    typedef enum {OPENGL,RAYTRACER} RENDERMODE;

public:
    View();
    ~View();

    //the delegation functions called from the glut functions in main.cpp
    void resize(int w,int h);
    void initialize();
    void draw();
    void animate(float time);
    void openFile(string filename);

    //helper functions to probe the supported (latest) version of OpenGL and GLSL.
    void getOpenGLVersion(int *major,int *minor);
    void getGLSLVersion(int *major,int *minor);

    inline void initOpenGLMode()
    {
        mode = OPENGL;
    }

    inline void initRaytraceMode()
    {
        mode = RAYTRACER;
    }

protected:
    //helper function to compile and link our GLSL shaders
    GLuint createShaders(ShaderInfo *shaders);
    //helper function to print error messages for shader compiling
    void printShaderInfoLog(GLuint shader);
    



private:
    //the width and height of the window
    int WINDOW_WIDTH,WINDOW_HEIGHT;
    //IDs for locating various variables in our shaders
    GLint projectionLocation,modelviewLocation,objectColorLocation;
    GLint mat_ambientLocation,mat_diffuseLocation,mat_specularLocation,mat_shininessLocation;
    //LightLocation lightLocation[10];

    //the scene graph
    Scenegraph *graph;

    //the handle to the shader program
    GLuint shaderProgram;
    GLuint rtProgram;

    Plane *rtPlane; //textured quad for the raytracer output

    GLint rtProjLocation,rtModelViewLocation,rtTextureLocation;

    GLuint rtTextureID;

    RENDERMODE mode;


    //the actual projection and modelview matrices
    stack<glm::mat4> proj,modelview;

    glm::mat4 worldToView; //to make sure the same camera is used in both modes
};

#endif // VIEW_H
