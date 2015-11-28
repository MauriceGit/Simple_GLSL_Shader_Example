/**
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
GLuint G_ShaderColor, G_ShaderTexture, G_ShaderPosColor;

/* Shader-Variablen */
GLuint G_Velocity_buffer_loc, G_Position_buffer_loc;
Attractor G_Attractor;
AttractorMass G_Attractor_Mass;
GLuint G_Position_buffer_tex, G_Velocity_buffer_tex;

/* Geometrie-Buffer */
GLuint G_ObjectsBuffer, G_Compute_Buffers, G_Position_buffer, G_Velocity_buffer, G_Attractor_buffer, G_AttractorMass_buffer, G_Life_buffer;

/* Textur */
GLuint G_TexImageRocks;

GLuint G_Dispatch_buffer;

int G_ShowTexture = 1;

int G_Width = 1920;
int G_Height = 1080;
int G_FullScreen = 1;
char* G_WindowTitle = "";
float G_Interval = 0;
float G_ThisCallTime = 0;

int G_FPS_Count = 0;
double G_FPS_All = 0;
int G_Help = 0;

GLfloat G_Objects[] = {
    -10.0, -10.0, -10.0, 	0.1, 0.0,
     10.0, -10.0, -10.0, 	1.0, 0.0,
     10.0,  10.0, -10.0,	1.0, 1.0,
    -10.0, -10.0, -10.0,	0.1, 0.0,
     10.0,  10.0, -10.0,	1.0, 1.0,
    -10.0,  10.0, -10.0,	0.1, 1.0
};

/**
 * Timer-Callback.
 * Initiiert Berechnung der aktuellen Position und Farben und anschliessendes
 * Neuzeichnen, setzt sich selbst erneut als Timer-Callback.
 * @param lastCallTime Zeitpunkt, zu dem die Funktion als Timer-Funktion
 *   registriert wurde (In).
 */
void cbTimer (int lastCallTime)
{
    
    /* Seit dem Programmstart vergangene Zeit in Millisekunden */
	int thisCallTime = glutGet (GLUT_ELAPSED_TIME);
    
	/* Seit dem letzten Funktionsaufruf vergangene Zeit in Sekunden */
	double interval = (double) (thisCallTime - lastCallTime) / 1000.0f;
		
	calcTimeRelatedStuff(interval);
			
	/* Wieder als Timer-Funktion registrieren */
	glutTimerFunc (1000 / TIMER_CALLS_PS, cbTimer, thisCallTime);

	/* Neuzeichnen anstossen */
	glutPostRedisplay ();
}

/**
 * Setzen der Projektionsmatrix.
 * Setzt die Projektionsmatrix unter Berücksichtigung des Seitenverhaeltnisses
 * des Anzeigefensters, sodass das Seitenverhaeltnisse der Szene unveraendert
 * bleibt und gleichzeitig entweder in x- oder y-Richtung der Bereich von -1
 * bis +1 zu sehen ist.
 * @param aspect Seitenverhaeltnis des Anzeigefensters (In).
 */
static void
setProjection (GLdouble aspect)
{
  /* Nachfolgende Operationen beeinflussen Projektionsmatrix */
  glMatrixMode (GL_PROJECTION);
  /* Matrix zuruecksetzen - Einheitsmatrix laden */
  glLoadIdentity ();
    
  {
      /* perspektivische Projektion */
      gluPerspective (90.0,     /* Oeffnungswinkel */
                      aspect,   /* Seitenverhaeltnis */
                      0.5,      /* nahe ClipPIEng-Ebene */
                      10000.0 /* ferne ClipPIEng-Ebene */ );
  }
}


