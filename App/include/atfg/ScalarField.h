#ifndef SCALAR_FIELD
#define SCALAR_FIELD

#include <basetsd.h>
#include <string>
#include <math/MUtils.h>

#include "DerivativeMask.h"

#define MASK_SIZE 3

namespace vr
{

	class ScalarField
	{
	public:
		ScalarField() : m_width(0), m_height(0), m_depth(0), 
			m_max_value(-DBL_MAX), m_min_value(DBL_MAX), m_max_gradient(-DBL_MAX), m_min_gradient(DBL_MAX), m_min_laplacian(DBL_MAX), m_max_laplacian(-DBL_MAX)
      , m_derivativeMask(MASK_SIZE)
      , m_scale(1.0f)
      , m_grad(NULL)
		{
			printf("ScalarField criado.\n");
		}

		ScalarField(const UINT32& width, const UINT32& height, const UINT32& depth)
			: m_width(width), m_height(height), m_depth(depth)
			, m_max_value(-DBL_MAX), m_min_value(DBL_MAX), m_max_gradient(-DBL_MAX), m_min_gradient(DBL_MAX), m_min_laplacian(DBL_MAX), m_max_laplacian(-DBL_MAX)
      , m_derivativeMask(MASK_SIZE)
      , m_scale(1.0f)
      , m_grad(NULL)
		{
			printf("ScalarField criado.\n");
		}

    virtual ~ScalarField()
    {
      delete[] m_grad;
      printf("ScalarField destruido.\n");
    }

		int GetId(const int& x, const int& y, const int& z) const
		{
			UINT32 xt = std::min(std::max(0, x), (int) m_width - 1);
			UINT32 yt = std::min(std::max(0, y), (int) m_height - 1);
			UINT32 zt = std::min(std::max(0, z), (int) m_depth - 1);
			return xt + (yt * m_width) + (zt * m_width * m_height);
		}

		int GetWidth() const
		{
			return m_width;
		}

		int GetHeight() const
		{
			return m_height;
		}

		int GetDepth() const
		{
			return m_depth;
		}

		double GetMinValue() const
		{
			return m_min_value;
		}

		double GetMaxValue() const
		{
			return m_max_value;
		}

		double GetMaxGradient() const
		{
			return m_max_gradient;
		}

		double GetMinGradient() const
		{
			return m_min_gradient;
		}

		double GetMinLaplacian() const
		{
			return m_min_laplacian;
		}

		double GetMaxLaplacian() const
		{
			return m_max_laplacian;
		}

		std::string GetName()
		{
			return m_name;
		}

		void SetName(std::string name)
		{
			m_name = name;
		}

		bool IsOutOfBoundary(const UINT32& x, const UINT32& y, const UINT32& z) const
		{
			return !((x >= 0 && x < m_width)
				&& (y >= 0 && y < m_height)
				&& (z >= 0 && z < m_depth));
		}

    bool ResetScale()
    {
      if (m_scale == glm::vec3(1.0f))
        return false;
      
      m_scale = glm::vec3(1.0f);
      return true;
    }

    float GetXScale()
    {
      return m_scale.x;
    }

    void SetXScale(float s)
    {
      m_scale.x = s;
    }

    float GetYScale()
    {
      return m_scale.y;
    }

    void SetYScale(float s)
    {
      m_scale.y = s;
    }

    float GetZScale()
    {
      return m_scale.z;
    }

    void SetZScale(float s)
    {
      m_scale.z = s;
    }

		int GetScalarValue(const UINT32& id, const int& max_histo_value)
		{
			double value = GetValue(id);
			return ((value - m_min_value) / (m_max_value - m_min_value)) * max_histo_value;
		}

		int GetScalarValue(const UINT32& x, const UINT32& y, const UINT32& z, const int& max_histo_value)
		{
			return GetScalarValue(GetId(x, y, z), max_histo_value);
		}

		int GetScalarGradient(const double& value, const int& max_histo_value)
		{
      return (value / m_max_gradient) * max_histo_value;
		}

		int GetScalarLaplacian(const double& value, const int& max_histo_value)
		{
			return ((value - m_min_laplacian) / (m_max_laplacian - m_min_laplacian)) * max_histo_value;
		}

		double ConvertScalarToValue(const int& histo_value, const int& max_histo_value)
		{
			double p = histo_value / double(max_histo_value);
      double v = p * (m_max_value - m_min_value);
			return v + m_min_value;
		}

		double ConvertScalarToGradient(const int& histo_value, const int& max_histo_value)
		{
			double p = histo_value / double(max_histo_value);
			return m_max_gradient * p;
		}

		double ConvertScalarToLaplacian(const int& histo_value, const int& max_histo_value)
		{
			double p = histo_value / double(max_histo_value);
			double h = p * (m_max_laplacian - m_min_laplacian);
			return h + m_min_laplacian;
		}

		virtual bool IsActive(const UINT32& x, const UINT32& y, const UINT32& z)
		{
			return true;
		}

		virtual double GetValue(const UINT32& x, const UINT32& y, const UINT32& z) = 0;
		virtual double GetValue(const UINT32& id) = 0;

		virtual double CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z) = 0;
		virtual double CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z) = 0;
		virtual void CalculateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, double* g, double* l) = 0;

		virtual bool Validate() = 0;

    virtual bool IsTank()
    {
      return false;
    }

	protected:
		unsigned int m_width, m_height, m_depth;
		double m_max_value;
		double m_min_value;
		double m_max_gradient;
		double m_min_gradient;
		double m_min_laplacian;
		double m_max_laplacian;
		std::string m_name;
    DerivativeMask m_derivativeMask;
    glm::vec3* m_grad;
    glm::vec3 m_scale;
	};
}

#endif