/*!
\file GBuffer.h
\brief Framebuffer class.
\author Leonardo Quatrin Campagnolo
*/

#ifndef LQC_GL_FRAMEBUFFEROBJECT_H
#define LQC_GL_FRAMEBUFFEROBJECT_H

#include <GL/glew.h>
#include <GL/glut.h>

#include <stdio.h>
#include <assert.h>
#include <vector>

/*! Class to store a framebuffer used in GLSLVisualization.
*/

class GBuffer
{
public:
  /*! Unbind the Framebuffer (bind GL_FRAMEBUFFER to 0).
  */
  static void Unbind ();

  void Bind ();

  /*! Enum with the color attachment types.
  */
  enum GBUFFER_TEXTURE_TYPE
  {
    GBUFFER_TEXTURE_FRONT_FACES,
    GBUFFER_TEXTURE_BACK_FACES,
    GBUFFER_NUM_TEXTURES
  };

  /*! Constructor
  \param width width of the new framebuffer.
  \param height height of the new framebuffer.
  */
  GBuffer (unsigned int width, unsigned int height);
  /*! Destructor
  */
  ~GBuffer ();

  /*! Print some FrameBuffer limits (GL_MAX_COLOR_ATTACHMENTS, GL_MAX_FRAMEBUFFER_WIDTH, GL_MAX_FRAMEBUFFER_HEIGHT, GL_MAX_FRAMEBUFFER_SAMPLES, GL_MAX_FRAMEBUFFER_LAYERS)
  */
  void PrintFramebufferLimits ();

  /*! Print a FrameBuffer Info.
  */
  void PrintFramebufferInfo (GLenum target, GLuint fbo);

  /*! Delete the current framebuffer and create another with the param size.
  \param width width of the new framebuffer.
  \param height height of the new framebuffer.
  */
  void resize (unsigned int width, unsigned int height);

  /*! Render all color attachments.
  \param width width value of the area available to be rendered.
  \param height height value of the area available to be rendered.
  */
  void renderBuffers (int width, int height);

  /*! Render just one color attachment.
  \param width width value of the area available to be rendered.
  \param height height value of the area available to be rendered.
  \param id index of the color attachment.
  */
  void renderBuffer (int width, int height, int id);

  /*! Get the Max number of color attachments.
  \return number of color attachments.
  */
  int GetMaxColorAttachments ();
  /*! Get the framebuffer width.
  \return FrameBuffer width.
  */
  int GetFramebufferWidth ();
  /*! Get the framebuffer height.
  \return FrameBuffer height.
  */
  int GetFramebufferHeight ();
  /*! Return the max number of samples.
  */
  int GetMaxSamples ();
  /*! Return the max number of layers.
  */
  int GetMaxLayers ();

  GLuint m_FrameBufferObject;

  GLuint m_colortexture;
  GLuint m_backtexture;
  GLuint m_subtracttexture;
  GLuint m_volumeTexture;
  GLuint m_depthtexture;
  unsigned int m_width;
  unsigned int m_height;
private:
  GLuint m_color_attachments[GL_MAX_COLOR_ATTACHMENTS];
  GLuint m_depth_attachment;
};


#endif