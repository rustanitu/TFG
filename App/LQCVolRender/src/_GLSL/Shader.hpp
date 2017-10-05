#ifndef GLSL_SHADER_HPP
#define GLSL_SHADER_HPP

#include <glutils/GLShader.h>

#include <iostream>

class RendererShader
{
public:
  RendererShader (lqc::GLShader* shader)
    : m_shader(shader) {}
  ~RendererShader () {}

  virtual void SetExclusiveUniforms () {}
protected:
  lqc::GLShader* m_shader;
private:
};

class RiemannRendererShader : public RendererShader
{
public:
  RiemannRendererShader (lqc::GLShader* shader)
    : RendererShader(shader) {}
  ~RiemannRendererShader () {}

  virtual void SetExclusiveUniforms ()
  {
    std::cout << "Set Uniform variables:" << std::endl;
    std::cout << "  - \"step_size\": ";
    std::cin >> m_step_size;
    std::cout << "  - \"render_mode\": ";
    std::cin >> m_render_mode;

    m_shader->Bind ();
    m_shader->SetUniformFloat ("step_size", m_step_size);
    m_shader->BindUniform ("step_size");
    m_shader->SetUniformInt ("render_mode", m_render_mode);
    m_shader->BindUniform ("render_mode");
    m_shader->Unbind ();
  }

protected:
private:
  float m_step_size;
  int m_render_mode;
};

class AdapSimpRendererShader : public RendererShader
{
public:
  AdapSimpRendererShader (lqc::GLShader* shader)
    : RendererShader (shader) {}
  ~AdapSimpRendererShader () {}

  virtual void SetExclusiveUniforms ()
  {
    std::cout << "Set Uniform variables:" << std::endl;
    std::cout << "  - \"initial_step\": ";
    std::cin >> m_initial_step;
    std::cout << "  - \"tol_error\": ";
    std::cin >> m_tol_error;

    m_shader->Bind ();
    m_shader->SetUniformFloat ("initial_step", m_initial_step);
    m_shader->BindUniform ("initial_step");
    m_shader->SetUniformFloat ("tol_error", m_tol_error);
    m_shader->BindUniform ("tol_error");
    m_shader->Unbind ();
  }

protected:
private:
  float m_initial_step;
  int m_tol_error;
};

#endif