#include "TerrainTessellator.h"


const D3DFORMAT TerrainTessellator::indexFormat = D3DFMT_INDEX32;


TerrainTessellator::TerrainTessellator() : instanceVertexBuffer(0)
{
}


void TerrainTessellator::init(ID3D11Device* device, unsigned size, unsigned numPatches, unsigned stripeSize)
{
	this->size = size;
	this->stripeSize = stripeSize;
	this->numPatches = numPatches;

	numVertices = (size + 3) * (size + 3);

	unsigned lod = (unsigned)MathHelper::log2(size) + 1;

	patchIndexBuffers = IndexBufferCollection(lod);
	skirtIndexBuffers = IndexBufferCollection(lod);

	for (unsigned i = 0; i < lod; ++i)
	{
		createPatchIndexBuffer(device, i);
		createSkirtIndexBuffer(device, i);
	}

	/*for (unsigned i = 0; i < lod; ++i)
	{
		patchIndexBuffers[i]->Release();
		skirtIndexBuffers[i]->Release();
	}*/

	VertexCollection vertices(numVertices);

	for (int row = -1; row <= (int)size + 1; ++row)
		for (int col = -1; col <= (int)size + 1; ++col)
		{
			int r = MathHelper::clamp(row, 0, (int)size);
			int c = MathHelper::clamp(col, 0, (int)size);

			float y = (r == row && c == col) ? 0.0f : -1.0f;

			vertices[getIndex(row, col)].position = Vector3((float)c, y, (float)r);
		}

	D3D11_BUFFER_DESC vBufferDesc;
	ZeroMemory(&vBufferDesc, sizeof(vBufferDesc));

	vBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vBufferDesc.ByteWidth = sizeof(TerrainVertex) * numVertices;
	vBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &vertices[0];
	device->CreateBuffer(&vBufferDesc, &data, &vertexBuffer);

	/*if(vertexBuffer)
		vertexBuffer->Release();*/

	initInstanceBuffer(device);
}


void TerrainTessellator::drawPatches(ID3D11Device* device, unsigned levelOfDetail, const std::vector<TerrainInstance>& instances, unsigned numInstances) const
{
	int lodSize = size / MathHelper::pow2(levelOfDetail);

	draw(device, levelOfDetail, lodSize * lodSize * 2, patchIndexBuffers, instances, numInstances);
}


void TerrainTessellator::drawSkirts(ID3D11Device* device, unsigned levelOfDetail, const std::vector<TerrainInstance>& instances, unsigned numInstances) const
{
	int lodSize = size / MathHelper::pow2(levelOfDetail);

	draw(device, levelOfDetail, lodSize * 8, skirtIndexBuffers, instances, numInstances);
}


void TerrainTessellator::drawPatch(ID3D11Device* device, unsigned levelOfDetail) const
{
	int lodSize = size / MathHelper::pow2(levelOfDetail);

	draw(device, levelOfDetail, lodSize * lodSize * 2, patchIndexBuffers);
}


void TerrainTessellator::drawSkirt(ID3D11Device* device, unsigned levelOfDetail) const
{
	int lodSize = size / MathHelper::pow2(levelOfDetail);

	draw(device, levelOfDetail, lodSize * 8, skirtIndexBuffers);
}


void TerrainTessellator::draw(ID3D11Device* device, unsigned levelOfDetail, unsigned primitiveCount, const IndexBufferCollection& indexBuffers, const std::vector<TerrainInstance>& instances, unsigned numInstances) const
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	D3D11_MAPPED_SUBRESOURCE terrainInstanceData;
	deviceContext->Map(instanceVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &terrainInstanceData);
	memcpy(terrainInstanceData.pData, (void*)&instances[0], instances.size() * sizeof(TerrainInstance));
	deviceContext->Unmap(instanceVertexBuffer, 0);

	ID3D11Buffer* bufferPointers[2];
	bufferPointers[0] = vertexBuffer;
	bufferPointers[1] = instanceVertexBuffer;

	unsigned int strides[2];
	strides[0] = sizeof(TerrainVertex);
	strides[1] = sizeof(TerrainInstance);

	unsigned int offsets[2];
	offsets[0] = 0, offsets[1] = 0;
	
	deviceContext->IASetVertexBuffers(0, 1, bufferPointers, strides, offsets);
	deviceContext->IASetIndexBuffer(indexBuffers[levelOfDetail], DXGI_FORMAT_R32_UINT, 0);

	deviceContext->DrawIndexed(primitiveCount+1, 0, 0);
	//deviceContext->DrawIndexedInstanced(primitiveCount * 3, numInstances, 0, 0, 0);

	deviceContext->Release();
}

	
void TerrainTessellator::draw(ID3D11Device* device, unsigned levelOfDetail, unsigned primitiveCount, const IndexBufferCollection& indexBuffers) const
{
	/*device.setStreamSource(vertexBuffer, sizeof(TerrainVertex));
	device.setIndices(indexBuffers[levelOfDetail]);

	device.drawIndexedPrimitive(D3DPT_TRIANGLELIST, primitiveCount, numVertices);*/
}


