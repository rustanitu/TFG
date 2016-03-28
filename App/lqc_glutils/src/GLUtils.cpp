#include <glutils/GLUtils.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

namespace gl
{
  void ExitOnGLError(const char* error_message)
  {
    const GLenum ErrorValue = glGetError();

    if (ErrorValue != GL_NO_ERROR)
    {
      const char* APPEND_DETAIL_STRING = ": %s\n";
      const size_t APPEND_LENGTH = strlen(APPEND_DETAIL_STRING) + 1;
      const size_t message_length = strlen(error_message);
      char* display_message = (char*)malloc(message_length + APPEND_LENGTH);

      memcpy(display_message, error_message, message_length);
      memcpy(&display_message[message_length], APPEND_DETAIL_STRING, APPEND_LENGTH);

      fprintf(stderr, display_message, gluErrorString(ErrorValue));

      free(display_message);
      exit(EXIT_FAILURE);
    }
  }

  void ShaderInfoLog(GLuint obj)
  {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0)
    {
      infoLog = (char *)malloc(infologLength);
      glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
      printf("%s\n",infoLog);
      free(infoLog);
    }else
    {
      printf("Shader Info Log: OK\n");
    }
  }

  void ProgramInfoLog(GLuint obj)
  {
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0)
    {
      infoLog = (char *)malloc(infologLength);
      glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
      printf("%s\n",infoLog);
      free(infoLog);
    }else
    {
      printf("Program Info Log: OK\n");
    }
  }

  int OglError(char *file, int line)
  {
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
      printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
      retCode = 1;
      glErr = glGetError();
    }
    return retCode;
  }

  GLuint LoadShader(const char* file_name, GLenum shader_type)
  {
    GLuint shader_id = 0;
    FILE* file;
    long file_size = -1;
    char* glsl_source;

    if (NULL != (file = fopen(file_name, "rb")) &&
      0 == fseek(file, 0, SEEK_END) &&
      -1 != (file_size = ftell(file)))
    {
      rewind(file);

      if (NULL != (glsl_source = (char*)malloc(file_size + 1)))
      {
        if (file_size == (long)fread(glsl_source, sizeof(char), file_size, file))
        {
          glsl_source[file_size] = '\0';

          if (0 != (shader_id = glCreateShader(shader_type)))
          {
            const char* const_glsl_source = glsl_source;
            glShaderSource(shader_id, 1, &const_glsl_source, NULL);
            glCompileShader(shader_id);

            const GLenum ErrorValue = glGetError();
            if (ErrorValue != GL_NO_ERROR)
            {
              const char* APPEND_DETAIL_STRING = ": %s\n";
              const size_t APPEND_LENGTH = strlen(APPEND_DETAIL_STRING) + 1;
              const size_t message_length = strlen("Could not compile a shader");
              char* display_message = (char*)malloc(message_length + APPEND_LENGTH);

              memcpy(display_message, "Could not compile a shader", message_length);
              memcpy(&display_message[message_length], APPEND_DETAIL_STRING, APPEND_LENGTH);

              fprintf(stderr, display_message, gluErrorString(ErrorValue));

              free(display_message);
              exit(EXIT_FAILURE);
            }
          }
          else
            fprintf(stderr, "ERROR: Could not create a shader.\n");
        }
        else
          fprintf(stderr, "ERROR: Could not read file %s\n", file_name);

        free(glsl_source);
      }
      else
        fprintf(stderr, "ERROR: Could not allocate %i bytes.\n", file_size);

      fclose(file);
    }
    else
      fprintf(stderr, "ERROR: Could not open file %s\n", file_name);

    return shader_id;
  }

  char* TextFileRead (const char* file_name)
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
}