#pragma once

#include <D3D11.h>
#include <D3DX11.h>
#include <d3dx11effect.h>

#include "TerrainController.h"
#include "TerrainTessellator.h"
#include "TerrainBlock.h"
#include "BoundingBox.h"

class VegetationRendering;

class TerrainView
{
	typedef std::vector<unsigned> UnsignedCollection;
	typedef std::vector<std::vector<unsigned>> BlockIndexCollection;
	typedef std::vector<std::vector<TerrainInstance>> TerrainInstanceCollection;
	typedef std::vector<TerrainBlock> TerrainBlockCollection;
	typedef std::pair<unsigned, unsigned> UnsignedPair;
	typedef std::vector<UnsignedPair> UnsignedPairCollection;

public:
	TerrainView(const VegetationRendering& vegetationRendering, TerrainController& terrainController);

	void init(ID3D11Device* device);
	void release();
	void update(const GameTime& gameTime);
	void draw(ID3D11Device* device, const GameTime& gameTime);

	unsigned getPatchSize() const { return patchSize; }
	unsigned getDetailLevels() const { return detailLevels; }

	void onResetDevice(ID3D11Device* device);

	const TerrainController& getTerrainController() const { return terrainController; }

	const TerrainTessellator& getTessellator() const { return tessellator; }

private:
	void update(unsigned row, unsigned col, unsigned numRows, unsigned numCols, bool frustumCulling = true);

	unsigned getBlockIndex(const TerrainPoint& p) const;
	unsigned getBlockIndex(unsigned row, unsigned col) const;
	BoundingBox getBoundingBox(unsigned row, unsigned col, unsigned numRows, unsigned numCols) const;
	UnsignedPairCollection getDrawOrder(unsigned rowMin, unsigned colMin, unsigned rowMax, unsigned colMax) const;

	unsigned width;
	unsigned length;

	unsigned patchSize;
	unsigned detailLevels;

	const VegetationRendering& vegetationRendering;
	TerrainController& terrainController;

	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* technique;
	ID3DX11EffectPass* pass;
	ID3D11InputLayout* inputLayout;

	ID3D11ShaderResourceView* heightmap;
	ID3D11ShaderResourceView* normalmap;

	TerrainTessellator tessellator;

	TerrainBlockCollection blocks;
	UnsignedCollection numVisibleBlocks;
	BlockIndexCollection visibleBlocks;
	TerrainInstanceCollection instances;
	UnsignedCollection numInstances;
};