TerrainTessellator::TerrainIndex TerrainTessellator::getIndex(int row, int col)
{
	return (row + 1) * (size + 3) + (col + 1);
}


void TerrainTessellator::createSkirtIndexBuffer(ID3D11Device* device, unsigned levelOfDetail)
{
	// distance between two neighbor vertices
	int delta = MathHelper::pow2(levelOfDetail);

	// size of grid in current lod (e.g. lod 0: 64, lod 1: 32)
	int lodSize = size / delta;

	// main indices + skirt indices
	int numIndices = lodSize * 4 * 6;

	IndexCollection indices(numIndices);

	int count = 0;

	int interval = min(stripeSize * delta, size);

	// init skirt indices
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (unsigned row = 0; row < (i ? size : 1); row += delta)
				for (unsigned col = 0; col < (i ? 1 : size); col += delta)
				{
					int r = i ? row : -1 + j * (size + 1);
					int c = i ? -1 + j * (size + 1) : col;

					int deltaCol = i ? 1 : delta;
					int deltaRow = i ? delta : 1;

					initQuadIndices(r, c, deltaRow, deltaCol, &indices, &count);
				}

	D3D11_BUFFER_DESC iBufferDesc;
	ZeroMemory(&iBufferDesc, sizeof(iBufferDesc));

	iBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	iBufferDesc.ByteWidth = sizeof(TerrainIndex) * numIndices;
	iBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &indices[0];
	device->CreateBuffer(&iBufferDesc, &data, &skirtIndexBuffers[levelOfDetail]);
}


void TerrainTessellator::createPatchIndexBuffer(ID3D11Device* device, unsigned levelOfDetail)
{
	// distance between two neighbor vertices
	int delta = MathHelper::pow2(levelOfDetail);

	// size of grid in current lod (e.g. lod 0: 64, lod 1: 32)
	int lodSize = size / delta;

	// main indices + skirt indices
	int numIndices = lodSize * lodSize * 6;

	IndexCollection indices(numIndices);

	int count = 0;

	unsigned interval = min(stripeSize * delta, size);

	// init main indices
	for (unsigned i = 0; i < size / interval; ++i)
		for (unsigned row = 0; row < size; row += delta)
			for (unsigned col = i * interval; col < (i+1) * interval; col += delta)
				initQuadIndices(row, col, delta, delta, &indices, &count);

	D3D11_BUFFER_DESC iBufferDesc;
	ZeroMemory(&iBufferDesc, sizeof(iBufferDesc));

	iBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	iBufferDesc.ByteWidth = sizeof(TerrainIndex) * numIndices;
	iBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &indices[0];
	device->CreateBuffer(&iBufferDesc, &data, &patchIndexBuffers[levelOfDetail]);
}


void TerrainTessellator::initQuadIndices(int row, int col, int deltaRow, int deltaCol, IndexCollection* indices, int* count)
{
	int& c = *count;
	IndexCollection& ind = *indices;

	ind[c++] = getIndex(row, col);
	ind[c++] = getIndex(row + deltaRow, col);
	ind[c++] = getIndex(row + deltaRow, col + deltaCol);

	ind[c++] = getIndex(row + deltaRow, col + deltaCol);
	ind[c++] = getIndex(row, col + deltaCol);
	ind[c++] = getIndex(row, col);
}


void TerrainTessellator::initInstanceBuffer(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vBufferDesc;
	ZeroMemory(&vBufferDesc, sizeof(vBufferDesc));

	vBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vBufferDesc.ByteWidth = sizeof(TerrainInstance) * numPatches;
	vBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	device->CreateBuffer(&vBufferDesc, 0, &instanceVertexBuffer);
}


void TerrainTessellator::onResetDevice(ID3D11Device* device)
{
	//initInstanceBuffer(device);
}


void TerrainTessellator::onLostDevice()
{
	if(instanceVertexBuffer)
		instanceVertexBuffer->Release();

	unsigned lod = (unsigned)MathHelper::log2(size) + 1;
	for (unsigned i = 0; i < lod; ++i)
	{
		patchIndexBuffers[i]->Release();
		skirtIndexBuffers[i]->Release();
	}

	if(vertexBuffer)
		vertexBuffer->Release();
}