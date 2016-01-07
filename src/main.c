/**
 * Einstieg in das gesamte Programm.
 * 
 * @author Maurice Tollmien. Github: MauriceGit
 */

/* ---- System Header einbinden ---- */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

//#include <GLFW/glfw3.h>
#include <GL/glut.h>

/* ---- Eigene Header einbinden ---- */
#include "io.h"
#include "scene.h" 
#include "types.h"

/**
 * Hauptprogramm.
 * Initialisiert Fenster, Anwendung und Callbacks, startet glutMainLoop.
 * @param argc Anzahl der Kommandozeilenparameter (In).
 * @param argv Kommandozeilenparameter (In).
 * @return Rueckgabewert im Fehlerfall ungleich Null.
 */
int
main (int argc, char **argv)
{
	srand (time (0));
	
	if (!initAndStartIO ("Much shader such wow ... ", 1920, 1080))
	{
		fprintf (stderr, "Initialisierung fehlgeschlagen!\n");
		return 1;
	} 
	
	return 0;
}
