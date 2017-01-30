/// IATFGenerator.h
/// Rustam Mesquita
/// rustam@tecgraf.puc-rio.br

#ifndef IATFGenerator_H
#define IATFGenerator_H

#include <volrend\ScalarField.h>
#include <volrend\TransferFunction.h>

/// <summary>
/// This interface encapsulates the Transfer Function
/// Generator baic concept, which is to construct a
/// transfer funciton given a scalarfield as input.
/// </summary>
class IATFGenerator
{
public:
	/// <summary>
	/// Initializes a new instance of the <see cref="IATFGenerator"/> class.
	/// </summary>
	/// <param name="scalarfield">The scalarfield whose tranfer function will be constructed.</param>
	IATFGenerator(vr::ScalarField* scalarfield)
	{
		if (!scalarfield)
			throw std::exception_ptr();

		m_scalarfield = scalarfield;
		m_width = scalarfield->GetWidth();
		m_height = scalarfield->GetHeight();
		m_depth = scalarfield->GetDepth();
	}

	/// <summary>
	/// Gets an instance of the Transfer Function generated
	/// automatically.
	/// </summary>
	/// <returns>Returns a Transfer Function.</returns>
	virtual vr::TransferFunction* GetTransferFunction() = 0;

	vr::ScalarField* GetScalarField()
	{
		return m_scalarfield;
	}

protected:
	/// <summary>
	/// The width of the scalarfield.
	/// </summary>
	unsigned int m_width;

	/// <summary>
	/// The height of the scalarfield.
	/// </summary>
	unsigned int m_height;

	/// <summary>
	/// The depth of the scalarfield.
	/// </summary>
	unsigned int m_depth;

	/// <summary>
	/// A reference to the scalarfield.
	/// </summary>
	vr::ScalarField* m_scalarfield;
};

#endif