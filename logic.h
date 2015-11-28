#ifndef __LOGIC_H__
#define __LOGIC_H__
/**
 * @author Maurice Tollmien. Github: MauriceGit
 */

#include <GL/glu.h>
#include <stdio.h>
#include <stdarg.h>
#include "types.h"


/* ------- GETTER / SETTER ------- */
void setMouseState(MouseInterpretType state);
void setMouseEvent(MouseInterpretType state,int x, int y);
void setMouseCoord(int x, int y);
MouseInterpretType getMouseEvent(void);
double getCameraPosition (int axis);
void setCameraMovement(int x,int y);
void setCameraZoom(int x,int y);
void setKey (int key, int value);

/* ------- BERECHNUNGEN ------- */
void calcTimeRelatedStuff (double interval);

/* ------- INIT ------- */
void initGame ();

#endif
