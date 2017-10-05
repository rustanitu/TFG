//References:
// GLSLProgramObject file from Nvidia dual depth peeling sample
// link: http://developer.download.nvidia.com/SDK/10/opengl/screenshots/samples/dual_depth_peeling.html

#ifndef LQC_GL_SHADER_H
#define LQC_GL_SHADER_H

#include <glutils/GLUtils.h>
#include <math/Vector2.h>
#include <math/Vector3.h>
#include <math/Vector4.h>
#include <math/Matrix.h>
#include <math/Matrix4.h>

#include <map>

//Matrix has Comlunm major order
namespace gl
{
  enum GLSL_VAR_TYPES
  {
    INT,
    FLOAT,
    DOUBLE,
    //VECTOR2,
    VECTOR3,
    //VECTOR4,
    //MATRIX2,
    //MATRIX3,
    MATRIX4,
    TEXTURE1D,
    TEXTURE2D,
    TEXTURE3D,
    VEC3,
    MAT4,
    NONE,
  };

  class GLShader
  {
  private:
    class UniformVariable
    {
    public:
      UniformVariable ();
      ~UniformVariable ();

      void Bind ();
      void DestroyValue ();

      void* value;
      GLSL_VAR_TYPES type;
      GLuint m_loc;
    };

  public:
	  static void Unbind();

    GLShader (std::string vert, std::string frag);
    GLShader (char *vert, char *frag, char* geom = NULL);
    ~GLShader ();

    void Bind ();
    void Clear ();

    void LinkProgram ();
    void Reload ();

    GLint GetUniformLoc (char* name);
    GLint GetAttribLoc (char* name);

    GLuint GetProgramID ();

    ////////////////////////
    // Uniforms functions //
    ////////////////////////
    void BindUniforms ();
    void ClearUniforms ();
    void BindUniform (std::string var_name);
    void SetUniformInt (std::string name, int value);
    void SetUniformFloat (std::string name, float value);
    void SetUniformDouble (std::string name, double value);
    void SetUniformVector3f (std::string name, glm::vec3 value);
    void SetUniformMatrix4f (std::string name, lqc::Matrix4f value);
    void SetUniformTexture1D (std::string name, GLuint textureid, GLuint activeTextureid);
    void SetUniformTexture2D (std::string name, GLuint textureid, GLuint activeTextureid);
    void SetUniformTexture3D (std::string name, GLuint textureid, GLuint activeTextureid);

    void SetUniformVec3 (std::string name, glm::vec3 value);
    void SetUniformMat4 (std::string name, glm::mat4 value);

    enum GS_INPUT
    {
      IN_POINTS = GL_POINTS,
      IN_LINES = GL_LINES,
      IN_LINES_ADJACENCY = GL_LINES_ADJACENCY,
      IN_TRIANGLES = GL_TRIANGLES,
      IN_TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY
    };

    enum GS_OUTPUT
    {
      OUT_POINTS = GL_POINTS,
      OUT_LINE_STRIP = GL_LINE_STRIP,
      OUT_TRIANGLE_STRIP = GL_TRIANGLE_STRIP
    };

    void SetGeometryShaderPrimitives (GLenum in, GLenum out);
    void SetGeometryShaderPrimitives (GLShader::GS_INPUT in, GLShader::GS_OUTPUT out);

  private:
    GLuint m_shader_program;
    GLuint m_shader_vertex;
    GLuint m_shader_geometry;
    GLuint m_shader_fragment;
    GLenum m_geometryPrimitiveIn;
    GLenum m_geometryPrimitiveOut;

    std::string m_vertexFileName;
    std::string m_geometryFileName;
    std::string m_fragmentFileName;

    std::map<std::string, GLShader::UniformVariable> m_uniform_variables;

    bool m_hasGeometryShader;

    void ReloadShaders ();
    void SetShaders (const char* vert, const char* frag, const char* geom = NULL);
    char* TextFileRead (const char* file_name);
    void SetUniformTexture (std::string name, GLuint textureid, GLuint activeTextureid, GLSL_VAR_TYPES type);
  };

  class GLShader2
  {
  public:
    static void Unbind ();
    void Bind ();

    GLShader2 ();
    ~GLShader2 ();

    void AttachVertexShader (char* filepath);
    void AttachFragmentShader (char* filepath);
    void Link ();
    void Destroy ();
  private:
    GLuint id_prog_shaders;
    std::vector<std::string> txt_vert_shaders;
    std::vector<std::string> txt_frag_shaders;
    std::vector<GLuint> id_vert_shaders;
    std::vector<GLuint> id_frag_shaders;
  };
}

#endif