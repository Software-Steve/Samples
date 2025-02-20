#include "View.h"
#include <GL/glew.h>
#include <cstdlib>
#include <fstream>
#include <vector>
using namespace std;

//glm headers to access various matrix producing functions, like ortho below in resize
#include <glm/gtc/matrix_transform.hpp>
//the glm header required to convert glm objects into normal float/int pointers expected by OpenGL
//see value_ptr function below for an example
#include <glm/gtc/type_ptr.hpp>

#include "Plane.h"
#include "Sphere.h"
#include "Cylinder.h"
//#include "Box.h"
#include "ObjectXMLReader.h"

View::View()
{
    graph = NULL;
    shaderProgram = 0;
    proj.push(glm::mat4(1.0));
    modelview.push(glm::mat4(1.0));
    mode = OPENGL;
    rtTextureID = -1;
}

View::~View()
{
    if (graph!=NULL)
        delete graph;

    if (rtPlane!=NULL)
        delete rtPlane;
}

void View::resize(int w, int h)
{
    //record the new dimensions
    WINDOW_WIDTH = w;
    WINDOW_HEIGHT = h;

    /*
     * This program uses orthographic projection. The corresponding matrix for this projection is provided by the glm function below.
     *The last two parameters are for the near and far planes.
     *
     *Very important: the last two parameters specify the position of the near and far planes with respect
     *to the eye, in the direction of gaze. Thus positive values are in front of the camera, and negative
     *values are in the back!
     **/
   // proj = glm::ortho(-WINDOW_WIDTH/2.0f,WINDOW_WIDTH/2.0f,-WINDOW_HEIGHT/2.0f,WINDOW_HEIGHT/2.0f,-0.1f,10000.0f);
    proj.top() = glm::perspective(60.0f,(float)WINDOW_WIDTH/WINDOW_HEIGHT,0.1f,10000.0f);

    rtPlane->setTransform(glm::rotate(glm::mat4(1.0),90.0f,glm::vec3(1.0,0.0,0.0)) * glm::scale(glm::mat4(1.0),glm::vec3(WINDOW_WIDTH,1,WINDOW_HEIGHT)));

}

void View::openFile(string filename)
{
    if (graph!=NULL)
    {
        delete graph;
        graph = NULL;
    }



    graph = ObjectXMLReader::readObjectXMLFile(filename);
    if (shaderProgram!=0)
    {
        graph->initShaderProgram(shaderProgram);
    }
}

void View::initialize()
{
    //populate our shader information. The two files below are present in this project.
    ShaderInfo shaders[] =
    {
        {GL_VERTEX_SHADER,"lighting-texturing.vert"},
        {GL_FRAGMENT_SHADER,"lighting-texturing.frag"},
        {GL_NONE,""} //used to detect the end of this array
    };

    //call helper function, get the program shader ID if everything went ok.
    shaderProgram = createShaders(shaders);

    shaders[0].filename = "bare.vert";
    shaders[1].filename = "bare.frag";

    rtProgram = createShaders(shaders);

    //assuming the program above is compiled and linked correctly, get IDs for all the input variables
    //that the shader expects our program to provide.
    //think of these IDs as references to those shader variables

    //the second parameter of glGetUniformLocation is a string that is the name of an actual variable in the shader program
    //this variable may exist in any of the shaders that are linked in this program.

    projectionLocation = glGetUniformLocation(shaderProgram,"projection");


    rtProjLocation = glGetUniformLocation(rtProgram,"projection");
    rtModelViewLocation = glGetUniformLocation(rtProgram,"modelview");
    rtTextureLocation = glGetUniformLocation(rtProgram,"image");


    rtPlane = new Plane(NULL);
    rtPlane->setTransform(glm::scale(glm::mat4(1.0),glm::vec3(WINDOW_WIDTH,1,WINDOW_HEIGHT)));




    worldToView = glm::lookAt(glm::vec3(200,100,200),glm::vec3(0,0,0),glm::vec3(0,1,0));
    //worldToView = glm::lookAt(glm::vec3(0,0,-110), glm::vec3(0,0,0),glm::vec3(0,1,0));

    //worldToView = glm::lookAt(glm::vec3(0,0,100),glm::vec3(0,0,0),glm::vec3(0,1,0));



}

