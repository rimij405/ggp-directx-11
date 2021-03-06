#pragma once

// -----------------------------------
// Include statements.
// -----------------------------------

#include "SimpleShader.h"

class Material
{
public:

	// -----------------------------------
	// Friend methods.
	// -----------------------------------

	friend void swap(Material& lhs, Material& rhs);

	// -----------------------------------
	// Constructor(s).
	// -----------------------------------

	Material();
	Material(SimpleVertexShader& _vShd, SimplePixelShader& _pShd);
	~Material();
	Material(const Material& other);
	Material(Material&& other) noexcept;
	Material& operator=(Material other);

	// -----------------------------------
	// Accessors.
	// -----------------------------------

	SimpleVertexShader* GetVertexShader() const;
	SimplePixelShader* GetPixelShader() const;

	// -----------------------------------
	// Mutators.
	// -----------------------------------

	void SetVertexShader(SimpleVertexShader& _vShd);
	void SetPixelShader(SimplePixelShader& _pShd);

private:

	// -----------------------------------
	// Data members.
	// -----------------------------------

	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;

};