static void drawColoredQuad(GLuint shader, double r, double g, double b) {
    glDisable(GL_CULL_FACE);
    
    /*
     * Hier dem Programm sagen, welchen (und ob) Shader es für die folgenden Zeichenoperationen nutzen soll.
     * Die Einrückung ist nur zur Verdeutlichung, in welchem Bereich der Shader angewendet wird.
     */
    glUseProgram(shader);
    
        /*
         * View- und Projektionsmatrix auslesen für den Vertex-Shader festlegen.
         */
        GLfloat mp[16], mv[16];
        glGetFloatv(GL_PROJECTION_MATRIX, mp);
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);
        
        /* 
         * Über die Identifier 'projMatrix' und 'viewMatrix' können die Matrizen im Shader zugegriffen werden! 
         */
        glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"),  1, GL_FALSE, &mp[0]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"),  1, GL_FALSE, &mv[0]);
			
        /*
         * Festlegen der Farbe, welche im Shader auf das zu zeichnende Objekt angewandt werden soll.
         */
        GLfloat color[] = {r, g, b};
        glUniform3fv(glGetUniformLocation(shader, "colorIn"), 1, color);
        
        /* 
         * Vertex-Buffer zum Rendern des im Buffer festgelegten Objektes! 
         */
        glBindBuffer (GL_ARRAY_BUFFER, G_ObjectsBuffer);
        /* 
         * Über die festgelegte Location (location = 0) kann der G_ObjectsBuffer im Shader zugegriffen werden 
         */
        int shaderPos = 0;
        glEnableVertexAttribArray(shaderPos);
        glVertexAttribPointer(shaderPos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
        
        
        /* 
         * Ganz normal zeichnen, wie vorher auch! Mit dem Unterschied, dass jetzt der Shader in der Pipeline 
         * hängt und auf die Vertices/Fragments angewandt wird. 
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
     * Hier dem Programm sagen, welchen (und ob) Shader es für die folgenden Zeichenoperationen nutzen soll.
     * Die Einrückung ist nur zur Verdeutlichung, in welchem Bereich der Shader angewendet wird.
     */
    glUseProgram(shader);
    
        /*
         * View- und Projektionsmatrix auslesen für den Vertex-Shader festlegen.
         */
        GLfloat mp[16], mv[16];
        glGetFloatv(GL_PROJECTION_MATRIX, mp);
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);
        
        /* 
         * Über die Identifier 'projMatrix' und 'viewMatrix' können die Matrizen im Shader zugegriffen werden! 
         */
        glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"),  1, GL_FALSE, &mp[0]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"),  1, GL_FALSE, &mv[0]);
			        
        /* 
         * Vertex-Buffer zum Rendern des im Buffer festgelegten Objektes! 
         */
        glBindBuffer (GL_ARRAY_BUFFER, G_ObjectsBuffer);
        /* 
         * Über die festgelegte Location (location = 0) kann der G_ObjectsBuffer im Shader zugegriffen werden 
         */
        int shaderPos = 0;
        glEnableVertexAttribArray(shaderPos);
        glVertexAttribPointer(shaderPos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
        
        /* 
         * Ganz normal zeichnen, wie vorher auch! Mit dem Unterschied, dass jetzt der Shader in der Pipeline 
         * hängt und auf die Vertices/Fragments angewandt wird. 
         */
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(shaderPos);
        
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);
}