void View::draw()
{
    //use the above program. After this statement, any rendering will use this above program
    //passing 0 to the function below disables using any shaders

    if (mode == OPENGL)
    {
        glUseProgram(shaderProgram);


        modelview.push(modelview.top());
        /*
         *The modelview matrix for the View class is going to store the world-to-view transformation
         *This effectively is the transformation that changes when the camera parameters chang
         *This matrix is provided by glm::lookAt
         */
        modelview.top() = modelview.top() * worldToView;
        // modelview.top() = modelview.top() * glm::lookAt(glm::vec3(000,200,200),glm::vec3(0,0,0),glm::vec3(0,1,0));

        glUniformMatrix4fv(projectionLocation,1,GL_FALSE,glm::value_ptr(proj.top()));

        /*
         *Instead of directly supplying the modelview matrix to the shader here, we pass it to the objects
         *This is because the object's transform will be multiplied to it before it is sent to the shader
         *for vertices of that object only.
         *
         *Since every object is in control of its own color, we also pass it the ID of the color
         *in the activated shader program.
         *
         *This is so that the objects can supply some of their attributes without having any direct control
         *of the shader itself.
         */

       // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        if (graph!=NULL)
        {
            graph->enableLights(modelview);
            graph->draw(modelview);
        }

        modelview.pop();

        glFinish();
    }
    else
    {
        //call raytracer, create texture of the result
        if (rtTextureID==-1)
        {
            float *buffer;

            modelview.push(modelview.top());
            /*
             *The modelview matrix for the View class is going to store the world-to-view transformation
             *This effectively is the transformation that changes when the camera parameters chang
             *This matrix is provided by glm::lookAt
             */

            //the eye position for the ray tracer
            modelview.top() = modelview.top() * worldToView;

            buffer = graph->raytrace(WINDOW_WIDTH,WINDOW_HEIGHT,modelview);


            modelview.pop();

            glGenTextures(1,&rtTextureID); //get a unique texture ID
            glBindTexture(GL_TEXTURE_2D,rtTextureID);// bind this texture as "current". All texture commands henceforth will apply to this ID

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_WIDTH,WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, buffer);

            delete []buffer;

            glBindTexture(GL_TEXTURE_2D,0); //unbind
        }
        glUseProgram(rtProgram);

        proj.push(proj.top());
        proj.top() = glm::ortho(-0.5f*WINDOW_WIDTH,0.5f*WINDOW_WIDTH,-0.5f*WINDOW_HEIGHT,0.5f*WINDOW_HEIGHT,0.1f,10000.0f);

        modelview.push(glm::lookAt(glm::vec3(0,0,10),glm::vec3(0,0,0),glm::vec3(0,1,0)));
        modelview.top() = modelview.top() * rtPlane->getTransform();
        /*
         *The modelview matrix for the View class is going to store the world-to-view transformation
         *This effectively is the transformation that changes when the camera parameters chang
         *This matrix is provided by glm::lookAt
         */
        // modelview.top() = modelview.top() * glm::lookAt(glm::vec3(000,200,200),glm::vec3(0,0,0),glm::vec3(0,1,0));

        glUniformMatrix4fv(rtProjLocation,1,GL_FALSE,glm::value_ptr(proj.top()));
        glUniformMatrix4fv(rtModelViewLocation,1,GL_FALSE,glm::value_ptr(modelview.top()));

        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,rtTextureID);
        glUniform1i(rtTextureLocation,0); //bind GL_TEXTURE0 to sampler2D (whatever is bound to GL_TEXTURE0)

        rtPlane->draw(modelview);

        modelview.pop();
        proj.pop();

        glFinish();
    }
    glUseProgram(0);
}

void View::animate(float time)
{
    if (graph!=NULL)
    {
        graph->animate(time);
    }
}




/*
 *This is a helper function that will take shaders info as a parameter, compiles them and links them
 *into a shader program.
 *
 *This function is standard and should not change from one program to the next.
 */

GLuint View::createShaders(ShaderInfo *shaders)
{
    ifstream file;
    GLuint shaderProgram;
    GLint linked;

    ShaderInfo *entries = shaders;

    shaderProgram = glCreateProgram();


    while (entries->type !=GL_NONE)
    {
        file.open(entries->filename.c_str());
        GLint compiled;


        if (!file.is_open())
            return false;

        string source,line;


        getline(file,line);
        while (!file.eof())
        {
            source = source + "\n" + line;
            getline(file,line);
        }
        file.close();


        const char *codev = source.c_str();


        entries->shader = glCreateShader(entries->type);
        glShaderSource(entries->shader,1,&codev,NULL);
        glCompileShader(entries->shader);
        glGetShaderiv(entries->shader,GL_COMPILE_STATUS,&compiled);

        if (!compiled)
        {
            printShaderInfoLog(entries->shader);
            for (ShaderInfo *processed = shaders;processed->type!=GL_NONE;processed++)
            {
                glDeleteShader(processed->shader);
                processed->shader = 0;
            }
            return 0;
        }
        glAttachShader( shaderProgram, entries->shader );
        entries++;
    }

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram,GL_LINK_STATUS,&linked);

    if (!linked)
    {
        printShaderInfoLog(entries->shader);
        for (ShaderInfo *processed = shaders;processed->type!=GL_NONE;processed++)
        {
            glDeleteShader(processed->shader);
            processed->shader = 0;
        }
        return 0;
    }

    return shaderProgram;
}

void View::printShaderInfoLog(GLuint shader)
{
    int infologLen = 0;
    int charsWritten = 0;
    GLubyte *infoLog;

    glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&infologLen);
    //	printOpenGLError();
    if (infologLen>0)
    {
        infoLog = (GLubyte *)malloc(infologLen);
        if (infoLog != NULL)
        {
            glGetShaderInfoLog(shader,infologLen,&charsWritten,(char *)infoLog);
            printf("InfoLog: %s\n\n",infoLog);
            free(infoLog);
        }

    }
//	printOpenGLError();
}

void View::getOpenGLVersion(int *major,int *minor)
{
    const char *verstr = (const char *)glGetString(GL_VERSION);
    if ((verstr == NULL) || (sscanf_s(verstr,"%d.%d",major,minor)!=2))
    {
        *major = *minor = 0;
    }
}

void View::getGLSLVersion(int *major,int *minor)
{
    int gl_major,gl_minor;

    getOpenGLVersion(&gl_major,&gl_minor);
    *major = *minor = 0;

    if (gl_major==1)
    {
        /* GL v1.x can only provide GLSL v1.00 as an extension */
        const char *extstr = (const char *)glGetString(GL_EXTENSIONS);
        if ((extstr!=NULL) && (strstr(extstr,"GL_ARB_shading_language_100")!=NULL))
        {
            *major = 1;
            *minor = 0;
        }
    }
    else if (gl_major>=2)
    {
        /* GL v2.0 and greater must parse the version string */
        const char *verstr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        if ((verstr==NULL) || (sscanf_s(verstr,"%d.%d",major,minor) !=2))
        {
            *major = 0;
            *minor = 0;
        }
    }
}
