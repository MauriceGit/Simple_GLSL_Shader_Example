# OpenGL Shader-Tutorial Project

What you see here is a kind of pragmatic tutorial. There is no list of steps, what you have to do to accomplish some results.

But what you get is a very simple and straight forward project with lots (LOTS) of comments.

So if you think about diving into shaders in OpenGL and you have some awesome ideas like Pathtracing or Screen-Space-Ambient-Occlusion
of anything else --> here you get all the source-code and basics you need to do just that.

With this kind of example-project I would have saved myself LOTS of hours some years ago.
I created this as a tutorial for university. But I think one or another of you might appreciate it.

## Knowledge Requirement

If you are new to OpenGL or new to programming this is not the tutorial for you!
What you need is a basic understanding of OpenGL and the rendering-pipeline.
Also you know how to initialize buffers, allocate memory and generally handle and write C-code.

So if you already build some smaller stuff with OpenGL and want to step your game up with some
awesome shaders - this is exactly for you!

## This is what you get

You can toggle between five different modes. Each mode is very shortly explained on the screen itself,
so you know what you are looking at.

- **Simple coloring of areas**. The fragment shader is used to color your object in just one color. This is
    the simplest shader you can get.

    ![One color](https://github.com/MauriceGit/Simple_GLSL_Shader_Example/blob/master/Screenshots/simple_color.png "Shader with one color")

- **Take the coordinates as color**. The fragment shader interprets the interpolated coordinates as RGB-color and colors your object
    with that color.

    ![Coord color](https://github.com/MauriceGit/Simple_GLSL_Shader_Example/blob/master/Screenshots/coord_color.png "Shader with coordinate-color")

- **Texture**. A texture is loaded and mapped onto the object.

    ![Texture](https://github.com/MauriceGit/Simple_GLSL_Shader_Example/blob/master/Screenshots/texture.png "Shader with texture")

- **Framebuffer-Texture**. The scene is rendered into a framebuffer-object which is bound to a texture-object. The texture is then
    mapped onto the object.

    ![fbo-texture](https://github.com/MauriceGit/Simple_GLSL_Shader_Example/blob/master/Screenshots/fbo_texture.png "Shader with framebuffer-object")

- **Framebuffer-Depth-Texture**. The scene is rendered into a framebuffer-object which is bound to a depth-buffer-texture. The texture-values
    are then linearized and mapped onto the object. Showing grey values corresponding to the depth of the scene.

    ![fbo-depth-texture](https://github.com/MauriceGit/Simple_GLSL_Shader_Example/blob/master/Screenshots/depthbuffer_texture.png "Shader with framebuffer-object (depth)")

## **Install && Run**

I only tested an ran this simulation on a debian-based unix OS (Ubuntu, Mint, ...). It should run on any other machine as well but is not
tested.

### **Requirements**

The following system-attributes are required for running this simulation:

- A graphics card supporting OpenGL version 3.3 (For the shaders).

- Unix-Libraries: xorg-dev, freeglut3-dev and mesa-common-dev

### **Running**

Compiling and running is pretty straight forward.

- ./compile.sh

- ./shaderDemo

While the simulation runs, you can move around (always looking to the center!) with your mouse (left-klick and move).

To toggle the different modes, press 's'.

Pressing 'h' at any point will give you help and all possible mouse/key assignments.


Good luck and have fun!














