/**
 * io.c is responsible for everything about screen-buffers, textures, framebuffers and buffers in general.
 * Because there is pretty much no logic left here (cause this is a shader-demo not a game)
 * there is not much (relevant) code anywhere else but here.
 * Also all buffer-initializations, texture loading and similar stuff happens here.
 *
 * @author Maurice Tollmien. Github: MauriceGit
 */

//#include <GLFW/glfw3.h>
#include <GL/glut.h>

/* ---- System Header einbinden ---- */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


/* ---- Eigene Header einbinden ---- */
#include "io.h"
#include "scene.h"
#include "logic.h"
#include "vector.h"
#include "imageLoader.h"
#include "stringOutput.h"

/* Shader-ID's */
GLuint G_ShaderColor, G_ShaderTexture, G_ShaderPosColor, G_ShaderDepthTexture;

/* Geometrie-Buffer */
GLuint G_ObjectsBuffer;

/* Textur */
GLuint G_TexImageRocks, G_TexCamera, G_TexCameraDepth;

/* Framebuffer-Objekte */
GLuint G_fboCam;

int G_ShowTexture = 0;

int G_Width = 1920;
int G_Height = 1080;
char* G_WindowTitle = "";
float G_Interval = 0;
float G_ThisCallTime = 0;
float G_NearPlane, G_FarPlane;

int G_FPS_Count = 0;
double G_FPS_All = 0;
int G_Help = 0;

GLfloat G_Objects[] = {
    -10.0, -10.0, -10.0,    0.1, 0.0,
     10.0, -10.0, -10.0,    1.0, 0.0,
     10.0,  10.0, -10.0,    1.0, 1.0,
    -10.0, -10.0, -10.0,    0.1, 0.0,
     10.0,  10.0, -10.0,    1.0, 1.0,
    -10.0,  10.0, -10.0,    0.1, 1.0
};

/**
 * Prints text on the screen.
 */
void printHelp (void)
{
    /* Textfarbe */
    GLfloat textColor[3] = { 0.0f, 0.0f, 0.0f };

    drawString (0.2f, 0.1f, textColor, HELP_OUTPUT_1);
    drawString (0.2f, 0.125f, textColor, HELP_OUTPUT_2);
    drawString (0.2f, 0.148f, textColor, HELP_OUTPUT_3);
    drawString (0.2f, 0.171f, textColor, HELP_OUTPUT_4);
    drawString (0.2f, 0.194f, textColor, HELP_OUTPUT_5);
    drawString (0.2f, 0.217f, textColor, HELP_OUTPUT_6);
    drawString (0.2f, 0.240f, textColor, HELP_OUTPUT_7);
    drawString (0.2f, 0.263f, textColor, HELP_OUTPUT_8);

}


void tellThemKidsWhatsGoingOn(void) {
    GLfloat textColor[3] = { 0.0f, 0.0f, 0.0f };
    switch (G_ShowTexture) {
        case 0:
            drawString (0.2f, 0.1f, textColor, "The area is plain-colored in the shader.");
            break;
        case 1:
            drawString (0.2f, 0.1f, textColor, "The 3d-positions will be mapped to a range of 0..1 and interpreted as RGB-color values in");
            drawString (0.2f, 0.12f, textColor, "the fragment shader. It basically shows coordinates as colores.");
            break;
        case 2:
            drawString (0.2f, 0.1f, textColor, "A texture is loaded from an image and mapped onto the area.");
            break;
        case 3:
            drawString (0.2f, 0.1f, textColor, "The scene with the image-texture is rendered into a new framebuffer-object.");
            drawString (0.2f, 0.12f, textColor, "This framebuffer-object is bound to a texture-object. So we basically render into a texture.");
            drawString (0.2f, 0.14f, textColor, "This texture is then rendered onto the actual area in the current scene.");
            break;
        case 4:
            drawString (0.2f, 0.1f, textColor, "The scene with the image-texture is rendered into a new framebuffer-object.");
            drawString (0.2f, 0.12f, textColor, "In this case not as a color-texture, but a depth-texture from the view of the camera.");
            drawString (0.2f, 0.14f, textColor, "The values of the depth-texture then get normalized (in the fragment-shader) and used");
            drawString (0.2f, 0.16f, textColor, "as color value. So we see now as grey-values how far we are away from objects in the scene.");
            break;
    }
}

