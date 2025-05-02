//
//  framebuffer.hpp
//  ObjectiveOpenGL
//
//  Created by 이용규 on 3/17/25.
//

#ifndef framebuffer_hpp
#define framebuffer_hpp

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <stdio.h>

#include <iostream>
using std::cerr;
using std::endl;

struct TextureFormat {
    /** Specifies the number of color components in the texture.
     */
    GLint internalFormat;
    /** Specifies the format of the pixel data
     
     */
    GLenum format;
    /** Specifies the data type of the pixel data.
     
     */
    GLenum type;
    
    /** Generate texture format data by internalFormat
    
     - Parameters:
        - parameter internalFormat: Split this into format and type value.
     */
    void generate(const GLint &internalFormat) {
        this->internalFormat = internalFormat;
        if (internalFormat == GL_RGBA32F) {
            this->format = GL_RGBA;
            this->type = GL_FLOAT;
        }
    }
};

/** Framebuffer managing object.
 */
struct Framebuffer {
    /** Framebuffer object's ID(handle)*/
    GLuint id = 0;
    int width = 0, height = 0;
    
    GLuint renderbuffer = 0;
    GLuint *textureIDs = nullptr;
    
    /** Generate a new framebuffer object.
     
     Generate a new framebuffer object with given width and height.
     
     - Parameters:
         - parameter w: **Width** of the framebuffer
         - parameter h: **Height** of the framebuffer
     */
    void init(GLFWwindow *window) {
        this->cleanup();
        
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        this->width = w;
        this->height = h;
        
        glGenFramebuffers(1, &(this->id));
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    }

    /** Set a default framebuffer object.
     
     Set a default framebuffer object with given width and height.
     
     - Parameters:
         - parameter w: **Width** of the default framebuffer
         - parameter h: **Height** of the default framebuffer
     */
    void initDefault(GLFWwindow *window) {
        this->cleanup();
        
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        this->width = w;
        this->height = h;
        
        this->id = 0;
    }
    
    
    /** Generate new 2D textures and attach to this framebuffer.
     
     Generated textures are canvas of this framebuffer's draw call.
     
     Used functions:
     
     - [glGenTextures](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGenTextures.xhtml)
     
     - [glTexImage2D](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml)
     
     - Parameters:
     
        - parameter nTexture: Number of the textures will be attached.
     
        - parameter format: Texture's format used when creating texture.
     */
    void attachTexture2D(const int &nTexture, const TextureFormat &format) {
        this->bind();
        
        this->textureIDs = (GLuint *) malloc(nTexture * sizeof(GLuint));
        
        glGenTextures(nTexture, this->textureIDs);
        for (int i = 0; i < nTexture; i++) {
            glBindTexture(GL_TEXTURE_2D, this->textureIDs[i]);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         format.internalFormat,
                         this->width,
                         this->height,
                         0,
                         format.format,
                         format.type,
                         0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0 + i,
                                   GL_TEXTURE_2D,
                                   this->textureIDs[i],
                                   0);
            GLint err = glGetError();
            if (err != GL_NO_ERROR) {
               printf("%08X ", err);
            }

            glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
        }
        GLint err = glGetError();
        if (err != GL_NO_ERROR) {
           printf("%08X ", err);
        }
        
        // unbind all
    //    glBindTexture(GL_TEXTURE_2D, 0);
        this->unbind();
    }

    
    /** Generate new 2D textures and attach to this framebuffer.
     
     Generated textures are canvas of this framebuffer's draw call.
     
     Used functions:
     
     - [glGenTextures](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGenTextures.xhtml)
     
     - [glTexImage2D](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml)
     
     - Parameters:
     
        - parameter nTexture: Number of the textures will be attached.
     
        - parameter internalFormat: Texture's internalFormat used when create TextureFormat object and call again.
     */
    void attachTexture2D(const int nTexture, const GLint internalFormat) {
    //    this->bind();
        
        TextureFormat tf;
        tf.generate(internalFormat);
        
        this->attachTexture2D(nTexture, tf);
        GLint err = glGetError();
        if (err != GL_NO_ERROR) {
           printf("%08X ", err);
        }
        
        this->unbind();
    }

    void attachRenderBuffer(const GLenum internalFormat) {
        this->bind();
        
        glGenRenderbuffers(1, &(this->renderbuffer));
        glBindRenderbuffer(GL_RENDERBUFFER, this->renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER,
                              internalFormat,
                              this->width,
                              this->height);
        
        GLenum attachment;
        if(internalFormat == GL_DEPTH24_STENCIL8)
            attachment = GL_DEPTH_STENCIL_ATTACHMENT;
        else
            return;
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  attachment,
                                  GL_RENDERBUFFER,
                                  this->renderbuffer);
        

        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        this->unbind();
    }

    void render(GLFWwindow* window, const GLuint vao) {
        this->bind();
    //    std::cout << "render id : " << this->id << std::endl;
        
        glViewport(0, 0, this->width, this->height);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glBindVertexArray(vao);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
        
        this->unbind();
    }

//private:
    /** Bind this framebuffer object.*/
    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, this->id);
    //    std::cout << "Framebuffer Bounded: " << this->id << std::endl;
    }
    /** Unbind this framebuffer object.*/
    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //    std::cout << "Framebuffer Unbounded: " << this->id << std::endl;
    }

    void cleanup() {
        if (this->id != 0) {
            glDeleteFramebuffers(1, &this->id);
        }
        if (this->textureIDs != nullptr) {
            glDeleteTextures(1, this->textureIDs); // 텍스처 삭제
            free(this->textureIDs);
        }
        if (this->renderbuffer != 0) {
            glDeleteRenderbuffers(1, &this->renderbuffer); // 렌더버퍼 삭제
        }
    }

public:
    ~Framebuffer() {
        cleanup();
    }
};

#endif /* framebuffer_hpp */
