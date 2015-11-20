/**
 * @author Maurice Tollmien. Github: MauriceGit
 */

/* ---- System Header einbinden ---- */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#ifdef MACOSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <GL/glu.h>

/* ---- Eigene Header einbinden ---- */
#include "scene.h"
#include "logic.h"
#include "types.h"
#include "vector.h"


/**
 * Initialisierung der Lichtquellen.
 * Setzt Eigenschaften der Lichtquellen (Farbe, Oeffnungswinkel, ...)
 */
static void initLight (void)
{

    /* Farbe der zweiten Lichtquelle */
	CGPoint4f lightColor1[3] =
	{ {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f,
														   1.0f}
	};
    
    /* Farbe der ersten Lichtquelle */
    CGPoint4f lightColor2[3] =
    { {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f,
                                                           1.0f}
    };
    
	/* Oeffnungswinkel der zweiten Lichtquelle */
	GLdouble lightCutoff1 = 90.0f;
	/* Lichtverteilung im Lichtkegel der zweiten Lichtquelle */
	GLdouble lightExponent1 = 20.0f;
    
    float globalAmbientLight[] = {0.3, 0.3, 0.3, 1.0};
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);
    
    /* Farbe der zweiten Lichtquelle setzen */
	glLightfv (GL_LIGHT1, GL_AMBIENT, lightColor1[0]);
	glLightfv (GL_LIGHT1, GL_DIFFUSE, lightColor1[1]);
	glLightfv (GL_LIGHT1, GL_SPECULAR, lightColor1[2]);
    
    /* Spotlight-Eigenschaften der zweiten Lichtquelle setzen */
	glLightf (GL_LIGHT1, GL_SPOT_CUTOFF, lightCutoff1);
	glLightf (GL_LIGHT1, GL_SPOT_EXPONENT, lightExponent1);
    
    /* Farbe der zweiten Lichtquelle setzen */
	glLightfv (GL_LIGHT2, GL_AMBIENT, lightColor2[0]);
	glLightfv (GL_LIGHT2, GL_DIFFUSE, lightColor2[1]);
	glLightfv (GL_LIGHT2, GL_SPECULAR, lightColor2[2]);
	
	/* Spotlight-Eigenschaften der zweiten Lichtquelle setzen */
	glLightf (GL_LIGHT2, GL_SPOT_CUTOFF, lightCutoff1);
	glLightf (GL_LIGHT2, GL_SPOT_EXPONENT, lightExponent1);
}

/**
 * Bei SPIEelbegin wird das SPIEelfeld komplett initialisiert
 * mit einem Hintergrund, einer Zeichenfarbe, Linienbreite.
 * Außerdem wird die Datenhaltung initialisiert (siehe initField (), initStones ()).
 * @return Ob ein Fehler aufgetreten ist.
 */
int initScene (void)
{
	glEnable (GL_DEPTH_TEST);
	glCullFace (GL_BACK);
	glEnable (GL_CULL_FACE);
	glEnable (GL_NORMALIZE);
	glEnable (GL_LIGHTING);
	initLight ();

	return 1;
}



/**
 * (De-)aktiviert den Wireframe-Modus.
 */
void
toggleWireframeMode (void)
{
    /* Flag: Wireframe: ja/nein */
    static GLboolean wireframe = GL_FALSE;

    /* Modus wechseln */
    wireframe = !wireframe;

    if (wireframe)
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
}

