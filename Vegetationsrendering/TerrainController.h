#pragma once

#include <D3D11.h>
#include <D3DX11.h>

#include "TerrainPoint.h"
#include "Matrix.h"
#include <vector>
#include <algorithm>

class VegetationRendering;

class TerrainController
{
	typedef unsigned short HeightmapFormat;
	typedef unsigned NormalmapFormat;

	typedef std::vector<float> HeightData;
	typedef std::vector<Vector3> NormalData;

public:
	TerrainController(const VegetationRendering& vegetationRendering);

	void init(ID3D11Device* device);

	float getBumpiness() const { return bumpiness; }
	float getQuality() const { return quality; }

	unsigned getWidth() const { return width; }
	unsigned getLength() const { return length; }

	Vector3 getNormal(const Vector3& position) const;
	Vector3 getNormal(float x, float z) const;
	Vector3 getNormal(const TerrainPoint& p) const;

	float getHeight(float x, float z) const;
	float getHeight(const TerrainPoint& p) const;
	Vector3 getHeight(const Vector3& position) const;

	const Vector3& getPosition() const { return position; }

	const Matrix& getWorld() const { return world; }

private:
	static float convertHeight(HeightmapFormat height);
	static Vector3 convertNormal(NormalmapFormat normal);

	template <class T> T get(float x, float z, T (TerrainController::*getFunction)(const TerrainPoint& p) const) const;

	float getHeightmapData(unsigned x, unsigned y) const;
	Vector3 getNormalmapData(unsigned x, unsigned y) const;

	const VegetationRendering& vegetationRendering;

	unsigned width;
	unsigned length;

	float bumpiness;
	float quality;

	Vector3 position;

	Matrix world;

	D3D11_TEXTURE2D_DESC heightmapDesc;
	D3D11_TEXTURE2D_DESC normalmapDesc;

	HeightData heightData;
	NormalData normalData;
};


template <class T>
T TerrainController::get(float x, float z, T (TerrainController::*getFunction)(const TerrainPoint& p) const) const
{
	// bug fix
	x -= 1.0f;

	x = MathHelper::clamp(x - position.x, 0.0f, (float)width);
	z = MathHelper::clamp(z - position.z, 0.0f, (float)length);

	// quad indices
	unsigned rLow = (unsigned)std::floor(z);
	unsigned rHigh = (unsigned)std::ceil(z);
	unsigned cLow = (unsigned)std::floor(x);
	unsigned cHigh = (unsigned)std::ceil(x);

	// quad heights/normals
	T h00 = (this->*getFunction)(TerrainPoint(rLow, cLow));
	T h01 = (this->*getFunction)(TerrainPoint(rLow, cHigh));
	T h10 = (this->*getFunction)(TerrainPoint(rHigh, cLow));
	T h11 = (this->*getFunction)(TerrainPoint(rHigh, cHigh));

	// coordinates inside quad
	float r = z - rLow;
	float c = x - cLow;

	// bilinear interpolation
	T h0 = h00 * (1.0f - c) + h01 * c;
	T h1 = h10 * (1.0f - c) + h11 * c;

	return h0 * (1.0f - r) + h1 * r;
}