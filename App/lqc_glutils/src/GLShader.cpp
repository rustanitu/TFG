#include <glutils/GLShader.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <cstdio>

namespace gl
{
  GLShader::UniformVariable::UniformVariable ()
  {
    value = NULL;
    type = GLSL_VAR_TYPES::NONE;
    m_loc = -1;
  }

  GLShader::UniformVariable::~UniformVariable ()
  {}

  void GLShader::UniformVariable::Bind ()
  {
    float* aux;
    switch (type)
    {
    case GLSL_VAR_TYPES::INT:
      glUniform1i (m_loc, *(int*)value);
      break;
    case GLSL_VAR_TYPES::FLOAT:
      glUniform1f (m_loc, *(float*)value);
      break;
    case GLSL_VAR_TYPES::DOUBLE:
      glUniform1d (m_loc, *(double*)value);
      break;
      /*case VECTOR2:
      aux = (float*)value;
      glUniform2f (m_loc, aux[0], aux[1]);
      break;
      */
    case GLSL_VAR_TYPES::VECTOR3:
      aux = (float*)value;
      glUniform3f (m_loc, aux[0], aux[1], aux[2]);
      break;
      /*
      case VECTOR4:
      aux = (float*)value;
      glUniform4f (m_loc, aux[0], aux[1], aux[2], aux[3]);
      break;
      case MATRIX2:
      glUniformMatrix2fv (m_loc, 1, GL_FALSE, (float*)value);
      break;
      case MATRIX3:
      glUniformMatrix3fv (m_loc, 1, GL_FALSE, (float*)value);
      break;
      */
    case GLSL_VAR_TYPES::MATRIX4:
      glUniformMatrix4fv (m_loc, 1, GL_FALSE, (float*)value);
      break;
    case GLSL_VAR_TYPES::TEXTURE1D:
    {
      GLuint *aux = (GLuint*)value;
      glActiveTexture (GL_TEXTURE0 + aux[1]);
      glBindTexture (GL_TEXTURE_1D, aux[0]);
      glUniform1i (m_loc, aux[1]);
    }
    break;
    case GLSL_VAR_TYPES::TEXTURE2D:
    {
      GLuint *aux = (GLuint*)value;
      glActiveTexture (GL_TEXTURE0 + aux[1]);
      glBindTexture (GL_TEXTURE_2D, aux[0]);
      glUniform1i (m_loc, aux[1]);
    }
    break;
    case GLSL_VAR_TYPES::TEXTURE3D:
    {
      GLuint *aux = (GLuint*)value;
      glActiveTexture (GL_TEXTURE0 + aux[1]);
      glBindTexture (GL_TEXTURE_3D, aux[0]);
      glUniform1i (m_loc, aux[1]);
    }
    break;
    case GLSL_VAR_TYPES::VEC3:
      aux = (float*)value;
      glUniform3f (m_loc, aux[0], aux[1], aux[2]);
      break;
    case GLSL_VAR_TYPES::MAT4:
      glUniformMatrix4fv (m_loc, 1, GL_FALSE, (float*)value);
      break;
    };
  }

  void GLShader::UniformVariable::DestroyValue ()
  {
    if (value)
    {
      switch (type)
      {

      case GLSL_VAR_TYPES::INT:
      {
        int *v = (int*)value;
        delete v;
      }
      break;
      case GLSL_VAR_TYPES::FLOAT:
      {
        float *v = (float*)value;
        delete v;
      }
      break;
      case GLSL_VAR_TYPES::DOUBLE:
      {
        double *v = (double*)value;
        delete v;
      }
      break;
      /*case VECTOR2:
      */
      case GLSL_VAR_TYPES::VECTOR3:
      {
        float *v = (float*)value;
        delete[] v;
      }
      break;
      /*
      case VECTOR4:
      case MATRIX2:
      case MATRIX3:
      */
      case GLSL_VAR_TYPES::MATRIX4:
      {
        float *v = (float*)value;
        delete[] v;
      }
      break;
      case TEXTURE1D:
      {
        GLuint *v = (GLuint*)value;
        delete[] v;
      }
      break;
      case TEXTURE2D:
      {
        GLuint *v = (GLuint*)value;
        delete[] v;
      }
      break;
      case TEXTURE3D:
      {
        GLuint *v = (GLuint*)value;
        delete[] v;
      }
      break;

      //GLM VALUES
      case GLSL_VAR_TYPES::VEC3:
      {
        float *v = (float*)value;
        delete[] v;
      }
      break;
      case GLSL_VAR_TYPES::MAT4:
      {
        float *v = (float*)value;
        delete[] v;
      }
      break;
      //GLM VALUES
      };
    }
  }

