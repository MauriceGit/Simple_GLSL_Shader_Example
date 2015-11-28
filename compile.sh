#!/bin/bash

programName="shaderExample"

echo "cleaning up . . ."
echo " --> rm -rf $(ls -C *.o 2> /dev/null)"
rm -rf *.o 2>/dev/null
rm "$programName" 2>/dev/null

# compile
echo "compiling sources . . ."
echo " --> gcc -I./ -c -o $(ls -C *.c 2>/dev/null)"
gcc -I./ -c -o vector.o vector.c
gcc -I./ -c -o stringOutput.o stringOutput.c
gcc -I./ -c -o imageLoader.o imageLoader.c
gcc -I./ -c -o main.o main.c
gcc -I./ -c -o scene.o scene.c
gcc -I./ -c -o io.o io.c
gcc -I./ -c -o logic.o logic.c


# link
echo "linking source object and image loader object files . . ."
#gcc vector.o stringOutput.o imageLoader.o main.o scene.o io.o logic.o -lz  -o "$programName" -lglut -lGLU -lGL
gcc  -L/lib -L/usr/X11R6/lib64 vector.o imageLoader.o main.o scene.o io.o logic.o stringOutput.o -lm -lglut -lGLU -lGL  -lz -o shaderDemo

rm -rf *.o 2>/dev/null

