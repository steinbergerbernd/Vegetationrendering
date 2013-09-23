#pragma once

#include <D3D11.h>
#include <vector>

#include "Types.h"
#include "MathHelper.h"
#include "TerrainInstance.h"
#include "TerrainGeometry.h"

class TerrainTessellator
{
	struct TerrainVertex
	{
		Vector3 position;
	};

	typedef unsigned TerrainIndex;
	typedef std::vector<ID3D11Buffer*> IndexBufferCollection;
	typedef std::vector<TerrainVertex> VertexCollection;
	typedef std::vector<TerrainIndex> IndexCollection;

public:
	TerrainTessellator();

	void init(ID3D11Device* device, unsigned size, unsigned numPatches, unsigned stripeSize);

	void drawPatches(ID3D11Device* device, unsigned levelOfDetail, const std::vector<TerrainInstance>& instances, unsigned numInstances) const;
	void drawSkirts(ID3D11Device* device, unsigned levelOfDetail, const std::vector<TerrainInstance>& instances, unsigned numInstances) const;

	void drawPatch(ID3D11Device* device, unsigned levelOfDetail = 0) const;
	void drawSkirt(ID3D11Device* device, unsigned levelOfDetail = 0) const;

	void onResetDevice(ID3D11Device* device);
	void onLostDevice();

private:
	void draw(ID3D11Device* device, unsigned levelOfDetail, unsigned primitiveCount, const IndexBufferCollection& indexBuffers, const std::vector<TerrainInstance>& instances, unsigned numInstances) const;
	void draw(ID3D11Device* device, unsigned levelOfDetail, unsigned primitiveCount, const IndexBufferCollection& indexBuffers) const;

	void createPatchIndexBuffer(ID3D11Device* device, unsigned levelOfDetail);
	void createSkirtIndexBuffer(ID3D11Device* device, unsigned levelOfDetail);

	void initInstanceBuffer(ID3D11Device* device);
	void initQuadIndices(int row, int col, int deltaRow, int deltaCol, IndexCollection* indices, int* count);

	TerrainIndex getIndex(int x, int z);

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* instanceVertexBuffer;

	unsigned size;
	unsigned stripeSize;
	unsigned numVertices;
	unsigned numPatches;

	IndexBufferCollection patchIndexBuffers;
	IndexBufferCollection skirtIndexBuffers;

	static const D3DFORMAT indexFormat;
};