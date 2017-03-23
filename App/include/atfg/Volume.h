#ifndef VOLREND_VOLUME_H
#define VOLREND_VOLUME_H

#include <iostream>
#include <stack>
#include <math/Vector3.h>
#include <math/Vector4.h>

#include <glutils/GLTexture3D.h>
#include "ScalarField.h"
#include "TransferFunction1D.h"

class ATFGenerator;

namespace vr
{
	enum VolumeType
	{
		STRUCTURED,
		UNSTRUCTURED,
	};

	//template<class T>
	class Volume : public ScalarField
	{
	public:
		Volume();
		Volume(const UINT32& width, const UINT32& height, const UINT32& depth);
		Volume(const UINT32& width, const UINT32& height, const UINT32& depth, double* scalars);
		Volume(const UINT32& width, const UINT32& height, const UINT32& depth, unsigned char* scalars);
		~Volume();
		
		lqc::Vector3f GetAnchorMin();
		lqc::Vector3f GetAnchorMax();
		void SetAnchors(lqc::Vector3f pmin, lqc::Vector3f pmax);

		double GetValue(const UINT32& x, const UINT32& y, const UINT32& z);
		double GetValue(const UINT32& id)
		{
			return m_scalar_values[id];
		}

		double CalculateGradient(const UINT32& x, const UINT32& y, const UINT32& z);
		double CalculateLaplacian(const UINT32& x, const UINT32& y, const UINT32& z);
		void CalculateDerivatives(const UINT32& x, const UINT32& y, const UINT32& z, double* g, double* l);

		double InterpolatedValue(double x, double y, double z);
		double InterpolatedValue(lqc::Vector3f pos);

		double TrilinearScalarFunction(lqc::Vector3f pos, lqc::Vector3f rayeye, lqc::Vector3f raydirection);

		bool IsOutOfBoundary(int x, int y, int z);

		bool Validate()
		{
			return m_scalar_values != NULL;
		}

		double* GetValues()
		{
			return m_scalar_values;
		}

	private:
		struct Voxel
		{
			int x, y, z;
		};

		int GetNeighborhod(int x, int y, int z, Voxel* neigh);

	private:
		std::string m_name;

		lqc::Vector3f m_pmin;
		lqc::Vector3f m_pmax;

		double* m_scalar_values;
	};
}

#endif