#pragma once

#include <D3D11.h>
#include <D3DX11.h>
#include <d3dx11effect.h>

#include "GameTime.h"
#include "Vector3.h"

class VegetationRendering;

class Triangle
{
	struct TriangleVertex
	{
		Vector3 position;
		D3DXCOLOR Color;
	};

public:
	Triangle(const VegetationRendering& vegetationRendering);
	~Triangle(void);

	void init(ID3D11Device* device);

	void draw(ID3D11Device* device, const GameTime& gameTime);

	void release();
private:
	//ID3D11VertexShader *vertexShader;
	//ID3D11PixelShader *pixelShader;
	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* technique;
	ID3DX11EffectPass* pass;

	ID3D11Buffer *vertexBuffer;

	ID3D11InputLayout *inputLayout;

	const VegetationRendering& vegetationRendering;
};

