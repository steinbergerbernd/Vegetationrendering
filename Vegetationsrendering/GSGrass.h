#pragma once

#include <D3D11.h>
#include <D3DX11.h>
#include <d3dx11effect.h>

#include "GameTime.h"

class VegetationRendering;

class GSGrass
{
public:
	GSGrass(const VegetationRendering& vegetationRendering);
	~GSGrass(void);

	void init(ID3D11Device* device);
	void draw(ID3D11Device* device, const GameTime& gameTime);

	void release();
private:
	const VegetationRendering& vegetationRendering;

	ID3D11RasterizerState* rasterizerState;

	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* technique;
	ID3DX11EffectPass* pass;

	ID3D11InputLayout* inputLayout;

	//Shader variables
	ID3DX11EffectMatrixVariable* evWorld;
	ID3DX11EffectMatrixVariable* evViewProjection;
	ID3DX11EffectShaderResourceVariable* evGrassTextureArray;
	ID3DX11EffectShaderResourceVariable* evRandomTexture;
	ID3DX11EffectScalarVariable* evGrassCoverage;
	ID3DX11EffectScalarVariable* evGrassWidth;
	ID3DX11EffectScalarVariable* evGrassHeight;
	ID3DX11EffectScalarVariable* evGrassMessiness;

	ID3D11Texture2D* grassTextures;
	ID3D11ShaderResourceView* grassTexturesSRV;

	ID3D11Texture1D* randomTexture;
	ID3D11ShaderResourceView* randomTextureSRV;

	void loadTextureArray(ID3D11Device* device);
	void createRandomTexture(ID3D11Device* device);
};

