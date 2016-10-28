#ifndef SCALAR_FIELD
#define SCALAR_FIELD

#include <basetsd.h>
#include <float.h>
#include <string>

namespace vr
{

	class ScalarField
	{
	public:
    ScalarField() : m_width(0), m_height(0), m_depth(0), 
      m_max_value(-FLT_MAX), m_min_value(FLT_MAX), m_max_gradient(-FLT_MAX), m_min_laplacian(FLT_MAX), m_max_laplacian(-FLT_MAX) {}
		ScalarField(const UINT32& width, const UINT32& height, const UINT32& depth)
			: m_width(width), m_height(height), m_depth(depth)
			, m_max_value(-FLT_MAX), m_min_value(FLT_MAX), m_max_gradient(-FLT_MAX), m_min_laplacian(FLT_MAX), m_max_laplacian(-FLT_MAX)
		{
		}

		int GetId(const UINT32& x, const UINT32& y, const UINT32& z) const
		{
			return x + (y * m_width) + (z * m_width * m_height);
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

    float GetMinValue() const
    {
      return m_min_value;
    }

    float GetMaxValue() const
    {
      return m_max_value;
    }

    float GetMaxGradient() const
    {
      return m_max_gradient;
    }

    float GetMinLaplacian() const
    {
      return m_min_laplacian;
    }

    float GetMaxLaplacian() const
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

		virtual float GetValue(const UINT32& x, const UINT32& y, const UINT32& z) = 0;
		virtual float GetValue(const UINT32& id) = 0;

		virtual float CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z) = 0;
		virtual float CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z) = 0;

    virtual bool Validate() = 0;

	protected:
		unsigned int m_width, m_height, m_depth;
		float m_max_value;
		float m_min_value;
		float m_max_gradient;
		float m_min_laplacian;
		float m_max_laplacian;
    std::string m_name;
	};
}

#endif