#include "gbuffer.h"



  void GBuffer::Unbind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  
  void GBuffer::Bind ()
  {
    glBindFramebuffer (GL_FRAMEBUFFER, m_FrameBufferObject);
  }

  GBuffer::GBuffer(unsigned int screenWidth, unsigned int screenHeight) : m_width(screenWidth), m_height(screenHeight)
  {
    glGenFramebuffers(1, &m_FrameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferObject);

    glGenTextures(1, &m_colortexture);
    glBindTexture(GL_TEXTURE_2D, m_colortexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_backtexture);
    glBindTexture(GL_TEXTURE_2D, m_backtexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_subtracttexture);
    glBindTexture(GL_TEXTURE_2D, m_subtracttexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_volumeTexture);
    glBindTexture(GL_TEXTURE_2D, m_volumeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_depthtexture);
    glBindTexture(GL_TEXTURE_2D, m_depthtexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, screenWidth, screenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);


    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colortexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_backtexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_subtracttexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, m_volumeTexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthtexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferObject);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      printf("lqc: There is a problem with the Frame Buffer Object.\n");
    }
    else
    {
#ifdef _DEBUG
      printf("lqc: Frame Buffer Object created.\n");
#endif
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  GBuffer::~GBuffer()
  {
    glDeleteTextures(1, &m_colortexture);
    glDeleteTextures(1, &m_backtexture);
    glDeleteTextures(1, &m_subtracttexture);
    glDeleteTextures(1, &m_volumeTexture);
    glDeleteTextures(1, &m_depthtexture);
    glDeleteFramebuffers(1, &m_FrameBufferObject);

  }

  void GBuffer::resize(unsigned int width, unsigned int height)
  {
    glDeleteTextures(1, &m_colortexture);
    glDeleteTextures(1, &m_backtexture);
    glDeleteTextures(1, &m_subtracttexture);
    glDeleteTextures(1, &m_volumeTexture);
    glDeleteTextures(1, &m_depthtexture);

    m_width = width;
    m_height = height;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBufferObject);

    glGenTextures(1, &m_colortexture);
    glBindTexture(GL_TEXTURE_2D, m_colortexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_backtexture);
    glBindTexture(GL_TEXTURE_2D, m_backtexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_subtracttexture);
    glBindTexture(GL_TEXTURE_2D, m_subtracttexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_volumeTexture);
    glBindTexture(GL_TEXTURE_2D, m_volumeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenTextures(1, &m_depthtexture);
    glBindTexture(GL_TEXTURE_2D, m_depthtexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colortexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_backtexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_subtracttexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, m_volumeTexture, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthtexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferObject);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      printf("lqc: error on initialization, frameBuffer isn't complete.");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  void GBuffer::renderBuffers (int width, int height)
  {
    //output frame = screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //input frame = gBuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBufferObject);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, height/2, width/2, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, m_width, m_height, width/2, height/2, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glReadBuffer(GL_COLOR_ATTACHMENT0 + 2);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, width/2, height/2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    
    glReadBuffer(GL_COLOR_ATTACHMENT0 + 3);
    glBlitFramebuffer(0, 0, m_width, m_height, width/2, 0, width, height/2, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  void GBuffer::renderBuffer (int width, int height, int id)
  {
    //output frame = screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //input frame = gBuffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FrameBufferObject);

    glReadBuffer(GL_COLOR_ATTACHMENT0 + id);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  void GBuffer::PrintFramebufferLimits() 
  {
    int res;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
    printf("lqc: Max Color Attachments on Framebuffer: %d\n", res);

    glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &res);
    printf("lqc: Max Framebuffer Width: %d\n", res);

    glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &res);
    printf("lqc: Max Framebuffer Height: %d\n", res);

    glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &res);
    printf("lqc: Max Framebuffer Samples: %d\n", res);

    glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &res);
    printf("lqc: Max Framebuffer Layers: %d\n", res);

  }

  void GBuffer::PrintFramebufferInfo (GLenum target, GLuint fbo)
  {
    int res, i = 0;
    GLint buffer;

    glBindFramebuffer(target,fbo);

    do {
      glGetIntegerv(GL_DRAW_BUFFER0+i, &buffer);

      if (buffer != GL_NONE) {

        printf("lqc: Shader Output Location %d - color attachment %d\n", 
          i, buffer - GL_COLOR_ATTACHMENT0);

        glGetFramebufferAttachmentParameteriv(target, buffer,
          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &res);
        printf("\tAttachment Type: %s\n", 
          res==GL_TEXTURE?"Texture":"Render Buffer");
        glGetFramebufferAttachmentParameteriv(target, buffer, 
          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &res);
        printf("\tAttachment object name: %d\n",res);
      }
      ++i;

    } while (buffer != GL_NONE);
  }

  int GBuffer::GetMaxColorAttachments ()
  {
    int res;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &res);
    return res;
  }

  int GBuffer::GetFramebufferWidth ()
  {
    int res;
    glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &res);
    return res;
  }

  int GBuffer::GetFramebufferHeight ()
  {
    int res;
    glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &res);
    return res;
  }

  int GBuffer::GetMaxSamples ()
  {
    int res;
    glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &res);
    return res;
  }

  int GBuffer::GetMaxLayers ()
  {
    int res;
    glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &res);
    return res;
  }
