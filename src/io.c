/**
 * io.c ist zuständig für alles, was mit Screenbuffern, Texturen und Framebuffern
 * zu tun hat.
 * Da wir fast alles darauf reduziert haben, ist dieses Modul für das gesamte
 * Zeichnen der Szene zuständig.
 * Weiterhin für Bufferinitialisierungen, Texturladen und als Objekt verfügbar machen etc.
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
    -10.0, -10.0, -10.0, 	0.1, 0.0,
     10.0, -10.0, -10.0, 	1.0, 0.0,
     10.0,  10.0, -10.0,	1.0, 1.0,
    -10.0, -10.0, -10.0,	0.1, 0.0,
     10.0,  10.0, -10.0,	1.0, 1.0,
    -10.0,  10.0, -10.0,	0.1, 1.0
};

/**
 * Wird aufgerufen, wenn "h" gedrückt worden ist.
 * Gibt einen Text auf einem schwarzen Hintergrund auf dem Bildschirm aus
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
			drawString (0.2f, 0.1f, textColor, "Die Flaeche wird im Shader einfarbig eingefaerbt.");
			break;
		case 1:
			drawString (0.2f, 0.1f, textColor,  "Die Position eines Vertices im Raum wird jeweils auf den Bereich 0..1 gemappt und ");
			drawString (0.2f, 0.12f, textColor, "im Fragmentshader als Farb-Komponente verwendet. Quasi eine Visualisierung von Koordinaten.");
			break;
		case 2:
			drawString (0.2f, 0.1f, textColor, "Eine Textur wird aus einer Bild-Datei geladen und im Shader auf die Flaeche gemappt.");
			break;
		case 3:
			drawString (0.2f, 0.1f, textColor, "Die Szene mit der Bild-Textur wird in ein neues Framebufferobjekt gerendert, welche");
			drawString (0.2f, 0.12f, textColor, "an eine Textur gebunden wird. Also wird quasi in eine Textur gerendert statt auf den ");
			drawString (0.2f, 0.14f, textColor, "Bildschirm. Diese Textur aus der aktuellen Szene wird nun auf die Flaeche gemappt.");
			break;
		case 4:
			drawString (0.2f, 0.1f, textColor, "Die Szene mit der Bild-Textur wird in ein neues Framebufferobjekt gerendert, in dem");
			drawString (0.2f, 0.12f, textColor, "Fall aber nicht als normale Textur sondern als Tiefenmap der Szene aus Sicht der");
			drawString (0.2f, 0.14f, textColor, "Kamera. Diese Tiefenmap wird im Fragmentshader normalisiert und als Grauwert betrachtet");
			drawString (0.2f, 0.16f, textColor, "genutzt, um die jeweiligen Fragments entsprechend ihrer Tiefe in der Szene einzufaerben.");
			break;
	}
}

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
                      G_NearPlane,      /* nahe ClipPIEng-Ebene */
                      G_FarPlane /* ferne ClipPIEng-Ebene */ );
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

static void drawTexturedQuad(GLuint shader, GLuint texture) {
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
        
        GLfloat nearPlane[] = {G_NearPlane};
		glUniform1fv(glGetUniformLocation(shader, "nearPlane"), 1, nearPlane);
		GLfloat farPlane[] = {G_FarPlane};
		glUniform1fv(glGetUniformLocation(shader, "farPlane"), 1, farPlane);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
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
 * Rendert eine Szene nicht auf den Bildschirm sondern in eine Textur.
 */
void drawSceneToSpecificFramebuffer(GLuint fbo, int renderToTexture) {
	/**
	 * Zeichnen in das übergebene Framebufferobjekt.
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
		printf ("Framebuffer ist nicht korrekt! 4\n");
	}

	drawDemo(renderToTexture);
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}	

/**
 * Zeichen-Callback.
 * Loescht die Buffer, ruft das Zeichnen der Szene auf und tauscht den Front-
 * und Backbuffer.
 */
static void cbDisplay ()
{
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0, 1.0, 1.0, 0.0);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	/**
	 * Zeichnen der Szene in das übergebene Frambuffer-Objekt und damit in die 
	 * entsprechenden Texturen (Farbe + Tiefe)
	 */
	drawSceneToSpecificFramebuffer(G_fboCam, 1);
	/**
	 * Zeichnen der Szene in das Framebufferobjekt 0, als den Bildschirm.
	 */
	drawSceneToSpecificFramebuffer(0, 0);
    
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

/**
 * Böses böses gehacktes (aber gut funktionierendes) Laden von Daten aus einer Datei :)
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
	 * Bilddaten laden.
	 */
	if (!imageLoad(name, image)) 
	{
		printf("Error reading image file");
		exit(1);
	}        
	
	/**
	 * Erstellung eines Texturobjektes für den übergebenen Identifier!
	 */
	glGenTextures(1, tex);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	/** Erstellen der Textur mit den Daten aus dem Bild! */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->sizeX, image->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);	
}

void allocateMemoryStuffDepth(GLuint * texID, GLuint * texID2, GLuint * fbo) {
	
	/**
	 * Eine Textur erzeugen.
	 */
	glGenTextures(1, texID);
	glBindTexture(GL_TEXTURE_2D, *texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	/** Beachtet den Identifier: GL_RGBA8 und GL_BGRA zur Klassifizierung der Textur als normale Textur mit Farbwerten */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, G_Width, G_Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	
	/**
	 * Die entsprechende Tiefentextur zur normalen.
	 */
	glGenTextures(1, texID2);
	glBindTexture(GL_TEXTURE_2D, *texID2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
	/** Beachtet den Identifier: GL_DEPTH_COMPONENT zur Klassifizierung der Textur als Tiefentextur! */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, G_Width, G_Height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	
	/**
	 * Framebufferobjekt erzeugen (Bildschirm ist auch quasi eins!)
	 */
	glGenFramebuffers(1, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);	
	
	/**
	 * Die alle miteinander verheiraten. Also Textur und Framebuffer.
	 * Die  Variable texID und texID2 entsprechen den Variablen, für die wir eben
	 * die entsprechenden Texturen erzeugt haben.
	 * 
	 * Beachtet die jeweilige Unterscheidung bezüglich: 
	 * GL_COLOR_ATTACHMENT0 (Normale Textur mit Farbwerten)
	 * GL_DEPTH_ATTACHMENT  (Tiefentextur!)
	 */
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *texID2, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	
	G_NearPlane = 0.5;
    G_FarPlane  = 50.0;
    
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
            printf ("--> Shader laden...\n"); fflush(stdout);
            
            registerCallBacks ();
            
            /*
             * Shader aus Datei laden!
             */
            G_ShaderTexture = loadShaders("textureVertexShader.vert", "textureFragmentShader.frag");
            G_ShaderColor = loadShaders("colorVertexShader.vert", "colorFragmentShader.frag");
            G_ShaderPosColor = loadShaders("posColorVertexShader.vert", "colorFragmentShader.frag");
            G_ShaderDepthTexture = loadShaders("textureVertexShader.vert", "textureDepthFragmentShader.frag");
            
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
            
            allocateMemoryStuffDepth(&G_TexCamera, &G_TexCameraDepth, &G_fboCam);
            
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
