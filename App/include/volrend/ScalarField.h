#ifndef SCALAR_FIELD
#define SCALAR_FIELD

#include <basetsd.h>
#include <float.h>
#include <string>
#include <math/MUtils.h>

namespace vr
{

	class ScalarField
	{
	public:
		ScalarField() : m_width(0), m_height(0), m_depth(0), 
			m_max_value(-FLT_MAX), m_min_value(FLT_MAX), m_max_gradient(-FLT_MAX), m_min_laplacian(FLT_MAX), m_max_laplacian(-FLT_MAX)
		{
			printf("ScalarField criado.\n");
		}

		ScalarField(const UINT32& width, const UINT32& height, const UINT32& depth)
			: m_width(width), m_height(height), m_depth(depth)
			, m_max_value(-FLT_MAX), m_min_value(FLT_MAX), m_max_gradient(-FLT_MAX), m_min_laplacian(FLT_MAX), m_max_laplacian(-FLT_MAX)
		{
			printf("ScalarField criado.\n");
		}

		virtual ~ScalarField() { printf("ScalarField destruido.\n"); }

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

		int GetScalarValue(const UINT32& id, const int& max_histo_value)
		{
			float value = GetValue(id);
			return ((value - m_min_value) / (m_max_value - m_min_value)) * max_histo_value;
		}

		int GetScalarValue(const UINT32& x, const UINT32& y, const UINT32& z, const int& max_histo_value)
		{
			return GetScalarValue(GetId(x, y, z), max_histo_value);
		}

		int GetScalarGradient(const float& value, const int& max_histo_value)
		{
			return (value / m_max_gradient) * max_histo_value;
		}

		int GetScalarLaplacian(const float& value, const int& max_histo_value)
		{
			return ((value - m_min_laplacian) / (m_max_laplacian - m_min_laplacian)) * max_histo_value;
		}

		float ConvertScalarToValue(const int& histo_value, const int& max_histo_value)
		{
			float p = histo_value / float(max_histo_value);
			return m_max_value* p;
		}

		float ConvertScalarToGradient(const int& histo_value, const int& max_histo_value)
		{
			float p = histo_value / float(max_histo_value);
			return m_max_gradient * p;
		}

		float ConvertScalarToLaplacian(const int& histo_value, const int& max_histo_value)
		{
			float p = histo_value / float(max_histo_value);
			float h = p * (m_max_laplacian - m_min_laplacian);
			return h + m_min_laplacian;
		}

		virtual bool IsActive(const UINT32& x, const UINT32& y, const UINT32& z)
		{
			return true;
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