  void GLShader::Unbind ()
  {
    glUseProgram (0);
  }

  GLShader::GLShader (std::string vert, std::string frag)
  {
    m_shader_program = m_shader_vertex = m_shader_geometry = m_shader_fragment = -1;
    m_vertexFileName.clear (); m_vertexFileName = vert;
    m_fragmentFileName.clear (); m_fragmentFileName = frag;

    m_hasGeometryShader = false;
    if (GLEW_ARB_vertex_shader)
    {
#ifdef _DEBUG
      printf ("Ready for GLSL - vertex and fragment units\n");
#endif
    }
    else {
      printf ("lqc: Error on GLShader\n");
      exit (1);
    }

    SetShaders (vert.c_str (), frag.c_str ());

    OglError (__FILE__, __LINE__);

    Unbind ();

    m_uniform_variables.clear ();
  }


  GLShader::GLShader (char *vert, char *frag, char* geom)
  {
    m_shader_program = m_shader_vertex = m_shader_geometry = m_shader_fragment = -1;
    m_vertexFileName.clear (); if (vert != NULL) m_vertexFileName.append (vert);
    m_fragmentFileName.clear (); if (frag != NULL) m_fragmentFileName.append (frag);
    m_geometryFileName.clear (); if (geom != NULL) m_geometryFileName.append (geom);

    if (geom != NULL)
    {
      m_hasGeometryShader = true;
      if (glewIsSupported ("GL_VERSION_4_0") && GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
      {
#ifdef _DEBUG
        printf ("Ready for GLSL - vertex, fragment, and geometry units\n");
#endif
      }
      else {
        printf ("lqc: Error on GLShader\n");
        exit (1);
      }
    }
    else
    {
      m_hasGeometryShader = false;
      if (GLEW_ARB_vertex_shader)
      {
#ifdef _DEBUG
        printf ("Ready for GLSL - vertex and fragment units\n");
#endif
      }
      else {
        printf ("lqc: Error on GLShader\n");
        exit (1);
      }
    }

    SetShaders (vert, frag, geom);

    OglError (__FILE__, __LINE__);

    Unbind ();

    m_uniform_variables.clear ();
  }

  GLShader::~GLShader ()
  {
    Clear ();
    ClearUniforms ();
    gl::ExitOnGLError ("ERROR: Could not destroy the shaders");
  }



  void GLShader::Bind ()
  {
    glUseProgram (m_shader_program);
    gl::ExitOnGLError ("ERROR: Could not use the shader program");
  }

  void GLShader::Clear ()
  {
    Unbind ();
    glDetachShader (m_shader_program, m_shader_fragment);
    if (m_hasGeometryShader) glDetachShader (m_shader_program, m_shader_geometry);
    glDetachShader (m_shader_program, m_shader_vertex);
    glDeleteShader (m_shader_fragment);
    if (m_hasGeometryShader) glDeleteShader (m_shader_geometry);
    glDeleteShader (m_shader_vertex);
    glDeleteProgram (m_shader_program);
  }

  void GLShader::LinkProgram ()
  {
    glLinkProgram (m_shader_program);
    ProgramInfoLog (m_shader_program);
  }

  void GLShader::Reload ()
  {
    Clear ();

    ReloadShaders ();
    Bind ();

    //SetUniforms
    for (std::map<std::string, GLShader::UniformVariable>::iterator it = m_uniform_variables.begin (); it != m_uniform_variables.end (); ++it)
      m_uniform_variables[it->first].m_loc = glGetUniformLocation (m_shader_program, it->first.c_str ());
    BindUniforms ();

    //SetAttributes

    printf ("Shader reloaded with id = %d\n", m_shader_program);
    printf ("  - Vertex   File: %s\n", m_vertexFileName.c_str ());
    printf ("  - Geometry File: %s\n", m_hasGeometryShader ? m_geometryFileName.c_str () : "NULL");
    printf ("  - Fragment File: %s\n", m_fragmentFileName.c_str ());
  }

  GLint GLShader::GetUniformLoc (char* name)
  {
    int uniform_location = glGetUniformLocation (m_shader_program, name);
    return uniform_location;
  }

  GLint GLShader::GetAttribLoc (char* name)
  {
    int attrib_location = glGetAttribLocation (m_shader_program, name);
    return attrib_location;
  }

  GLuint GLShader::GetProgramID ()
  {
    return m_shader_program;
  }

  void GLShader::BindUniforms ()
  {
    for (std::map<std::string, GLShader::UniformVariable>::iterator it = m_uniform_variables.begin (); it != m_uniform_variables.end (); ++it)
      m_uniform_variables[it->first].Bind ();
    gl::ExitOnGLError ("ERROR: Could not set the shader uniforms");
  }

  void GLShader::ClearUniforms ()
  {
    for (std::map<std::string, GLShader::UniformVariable>::iterator it = m_uniform_variables.begin (); it != m_uniform_variables.end (); ++it)
      it->second.DestroyValue ();
    m_uniform_variables.clear ();
  }

  void GLShader::BindUniform (std::string var_name)
  {
    if (m_uniform_variables.find (var_name) != m_uniform_variables.end ())
    {
      m_uniform_variables[var_name].Bind ();
      gl::ExitOnGLError ("ERROR: Could not Bind Uniform");
    }
    else
    {
      std::cout << "lqc: Uniform " << var_name << " Not Found!" << std::endl;
    }
  }

  void GLShader::SetUniformInt (std::string name, int value)
  {
    int* input_uniform = new int ();
    *input_uniform = value;

    if (m_uniform_variables.find (name) != m_uniform_variables.end ())
    {
      m_uniform_variables[name].DestroyValue ();
      m_uniform_variables[name].value = input_uniform;
    }
    else
    {
      GLShader::UniformVariable ul;
      ul.type = GLSL_VAR_TYPES::INT;
      ul.m_loc = glGetUniformLocation (m_shader_program, name.c_str ());
      ul.value = input_uniform;
      m_uniform_variables.insert (std::pair<std::string, GLShader::UniformVariable> (name, ul));
    }
    gl::ExitOnGLError ("ERROR: Could not set shader Uniform");
  }

  void GLShader::SetUniformFloat (std::string name, float value)
  {
    float* input_uniform = new float ();
    *input_uniform = value;

    if (m_uniform_variables.find (name) != m_uniform_variables.end ())
    {
      m_uniform_variables[name].DestroyValue ();
      m_uniform_variables[name].value = input_uniform;
    }
    else
    {
      GLShader::UniformVariable ul;
      ul.type = GLSL_VAR_TYPES::FLOAT;
      ul.m_loc = glGetUniformLocation (m_shader_program, name.c_str ());
      ul.value = input_uniform;
      m_uniform_variables.insert (std::pair<std::string, GLShader::UniformVariable> (name, ul));
    }
    gl::ExitOnGLError ("ERROR: Could not set shader Uniform");
  }

  void GLShader::SetUniformDouble (std::string name, double value)
  {
    double* input_uniform = new double ();
    *input_uniform = value;

    if (m_uniform_variables.find (name) != m_uniform_variables.end ())
    {
      m_uniform_variables[name].DestroyValue ();
      m_uniform_variables[name].value = input_uniform;
    }
    else
    {
      GLShader::UniformVariable ul;
      ul.type = GLSL_VAR_TYPES::DOUBLE;
      ul.m_loc = glGetUniformLocation (m_shader_program, name.c_str ());
      ul.value = input_uniform;
      m_uniform_variables.insert (std::pair<std::string, GLShader::UniformVariable> (name, ul));
    }
    gl::ExitOnGLError ("ERROR: Could not set shader Uniform");
  }

  void GLShader::SetUniformVector3f (std::string name, lqc::Vector3f value)
  {
    float* input_uniform = new float[3];
    input_uniform[0] = value.x;
    input_uniform[1] = value.y;
    input_uniform[2] = value.z;

    if (m_uniform_variables.find (name) != m_uniform_variables.end ())
    {
      m_uniform_variables[name].DestroyValue ();
      m_uniform_variables[name].value = input_uniform;
    }
    else
    {
      GLShader::UniformVariable ul;
      ul.type = GLSL_VAR_TYPES::VECTOR3;
      ul.m_loc = glGetUniformLocation (m_shader_program, name.c_str ());
      ul.value = input_uniform;
      m_uniform_variables.insert (std::pair<std::string, GLShader::UniformVariable> (name, ul));
    }
    gl::ExitOnGLError ("ERROR: Could not set shader Uniform");
  }

  void GLShader::SetUniformMatrix4f (std::string name, lqc::Matrix4f value)
  {
    float* input_uniform = new float[16];
    input_uniform[0] = value.m[0];   input_uniform[1] = value.m[1];   input_uniform[2] = value.m[2];   input_uniform[3] = value.m[3];
    input_uniform[4] = value.m[4];   input_uniform[5] = value.m[5];   input_uniform[6] = value.m[6];   input_uniform[7] = value.m[7];
    input_uniform[8] = value.m[8];   input_uniform[9] = value.m[9];   input_uniform[10] = value.m[10]; input_uniform[11] = value.m[11];
    input_uniform[12] = value.m[12]; input_uniform[13] = value.m[13]; input_uniform[14] = value.m[14]; input_uniform[15] = value.m[15];

    if (m_uniform_variables.find (name) != m_uniform_variables.end ())
    {
      m_uniform_variables[name].DestroyValue ();
      m_uniform_variables[name].value = input_uniform;
    }
    else
    {
      GLShader::UniformVariable ul;
      ul.type = GLSL_VAR_TYPES::MATRIX4;
      ul.m_loc = glGetUniformLocation (m_shader_program, name.c_str ());
      ul.value = input_uniform;
      m_uniform_variables.insert (std::pair<std::string, GLShader::UniformVariable> (name, ul));
    }
    gl::ExitOnGLError ("ERROR: Could not set shader Uniform");
  }

  void GLShader::SetUniformTexture1D (std::string name, GLuint textureid, GLuint activeTextureid)
  {
    SetUniformTexture (name, textureid, activeTextureid, GLSL_VAR_TYPES::TEXTURE1D);
  }

  void GLShader::SetUniformTexture2D (std::string name, GLuint textureid, GLuint activeTextureid)
  {
    SetUniformTexture (name, textureid, activeTextureid, GLSL_VAR_TYPES::TEXTURE2D);
  }

  void GLShader::SetUniformTexture3D (std::string name, GLuint textureid, GLuint activeTextureid)
  {
    SetUniformTexture (name, textureid, activeTextureid, GLSL_VAR_TYPES::TEXTURE3D);
  }



  void GLShader::SetUniformVec3 (std::string name, glm::vec3 value)
  {
    float* input_uniform = new float[3];
    input_uniform[0] = value.x;
    input_uniform[1] = value.y;
    input_uniform[2] = value.z;

    if (m_uniform_variables.find (name) != m_uniform_variables.end ())
    {
      m_uniform_variables[name].DestroyValue ();
      m_uniform_variables[name].value = input_uniform;
    }
    else
    {
      GLShader::UniformVariable ul;
      ul.type = GLSL_VAR_TYPES::VEC3;
      ul.m_loc = glGetUniformLocation (m_shader_program, name.c_str ());
      ul.value = input_uniform;
      m_uniform_variables.insert (std::pair<std::string, GLShader::UniformVariable> (name, ul));
    }
    gl::ExitOnGLError ("ERROR: Could not set shader Uniform");
  }

  void GLShader::SetUniformMat4 (std::string name, glm::mat4 value)
  {
    float* input_uniform = new float[16];
    input_uniform[0] = value[0][0];
    input_uniform[1] = value[0][1];
    input_uniform[2] = value[0][2];
    input_uniform[3] = value[0][3];
    input_uniform[4] = value[1][0];
    input_uniform[5] = value[1][1];
    input_uniform[6] = value[1][2];
    input_uniform[7] = value[1][3];
    input_uniform[8] = value[2][0];
    input_uniform[9] = value[2][1];
    input_uniform[10] = value[2][2];
    input_uniform[11] = value[2][3];
    input_uniform[12] = value[3][0];
    input_uniform[13] = value[3][1];
    input_uniform[14] = value[3][2];
    input_uniform[15] = value[3][3];

    if (m_uniform_variables.find (name) != m_uniform_variables.end ())
    {
      m_uniform_variables[name].DestroyValue ();
      m_uniform_variables[name].value = input_uniform;
    }
    else
    {
      GLShader::UniformVariable ul;
      ul.type = GLSL_VAR_TYPES::MAT4;
      ul.m_loc = glGetUniformLocation (m_shader_program, name.c_str ());
      ul.value = input_uniform;
      m_uniform_variables.insert (std::pair<std::string, GLShader::UniformVariable> (name, ul));
    }
    gl::ExitOnGLError ("ERROR: Could not set shader Uniform");
  }














  void GLShader::SetGeometryShaderPrimitives (GLenum in, GLenum out)
  {
    m_geometryPrimitiveIn = in;
    m_geometryPrimitiveOut = out;
    int temp;
    glProgramParameteriEXT (m_shader_program, GL_GEOMETRY_INPUT_TYPE_EXT, in);
    glProgramParameteriEXT (m_shader_program, GL_GEOMETRY_OUTPUT_TYPE_EXT, out);
    glGetIntegerv (GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
    glProgramParameteriEXT (m_shader_program, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
  }

  void GLShader::SetGeometryShaderPrimitives (GS_INPUT in, GS_OUTPUT out)
  {
    m_geometryPrimitiveIn = in;
    m_geometryPrimitiveOut = out;
    int temp;
    glProgramParameteriEXT (m_shader_program, GL_GEOMETRY_INPUT_TYPE_EXT, in);
    glProgramParameteriEXT (m_shader_program, GL_GEOMETRY_OUTPUT_TYPE_EXT, out);
    glGetIntegerv (GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
    glProgramParameteriEXT (m_shader_program, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
  }

  void GLShader::ReloadShaders ()
  {
    m_shader_program = glCreateProgram ();
    gl::ExitOnGLError ("ERROR: Could not create the shader program");

    m_shader_vertex = LoadShader (m_vertexFileName.c_str (), GL_VERTEX_SHADER);
    glAttachShader (m_shader_program, m_shader_vertex);
#ifdef _DEBUG
    ShaderInfoLog (m_shader_vertex);
#endif

    m_shader_fragment = LoadShader (m_fragmentFileName.c_str (), GL_FRAGMENT_SHADER);
    glAttachShader (m_shader_program, m_shader_fragment);
#ifdef _DEBUG
    ShaderInfoLog (m_shader_fragment);
#endif

    if (m_hasGeometryShader)
    {
      m_shader_geometry = LoadShader (m_geometryFileName.c_str (), GL_GEOMETRY_SHADER);
      glAttachShader (m_shader_program, m_shader_geometry);
#ifdef _DEBUG
      ShaderInfoLog (m_shader_geometry);
#endif
    }

    glLinkProgram (m_shader_program);
#ifdef _DEBUG
    ProgramInfoLog (m_shader_program);
#endif
  }

  void GLShader::SetShaders (const char* vert, const char* frag, const char* geom)
  {
    m_shader_program = glCreateProgram ();
    gl::ExitOnGLError ("ERROR: Could not create the shader program");

    char *vertex_shader_source;
    char *geometry_shader_source;
    char *fragment_shader_source;

    m_shader_vertex = glCreateShader (GL_VERTEX_SHADER);
    m_vertexFileName.clear ();
    m_vertexFileName.append (vert);
    vertex_shader_source = TextFileRead (vert);
    const char * const_vertex_shader_source = vertex_shader_source;
    glShaderSource (m_shader_vertex, 1, &const_vertex_shader_source, NULL);
    free (vertex_shader_source);
    glCompileShader (m_shader_vertex);
    glAttachShader (m_shader_program, m_shader_vertex);
#ifdef _DEBUG
    ShaderInfoLog (m_shader_vertex);
#endif

    m_shader_fragment = glCreateShader (GL_FRAGMENT_SHADER);
    m_fragmentFileName.clear ();
    m_fragmentFileName.append (frag);
    fragment_shader_source = TextFileRead (frag);
    const char * const_fragment_shader_source = fragment_shader_source;
    glShaderSource (m_shader_fragment, 1, &const_fragment_shader_source, NULL);
    free (fragment_shader_source);
    glCompileShader (m_shader_fragment);
    glAttachShader (m_shader_program, m_shader_fragment);
#ifdef _DEBUG
    ShaderInfoLog (m_shader_fragment);
#endif

    if (geom != NULL)
    {
      // ou GL_GEOMETRY_SHADER_EXT
      m_shader_geometry = glCreateShader (GL_GEOMETRY_SHADER_EXT);
      m_geometryFileName.clear ();
      m_geometryFileName.append (geom);
      geometry_shader_source = TextFileRead (geom);
      const char * const_geometry_shader_source = geometry_shader_source;
      glShaderSource (m_shader_geometry, 1, &const_geometry_shader_source, NULL);
      free (geometry_shader_source);
      glCompileShader (m_shader_geometry);
      glAttachShader (m_shader_program, m_shader_geometry);
      ShaderInfoLog (m_shader_geometry);
    }

    glLinkProgram (m_shader_program);
    ExitOnGLError ("ERROR: Could not link the shader program");

#ifdef _DEBUG
    ProgramInfoLog (m_shader_program);
#endif
  }

  char* GLShader::TextFileRead (const char* file_name)
  {
#ifdef _DEBUG
    printf ("File Name TextFileRead \"%s\"\n", file_name);
#endif
    FILE *file_source;
    char *content = NULL;
    int count = 0;
    if (file_name != NULL)
    {
      file_source = fopen (file_name, "rt");

      if (file_source != NULL)
      {
        fseek (file_source, 0, SEEK_END);
        count = ftell (file_source);
        rewind (file_source);

        if (count > 0) {
          content = (char *)malloc (sizeof (char)* (count + 1));
          count = (int)fread (content, sizeof (char), count, file_source);
          content[count] = '\0';
        }
        fclose (file_source);
      }
      else
      {
        printf ("\nFile \"%s\" not found", file_name);
        getchar ();
        exit (1);
      }
    }
    return content;
  }

  void GLShader::SetUniformTexture (std::string name, GLuint textureid, GLuint activeTextureid, GLSL_VAR_TYPES type)
  {
    GLuint* input_uniform = new GLuint[2];
    input_uniform[0] = textureid;
    input_uniform[1] = activeTextureid;

    if (m_uniform_variables.find (name) != m_uniform_variables.end ())
    {
      m_uniform_variables[name].DestroyValue ();
      m_uniform_variables[name].value = input_uniform;
    }
    else
    {
      GLShader::UniformVariable ul;
      ul.type = type;
      ul.m_loc = glGetUniformLocation (m_shader_program, name.c_str ());
      ul.value = input_uniform;
      m_uniform_variables.insert (std::pair<std::string, GLShader::UniformVariable> (name, ul));
    }
    gl::ExitOnGLError ("ERROR: Could not set shader Uniform");
  }

  GLShader2::GLShader2 ()
    : id_prog_shaders (0)
  {}

  GLShader2::~GLShader2 ()
  {}

  void GLShader2::Unbind ()
  {
    glUseProgram (0);
  }

  void GLShader2::Bind ()
  {
    glUseProgram (id_prog_shaders);
    gl::ExitOnGLError ("GLSLShader::Bind");
  }

  void GLShader2::AttachVertexShader (char* filepath)
  {
    GLuint sdr_vertex = glCreateShader (GL_VERTEX_SHADER);

    char *vtx_sdr_src = TextFileRead (filepath);
    const char * const_vss = vtx_sdr_src;

    glShaderSource (sdr_vertex, 1, &const_vss, NULL);
    glCompileShader (sdr_vertex);
    glAttachShader (id_prog_shaders, sdr_vertex);
    free (vtx_sdr_src);

    txt_vert_shaders.push_back (filepath);
    id_vert_shaders.push_back (sdr_vertex);

#ifdef _DEBUG
    ShaderInfoLog (sdr_vertex);
#endif
    gl::ExitOnGLError ("GLShader::AttachVertexShader");
  }

  void GLShader2::AttachFragmentShader (char* filepath)
  {
    GLuint sdr_fragment = glCreateShader (GL_FRAGMENT_SHADER);

    char *frg_sdr_src = TextFileRead (filepath);
    const char * const_fss = frg_sdr_src;

    glShaderSource (sdr_fragment, 1, &const_fss, NULL);
    glCompileShader (sdr_fragment);
    glAttachShader (id_prog_shaders, sdr_fragment);
    free (frg_sdr_src);

    txt_frag_shaders.push_back (filepath);
    id_frag_shaders.push_back (sdr_fragment);

#ifdef _DEBUG
    ShaderInfoLog (sdr_fragment);
#endif
    gl::ExitOnGLError ("GLShader::AttachFragmentShader");
  }

  void GLShader2::Link ()
  {
    glLinkProgram (id_prog_shaders);
    gl::ExitOnGLError ("GLShader::Link");
  }

  void GLShader2::Destroy ()
  {
    for (unsigned i = 0; i < id_vert_shaders.size (); i++)
      glDeleteShader (id_vert_shaders[i]);
    for (unsigned i = 0; i < id_frag_shaders.size (); i++)
      glDeleteShader (id_frag_shaders[i]);
    if (id_prog_shaders != 0)
      glDeleteProgram (id_prog_shaders);
    gl::ExitOnGLError ("GLShader::Destroy");
  }
}