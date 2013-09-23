#pragma once

#include <D3D11.h>
#include <d3dx11effect.h>
#include "DXUT11\Core\DXUT.h"
#include "DXUT11\Optional\SDKmesh.h"
#include "DXUT11\Optional\SDKmisc.h"
#include "Types.h"
#include <string>

class VegetationRendering;

class DefaultModel
{
public:
	DefaultModel(const VegetationRendering& vegetationRendering, const wchar_t* modelPath);
	~DefaultModel(void);

	void init(ID3D11Device* device);
	void release();

	void draw(ID3D11Device* device, const GameTime& gameTime);

	void setWorld(Matrix world);
private:
	const VegetationRendering& vegetationRendering;

	const wchar_t* modelPath;

	CDXUTSDKMesh model;

	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* technique;
	ID3DX11EffectPass* pass;
	ID3D11InputLayout* inputLayout;

	ID3DX11EffectMatrixVariable* evWorld;
	ID3DX11EffectMatrixVariable* evViewProjection;
	ID3DX11EffectShaderResourceVariable* evModelTexture;

	ID3DX11EffectVectorVariable* evCameraPosition;
	ID3DX11EffectVectorVariable* evLightVector;
	ID3DX11EffectVectorVariable* evAmbientLight;
	ID3DX11EffectVectorVariable* evDiffuseLight;
	ID3DX11EffectVectorVariable* evSpecularLight;
	ID3DX11EffectScalarVariable* evShininess;

	Matrix world;
};

