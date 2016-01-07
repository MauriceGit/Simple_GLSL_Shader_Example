#ifndef __TYPES_H__
#define __TYPES_H__

/**
 * @author Maurice Tollmien. Github: MauriceGit
 */

#include <GL/gl.h>

/* ---- Eigene Konstanten */

/**
 * PHYSIK!
 */
#define PARTICLE_COUNT	6000000
#define RAND_COUNT		(PARTICLE_COUNT*100)
#define ATTRACTOR_COUNT	8



#define GROUND_HEIGHT	0.3

#define CAMERA_X        -10.1
#define CAMERA_Y        10.0
#define CAMERA_Z        15.0
#define CAMERA_SPEED    15.0

#define CAMERA_MOVEMENT_SPEED	4.0
#define CAMERA_ZOOM_SPEED 10.0

#define WORLD_SIZE 150

#define E			2.71828183
#define PI             3.14159265  
#define EPS		0.0001

/** Anzahl der Aufrufe der Timer-Funktion pro Sekunde */
#define TIMER_CALLS_PS      1920

#define RED                     0.7, 0.0, 0.0
#define BLUE                    0.0, 0.0, 0.7
#define GREEN                   0.0, 0.7, 0.0
#define BLACK                   0.0, 0.0, 0.0
#define WHITE                   1.0, 1.0, 1.0
#define GREY                    0.4, 0.4, 0.4 
#define YELLOW                  0.7, 0.7, 0.0

/* Tastatur */
/** Keyboardtaste ESC definieren */
#define ESC     27
/** Keyboardtaste SPACE definieren */
#define SPACE   32

/* Text */
/** Textausgabe, wenn das SPIEel zu ende ist, weil kein Stein mehr verfuegbar ist */
#define HELP_OUTPUT_1       "====== U'r blinded by pure AWESOMENESS!!!!! ======"
#define HELP_OUTPUT_2       "|________________________________________________|"
#define HELP_OUTPUT_3       "q/Q    Beendet die Demo."
#define HELP_OUTPUT_4       "s      Wechselt zwischen den verschiedenen Textur-Modi"
#define HELP_OUTPUT_5       "h/H    Oeffnet/schliesst den Hilfemodus."
#define HELP_OUTPUT_6       "f1     Wireframe an/aus."
#define HELP_OUTPUT_7       "Maus + rechte Maustaste    Abstand zum Mittelpunkt."
#define HELP_OUTPUT_8       "Maus + linke Maustaste     Bewegung im Raum."

/** Mausereignisse. */
enum e_MouseEventType
{ mouseButton, mouseMotion, mousePassiveMotion };
/** Datentyp fuer Mausereignisse. */
typedef enum e_MouseEventType CGMouseEventType;

/** Mausereignisse. */
enum e_MouseInterpretType
{ NONE, MOVE, ZOOM};
/** Datentyp fuer Mausereignisse. */
typedef enum e_MouseInterpretType MouseInterpretType;

/** Punkt im 3D-Raum (homogene Koordinaten) */
typedef GLfloat CGPoint4f[4];
typedef GLfloat CGPoint3f[3];
typedef CGPoint3f CGColor;

/** Datentyp fuer einen Vektor */
typedef double Vector4D[4];
typedef Vector4D Punkt4D;

/** Vektor im 3D-Raum */
typedef GLfloat CGVector3D[3];

typedef int Movement[3];

typedef struct {
	GLfloat x,y,z,w;
} Vec4;

typedef struct {
	GLfloat x,y,z;
} Vec3;

typedef Vec4 Attractor[ATTRACTOR_COUNT]; /* xyz = Position, w = Mass */

typedef float AttractorMass[ATTRACTOR_COUNT];

#endif