static void drawTexturedQuad(GLuint shader) {
    glDisable(GL_CULL_FACE);
    
    /*
     * Hier dem Programm sagen, welchen (und ob) Shader es für die folgenden Zeichenoperationen nutzen soll.
     * Die Einrückung ist nur zur Verdeutlichung, in welchem Bereich der Shader angewendet wird.
     */
    glUseProgram(shader);
    
        /*
         * View- und Projektionsmatrix auslesen für den Vertex-Shader festlegen.
         */
        GLfloat mp[16], mv[16];
        glGetFloatv(GL_PROJECTION_MATRIX, mp);
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);
        
        /* 
         * Über die Identifier 'projMatrix' und 'viewMatrix' können die Matrizen im Shader zugegriffen werden! 
         */
        glUniformMatrix4fv(glGetUniformLocation(shader, "projMatrix"),  1, GL_FALSE, &mp[0]);
        glUniformMatrix4fv(glGetUniformLocation(shader, "viewMatrix"),  1, GL_FALSE, &mv[0]);
        
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, G_TexImageRocks);
		glUniform1i(glGetUniformLocation(shader, "texsampler"), 0);
        
        /* 
         * Vertex-Buffer zum Rendern des im Buffer festgelegten Objektes! 
         */
        glBindBuffer (GL_ARRAY_BUFFER, G_ObjectsBuffer);
        /* 
         * Über die festgelegte Location (location = 0) kann der G_ObjectsBuffer im Shader zugegriffen werden 
         */
        int shaderPos = 0;
        glEnableVertexAttribArray(shaderPos);
        glVertexAttribPointer(shaderPos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
        
        int shaderPosTex = 1;
        glEnableVertexAttribArray(shaderPosTex);
        glVertexAttribPointer(shaderPosTex, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (GLvoid*) (3*sizeof(GLfloat)));
        
        
        /* 
         * Ganz normal zeichnen, wie vorher auch! Mit dem Unterschied, dass jetzt der Shader in der Pipeline 
         * hängt und auf die Vertices/Fragments angewandt wird. 
         */
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        
        glBindBuffer (GL_ARRAY_BUFFER, 0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);
}

/**
 * Wird aufgerufen, wenn "h" gedrückt worden ist.
 * Gibt einen Text auf einem schwarzen Hintergrund auf dem Bildschirm aus
 */
void printHelp (void)
{
    /* Textfarbe */
    GLfloat textColor[3] = { 1.0f, 0.5f, 0.5f };

    drawString (0.2f, 0.1f, textColor, HELP_OUTPUT_1);
    drawString (0.2f, 0.125f, textColor, HELP_OUTPUT_2);
    drawString (0.2f, 0.148f, textColor, HELP_OUTPUT_3);
    drawString (0.2f, 0.171f, textColor, HELP_OUTPUT_4);
    drawString (0.2f, 0.194f, textColor, HELP_OUTPUT_5);
    drawString (0.2f, 0.217f, textColor, HELP_OUTPUT_6);
    drawString (0.2f, 0.240f, textColor, HELP_OUTPUT_7);
    drawString (0.2f, 0.263f, textColor, HELP_OUTPUT_8);

}

/**
 * Zeichen-Callback.
 * Loescht die Buffer, ruft das Zeichnen der Szene auf und tauscht den Front-
 * und Backbuffer.
 */