/**
 * Timer-Callback
 * Initializes the timer and calls the redisplay of the scene.
 */
void cbTimer (int lastCallTime)
{
    /* Time since program start. */
    int thisCallTime = glutGet (GLUT_ELAPSED_TIME);

    /* Time since last call */
    double interval = (double) (thisCallTime - lastCallTime) / 1000.0f;

    calcTimeRelatedStuff(interval);

    /* Register itself as timer for the next time */
    glutTimerFunc (1000 / TIMER_CALLS_PS, cbTimer, thisCallTime);

    /* Re-display */
    glutPostRedisplay ();
}

/**
 * Sets the projection-matrix with the given aspect ratio for the window.
 */
static void
setProjection (GLdouble aspect)
{
  glMatrixMode (GL_PROJECTION);
  /* Load identity matrix and reset. */
  glLoadIdentity ();

  {
      /* perspective projection */
      gluPerspective (90.0,     /* opening angle */
                      aspect,   /* aspect ratio */
                      G_NearPlane,      /* near clipping plane */
                      G_FarPlane /* far clipping plane */ );
  }
}


static void drawColoredQuad(GLuint shader, double r, double g, double b) {
    glDisable(GL_CULL_FACE);

    /*
     * Tells the program if and which shader to use for the following drawing operations.
     * The intention is just for showing where the shader is going to be active.
     */
    glUseProgram(shader);

        /*
         * Read view and projection matrix for the vertex-shader
         */
        GLfloat mp[16], mv[16];
        glGetFloatv(GL_PROJECTION_MATRIX, mp);
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);

        /*
         * With the identifiers 'projMatrix' and 'viewMatrix' the matrices can now be accessed in the shader
         */
        glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"),  1, GL_FALSE, &mp[0]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"),  1, GL_FALSE, &mv[0]);

        /*
         * Color for the object (will be colored in the shader)
         */
        GLfloat color[] = {r, g, b};
        glUniform3fv(glGetUniformLocation(shader, "colorIn"), 1, color);

        /*
         * Vertex-buffer for rendering purposes of the objects in the buffer.
         */
        glBindBuffer (GL_ARRAY_BUFFER, G_ObjectsBuffer);
        /*
         * With the location (location = 0) the G_ObjectsBuffer can now be accessed in the shader!
         */
        int shaderPos = 0;
        glEnableVertexAttribArray(shaderPos);
        glVertexAttribPointer(shaderPos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);


        /*
         * Just normal drawing of the triangles (in this case). The only difference is, that now the shader is in
         * the pipeline and will work on the vertices/fragments.
         */
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(shaderPos);

        glBindBuffer (GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);
}

static void drawPosColoredQuad(GLuint shader) {
    glDisable(GL_CULL_FACE);

    /*
     * Tells the program if and which shader to use for the following drawing operations.
     * The intention is just for showing where the shader is going to be active.
     */
    glUseProgram(shader);

        /*
         * Read view and projection matrix for the vertex-shader
         */
        GLfloat mp[16], mv[16];
        glGetFloatv(GL_PROJECTION_MATRIX, mp);
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);

        /*
         * With the identifiers 'projMatrix' and 'viewMatrix' the matrices can now be accessed in the shader
         */
        glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"),  1, GL_FALSE, &mp[0]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"),  1, GL_FALSE, &mv[0]);

        /*
         * Vertex-buffer for rendering purposes of the objects in the buffer.
         */
        glBindBuffer (GL_ARRAY_BUFFER, G_ObjectsBuffer);
        /*
         * With the location (location = 0) the G_ObjectsBuffer can now be accessed in the shader!
         */
        int shaderPos = 0;
        glEnableVertexAttribArray(shaderPos);
        glVertexAttribPointer(shaderPos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);

        /*
         * Just normal drawing of the triangles (in this case). The only difference is, that now the shader is in
         * the pipeline and will work on the vertices/fragments.
         */
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(shaderPos);

        glBindBuffer (GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);
}

