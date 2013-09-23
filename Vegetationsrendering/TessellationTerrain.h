#pragma once

#include <D3D11.h>
#include <D3DX11.h>
#include <d3dx11effect.h>
#include "DXUT11\Core\DXUT.h"

#include "GameTime.h"
#include "Vector3.h"

class VegetationRendering;

class TessellationTerrain
{
public:
	struct TerrainVertex
	{
		Vector3 position;
		Vector2 texCoord;
	};

	TessellationTerrain(const VegetationRendering& vegetationRendering);
	~TessellationTerrain(void);

	void init(ID3D11Device* device);

	void draw(ID3D11Device* device, const GameTime& gameTime);

	void release();

	unsigned getIndexCount() const { return indexCount; }
	ID3D11Buffer* getVertexBuffer() const { return vertexBuffer; }
	ID3D11Buffer* getIndexBuffer() const { return indexBuffer; }
private:
	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* technique;
	ID3DX11EffectPass* pass;

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	unsigned indexCount;

	ID3D11InputLayout* inputLayout;

	const VegetationRendering& vegetationRendering;
};

