#ifndef SCALAR_FIELD
#define SCALAR_FIELD

#include <basetsd.h>

namespace vr
{

	class ScalarField
	{
	public:
		ScalarField() : m_width(0), m_height(0), m_depth(0) {}
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

		virtual float GetValue(const UINT32& x, const UINT32& y, const UINT32& z) = 0;
		virtual float GetValue(const UINT32& id) = 0;

		virtual float CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z) = 0;
		virtual float CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z) = 0;

	protected:
		unsigned int m_width, m_height, m_depth;
		float m_max_value;
		float m_min_value;
		float m_max_gradient;
		float m_min_laplacian;
		float m_max_laplacian;
	};
}

#endif