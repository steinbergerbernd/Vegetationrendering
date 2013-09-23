#pragma once

#include <D3D11.h>
#include <D3DX11.h>
#include <d3dx11effect.h>
#include "DXUT.h"
#include "SDKmisc.h"


#include "GameTime.h"
#include "Types.h"

class VegetationRendering;

class Terrain
{
public:
	struct TerrainVertex
	{
		Vector3 position;
		Vector3 normal;
		D3DXCOLOR color;
		Vector2 texCoord;
	};

	struct TerrainInstance
	{
		Vector3 position;
	};

	Terrain(const VegetationRendering& vegetationRendering);
	~Terrain(void);

	void init(ID3D11Device* device);

	void draw(ID3D11Device* device, const GameTime& gameTime);

	void release();

	unsigned getIndexCount() const { return indexCount; }
	ID3D11Buffer* getVertexBuffer() const { return vertexBuffer; }
	ID3D11Buffer* getIndexBuffer() const { return indexBuffer; }
private:
	ID3D11ShaderResourceView* surfaceTexture;

	ID3DX11EffectShaderResourceVariable* evSurfaceTexture;
	ID3DX11EffectMatrixVariable* evWorld;
	ID3DX11EffectMatrixVariable* evViewProjection;

	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* technique;
	ID3DX11EffectPass* pass;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* instanceBuffer;
	ID3D11Buffer* indexBuffer;
	unsigned indexCount;

	ID3D11InputLayout* inputLayout;

	Matrix world;

	const VegetationRendering& vegetationRendering;
};