static void drawTexturedQuad(GLuint shader, GLuint texture) {
    glDisable(GL_CULL_FACE);

    /*
     * Tells the program if and which shader to use for the following drawing operations.
     * The intention is just for showing where the shader is going to be active.
     */
    glUseProgram(shader);

        /*
         * Read view and projection matrix for the vertex-shader
         */
        GLfloat mp[16], mv[16];
        glGetFloatv(GL_PROJECTION_MATRIX, mp);
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);

        /*
         * With the identifiers 'projMatrix' and 'viewMatrix' the matrices can now be accessed in the shader
         */
        glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"),  1, GL_FALSE, &mp[0]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"),  1, GL_FALSE, &mv[0]);

        GLfloat nearPlane[] = {G_NearPlane};
        glUniform1fv(glGetUniformLocation(shader, "nearPlane"), 1, nearPlane);
        GLfloat farPlane[] = {G_FarPlane};
        glUniform1fv(glGetUniformLocation(shader, "farPlane"), 1, farPlane);

        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shader, "texsampler"), 0);

        /*
         * Vertex-buffer for rendering purposes of the objects in the buffer.
         */
        glBindBuffer (GL_ARRAY_BUFFER, G_ObjectsBuffer);
        /*
         * With the location (location = 0) (or = 1 for tex-pos) the G_ObjectsBuffer can now be accessed in the shader!
         */
        int shaderPos = 0;
        glEnableVertexAttribArray(shaderPos);
        glVertexAttribPointer(shaderPos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);

        int shaderPosTex = 1;
        glEnableVertexAttribArray(shaderPosTex);
        glVertexAttribPointer(shaderPosTex, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (GLvoid*) (3*sizeof(GLfloat)));


        /*
         * Just normal drawing of the triangles (in this case). The only difference is, that now the shader is in
         * the pipeline and will work on the vertices/fragments.
         */
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisable(GL_TEXTURE_2D);

        glBindBuffer (GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);
}

void drawDemo(int renderToTexture) {
    if (!G_Help) {
        if (!renderToTexture) {
            switch(G_ShowTexture) {
                case 0:
                    drawColoredQuad(G_ShaderColor, 0, 0, 1);
                    break;
                case 1:
                    drawPosColoredQuad(G_ShaderPosColor);
                    break;
                case 2:
                    drawTexturedQuad(G_ShaderTexture, G_TexImageRocks);
                    break;
                case 3:
                    drawTexturedQuad(G_ShaderTexture, G_TexCamera);
                    break;
                case 4:
                    drawTexturedQuad(G_ShaderDepthTexture, G_TexCameraDepth);
                    break;
            }
        } else {
            drawTexturedQuad(G_ShaderTexture, G_TexImageRocks);
        }
        tellThemKidsWhatsGoingOn();
    } else {
        printHelp();
    }
}

/**
 * Renders a scene not to the screen but into a texture
 */
void drawSceneToSpecificFramebuffer(GLuint fbo, int renderToTexture) {
    /**
     * Drawing into the given framebuffer-object
     */
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (renderToTexture) {
        glClearColor(0.0, 0.8, 0.8, 0.0);
    } else {
        glClearColor(0.0, 1.0, 1.0, 0.0);
    }
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CULL_FACE);
    glViewport (0, 0, G_Width, G_Height);
    setProjection ((double)G_Width/G_Height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt (getCameraPosition(0), getCameraPosition(1), getCameraPosition(2),
         0.0, 0.0, 0.0,
         0.0, 1.0, 0.0);

    glDisable(GL_TEXTURE_2D);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf ("Framebuffer is not correct!\n");
    }

    drawDemo(renderToTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * Drawing-callback.
 * Deletes the buffer, calls the drawing routines of the scene and
 * switches front- and backbuffer (double-buffering).
 */
static void cbDisplay ()
{

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 1.0, 1.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /**
     * Drawing the scene into the given framebuffer-object and so basically into the texture.
     * (Color and depth. Both attached to the same framebuffer-object)
     */
    drawSceneToSpecificFramebuffer(G_fboCam, 1);
    /**
     * Drawing the scene onto the screen.
     */
    drawSceneToSpecificFramebuffer(0, 0);

    glutSwapBuffers ();
}



/**
 * Callback for keyboard.
 */
void cbKeyboard (unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'q':
        case 'Q':
        case ESC:
            exit(0);
            break;
        case 'h':
        case 'H':
            G_Help = !G_Help;
            break;
        case 's':
        case 'S':
            G_ShowTexture = (G_ShowTexture + 1) % 5;
            break;

    }

}