static void cbDisplay ()
{
    int i;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
        
    glDisable(GL_CULL_FACE);
    
    glViewport (0, 0, G_Width, G_Height);       
    setProjection ((double)G_Width/G_Height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt (getCameraPosition(0), getCameraPosition(1), getCameraPosition(2),
         0.0, 0.0, 0.0,
         0.0, 1.0, 0.0);
    
	if (!G_Help) {	
		switch(G_ShowTexture) {
			case 1:
				drawTexturedQuad(G_ShaderTexture);
				break;
			case 0:
				drawColoredQuad(G_ShaderColor, 0, 0, 1);
				break;
			case 2:
				drawPosColoredQuad(G_ShaderPosColor);
				break;
		}
	} else {
		printHelp();
	}
    
    glutSwapBuffers ();
    
}



/**
 * Callback fuer Tastendruck.
 * Ruft Ereignisbehandlung fuer Tastaturereignis auf.
 * @param key betroffene Taste (In).
 * @param x x-Position der Maus zur Zeit des Tastendrucks (In).
 * @param y y-Position der Maus zur Zeit des Tastendrucks (In).
 */
void cbKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
		case 'q':
		case 'Q':
		case ESC:
			exit(0);
			break;_H:
			break;
		case 'h':
		case 'H':
			G_Help = !G_Help;
			break;
		case 's':
		case 'S':
			G_ShowTexture = (G_ShowTexture + 1) % 3;
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
 * Callback fuer Aenderungen der Fenstergroesse.
 * Initiiert Anpassung der Projektionsmatrix an veraenderte Fenstergroesse.
 * @param w Fensterbreite (In).
 * @param h Fensterhoehe (In).
 */
void cbReshape (int w, int h)
{
  /* Das ganze Fenster ist GL-Anzeigebereich */
  glViewport (0, 0, (GLsizei) w, (GLsizei) h);

  /* Anpassen der Projektionsmatrix an das Seitenverhaeltnis des Fensters */
  setProjection ((GLdouble) w / (GLdouble) h);
}



/**
 * Registrierung der GLUT-Callback-Routinen.
 */
void registerCallBacks (void)
{
	
    glutMotionFunc(cbMouseMotion);
    glutMouseFunc (cbMouseButton);
    
    /* Timer-Callback - wird einmalig nach msescs Millisekunden ausgefuehrt */
	glutTimerFunc (1000 / TIMER_CALLS_PS, /* msecs - bis Aufruf von func */
                 cbTimer,       /* func  - wird aufgerufen    */
                 glutGet (GLUT_ELAPSED_TIME));  /* value - Parameter, mit dem
                                                   func aufgerufen wird */
    
    glutReshapeFunc (cbReshape);
    glutDisplayFunc (cbDisplay);
    
    glutKeyboardFunc (cbKeyboard);
    glutSpecialFunc (cbSpecial);
    
    glutIgnoreKeyRepeat (1);
    
    
}

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
	
	if (!imageLoad(name, image)) 
	{
		printf("Error reading image file");
		exit(1);
	}        
	
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->sizeX, image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);	
}

/**
 * Initialisiert das Programm (inkl. I/O und OpenGL) und startet die
 * Ereignisbehandlung.
 * @param title Beschriftung des Fensters
 * @param width Breite des Fensters
 * @param height Hoehe des Fensters
 * @return ID des erzeugten Fensters, 0 im Fehlerfall
 */
int initAndStartIO (char *title, int width, int height)
{
	int windowID = 0;
        
    G_Width = width;
    G_Height = height;
    G_WindowTitle = title;
    int argc = 1;
	char *argv = "cmd";
    
	/* Glut initialisieren */
	glutInit (&argc, &argv);
	
	/* FensterInitialisierung */
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	/* FensterGröße */
	glutInitWindowSize (G_Width, G_Height);
	/* FensterPosition */
	glutInitWindowPosition (0, 0);
	
	windowID = glutCreateWindow (G_WindowTitle);
	
    if (windowID)
    {   
        
        /* Hintergrund und so werden initialisiert (Farben) */
        if (initScene ())
        {
            int i;
            printf ("--> Shader laden...\n"); fflush(stdout);
            
            registerCallBacks ();
            
            /*
             * Shader aus Datei laden!
             */
            G_ShaderTexture = loadShaders("textureVertexShader.vert", "textureFragmentShader.frag");
            G_ShaderColor = loadShaders("colorVertexShader.vert", "colorFragmentShader.frag");
            G_ShaderPosColor = loadShaders("posColorVertexShader.vert", "colorFragmentShader.frag");
            
            printf ("--> Shader sind geladen.\n"); fflush(stdout);
            
            /*
             * Texture aus Datei laden!
             */
            Image * imageRocks;
            imageRocks = malloc(sizeof(Image));
            loadTextureImage(imageRocks, "sunset-red.bmp", &G_TexImageRocks);
            
            /*
             * Buffer für die zu zeichnenden Objekte erzeugen.
             */
            glGenBuffers(1, &G_ObjectsBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, G_ObjectsBuffer); 
            glBufferData(GL_ARRAY_BUFFER, sizeof(G_Objects)*sizeof(GLfloat), G_Objects, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
            printf ("--> Initialisierung angeschlossen.\n"); fflush(stdout);
            
            /* Die Endlosschleife wird angestoßen */
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
