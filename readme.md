# YGL: Yong's Graphics Library

This library is developed to use OpenGL conveniently.

> [!NOTE]
> glfw, glew libraries are needed inside the library folder.
>
> glfw, glew, glm, stb_image headers are needed inside the include folder.
>
> Please check what is being ignored in this repo.

## program.hpp

Create and manage shader program

## framebuffer.hpp

Create and manage framebuffer object 

## objreader.hpp

Read a wavefront .obj file format and generate vao, vbo, and etc.

You can render it by calling `render` method.

## camera.hpp

It contain some useful methods for VP matrices and callback methods which can be used in glfw callbacks. 