void cbSpecial (int key, int x, int y)
{
    switch (key)
    {
        case GLUT_KEY_F1:
            toggleWireframeMode();
            break;

    }
}

void
handleMouseEvent (int x, int y, CGMouseEventType eventType, int button, int buttonState)
{
    switch (eventType)
    {
        case mouseButton:
            switch (button)
            {
                case GLUT_LEFT_BUTTON:
                    setMouseEvent(MOVE,x,y);
                break;
                case GLUT_RIGHT_BUTTON:
                    setMouseEvent(ZOOM,x,y);
                break;
                default:
                  break;
            }
        break;
        default:
          break;
    }
    if (buttonState)
        setMouseEvent(NONE,x,y);
}

void cbMouseButton (int button, int state, int x, int y)
{
    handleMouseEvent (x, y, mouseButton, button, state);
}

void cbMouseMotion (int x, int y)
{
    if (getMouseEvent() == MOVE)
        setCameraMovement(x,y);

    if (getMouseEvent() == ZOOM)
        setCameraZoom(x,y);

    setMouseCoord(x,y);
}


/**
 * Callback for changes in the window-size.
 * Initiate the change of the projection-matrix to the windowsize.
 * @param w screen-width
 * @param h screen-height.
 */
void cbReshape (int w, int h)
{
  /* Whole screen is relevant. */
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);

  /* Adjust of the projection-matrix to the new aspect-ratio */
  setProjection ((GLdouble) w / (GLdouble) h);
}



/**
 * Register the GLUT-Callback-routines.
 */
void registerCallBacks (void)
{

    glutMotionFunc(cbMouseMotion);
    glutMouseFunc (cbMouseButton);

    /* Timer-Callback */
    glutTimerFunc (1000 / TIMER_CALLS_PS,
                 cbTimer,
                 glutGet (GLUT_ELAPSED_TIME));


    glutReshapeFunc (cbReshape);
    glutDisplayFunc (cbDisplay);

    glutKeyboardFunc (cbKeyboard);
    glutSpecialFunc (cbSpecial);

    glutIgnoreKeyRepeat (1);


}

/**
 * Very very very evil file-load-hacking (but working :P) ;)
 */
int readFile (char * name, GLchar ** buffer) {
    FILE *f = fopen(name, "rb");
    fseek(f, 0, SEEK_END);
    int pos = ftell(f);
    fseek(f, 0, SEEK_SET);

    (*buffer) = malloc(pos+1);
    fread(*buffer, pos-1, 1, f);
    (*buffer)[pos-1] = '\0';
    fclose(f);
}

GLuint loadShaders(char * vertexShader, char * fragmentShader){

    /* Create the shaders */
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;

    /* Compile Vertex Shader */
    printf("Compiling Vertex shader\n");
    char * VertexSourcePointer = NULL;
    readFile(vertexShader, &VertexSourcePointer);

    glShaderSource(VertexShaderID, 1, (const GLchar **)&VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    /* Check Vertex Shader */
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char * vertexShaderErrorMessage = calloc(InfoLogLength, sizeof(char));
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &(vertexShaderErrorMessage[0]));
    fprintf(stdout, "vertexShaderErrorMessage: %s\n", vertexShaderErrorMessage);

    /* Compile Fragment Shader */
    printf("Compiling Fragment shader\n");
    char * FragmentSourcePointer = NULL;
    readFile(fragmentShader, &FragmentSourcePointer);

    glShaderSource(FragmentShaderID, 1, (const GLchar **)&FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    /* Check Fragment Shader */
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char * fragmentShaderErrorMessage = calloc(InfoLogLength, sizeof(char));
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &(fragmentShaderErrorMessage[0]));
    fprintf(stdout, "fragmentShaderErrorMessage: %s\n", fragmentShaderErrorMessage);

    /*  Link the program */
    GLuint ProgramID = glCreateProgram();

    glAttachShader(ProgramID, VertexShaderID);

    glAttachShader(ProgramID, FragmentShaderID);

    glLinkProgram(ProgramID);

    /* Check the program */
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char * programErrorMessage = calloc(InfoLogLength, sizeof(char));
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &(programErrorMessage[0]));
    fprintf(stdout, "programErrorMessage: %s\n", programErrorMessage);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

