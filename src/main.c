/**
 * Program-Entry point
 *
 * @author Maurice Tollmien. Github: MauriceGit
 */

/* ---- System headers ---- */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

//#include <GLFW/glfw3.h>
#include <GL/glut.h>

/* ---- Own headers ---- */
#include "io.h"
#include "scene.h"
#include "types.h"

/**
 * Main program and entry point.
 * Initializes the window, callback and much more.
 * All the magic is happening in io.c though :)
 */
int
main (int argc, char **argv)
{
    srand (time (0));

    if (!initAndStartIO ("Much shader such wow ... ", 1920, 1080))
    {
        fprintf (stderr, "Initialization went wrong\n");
        return 1;
    }

    return 0;
}