int loadTextureImage(Image * image, char * name, GLuint * tex) {

    /**
     * Load image-file
     */
    if (!imageLoad(name, image))
    {
        printf("Error reading image file");
        exit(1);
    }

    /**
     * Create Texture-object with the given identifier
     */
    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /** Create texture with the data from the image */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->sizeX, image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
}

void allocateMemoryStuffDepth(GLuint * texID, GLuint * texID2, GLuint * fbo) {

    /**
     * Create texture
     */
    glGenTextures(1, texID);
    glBindTexture(GL_TEXTURE_2D, *texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    /** Observe the values: GL_RGBA8 and GL_BGRA for classification of the texture as a color-texture */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, G_Width, G_Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

    /**
     * Now the corresponding depth-texture
     */
    glGenTextures(1, texID2);
    glBindTexture(GL_TEXTURE_2D, *texID2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    /** Observe the values: GL_DEPTH_COMPONENT for classification as depth-texture */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, G_Width, G_Height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

    /**
     * Create framebuffer-object
     */
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    /**
     * OK, now we marry the texture and the framebuffer with each other.
     * See the corresponding variables: texID and texID2 and the type of attachment
     *
     * Observe the difference:
     * GL_COLOR_ATTACHMENT0 (normal color-texture)
     * GL_DEPTH_ATTACHMENT  (depth-texture)
     */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texID, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *texID2, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * Initializes the program (including io and OpenGL) and starts the
 * event handling.
 */
int initAndStartIO (char *title, int width, int height)
{
    int windowID = 0;

    G_Width = width;
    G_Height = height;
    G_WindowTitle = title;
    int argc = 1;
    char *argv = "cmd";

    G_NearPlane = 0.5;
    G_FarPlane  = 50.0;

    /* initialize GLUT */
    glutInit (&argc, &argv);

    /* initialize window */
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    /* window size */
    glutInitWindowSize (G_Width, G_Height);
    /* window position */
    glutInitWindowPosition (0, 0);

    windowID = glutCreateWindow (G_WindowTitle);

    if (windowID)
    {

        /* Background color and other stuff initialization */
        if (initScene ())
        {
            printf ("--> Load shaders...\n"); fflush(stdout);

            registerCallBacks ();

            /*
             * Load shaders from file
             */
            G_ShaderTexture = loadShaders("textureVertexShader.vert", "textureFragmentShader.frag");
            G_ShaderColor = loadShaders("colorVertexShader.vert", "colorFragmentShader.frag");
            G_ShaderPosColor = loadShaders("posColorVertexShader.vert", "colorFragmentShader.frag");
            G_ShaderDepthTexture = loadShaders("textureVertexShader.vert", "textureDepthFragmentShader.frag");

            printf ("--> Shaders are loaded.\n"); fflush(stdout);

            /*
             * Load texture from file
             */
            Image * imageRocks;
            imageRocks = malloc(sizeof(Image));
            loadTextureImage(imageRocks, "sunset-red.bmp", &G_TexImageRocks);

            /*
             * Create buffer for the object we want to draw.
             */
            glGenBuffers(1, &G_ObjectsBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, G_ObjectsBuffer);
            glBufferData(GL_ARRAY_BUFFER, sizeof(G_Objects)*sizeof(GLfloat), G_Objects, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            allocateMemoryStuffDepth(&G_TexCamera, &G_TexCameraDepth, &G_fboCam);

            printf ("--> Finished Initialization.\n"); fflush(stdout);

            /* Endless loop is started. */
            glutMainLoop ();
            windowID = 0;


        } else {
            glutDestroyWindow (windowID);
            return 0;
        }
    } else {
        return 0;
    }
    glutDestroyWindow (windowID);

    return 1;
}
