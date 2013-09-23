#include "TerrainView.h"
#include "MathHelper.h"
#include "Config.h"
#include "ContainmentType.h"
#include "Camera.h"
#include "VegetationRendering.h"
#include "EffectCamera.h"

#undef min
#undef max

TerrainView::TerrainView(const VegetationRendering& vegetationRendering, TerrainController& terrainController)
	: terrainController(terrainController), vegetationRendering(vegetationRendering)
{
}


void TerrainView::init(ID3D11Device* device)
{
	width = terrainController.getWidth();
	length = terrainController.getLength();

	//patchSize = Config::getValue<unsigned>(LLConfigKeys::terrainPatchSize);
	patchSize = 32;

	detailLevels = (unsigned)MathHelper::log2(patchSize) + 1U;

	unsigned numBlocks = (width / patchSize) * (length / patchSize);

	visibleBlocks = BlockIndexCollection(detailLevels);
	instances = TerrainInstanceCollection(detailLevels);
	numVisibleBlocks = UnsignedCollection(detailLevels);
	numInstances = UnsignedCollection(detailLevels);

	for (unsigned i = 0; i < detailLevels; ++i)
	{
		visibleBlocks[i].resize(numBlocks);
		instances[i].resize(numBlocks);
	}

	unsigned patchStripeSize = 4;

	tessellator.init(device, patchSize, numBlocks, patchStripeSize);

	blocks = TerrainBlockCollection(numBlocks);

	for (unsigned row = 0; row < length / patchSize; ++row)
		for (unsigned col = 0; col < width / patchSize; ++col)
			blocks[getBlockIndex(row, col)].init(*this, TerrainPoint(row * patchSize, col * patchSize));

	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	HRESULT result = D3DX11CompileFromFileA(Config::getValue(ConfigKeys::terrainEffectPath).c_str(), 0, 0, 0, "fx_5_0", 0, 0, 0, &shaderBlob, &errorBlob, 0);

	char* errorBuffer;
	if(errorBlob)
	{
		errorBuffer = (char*)errorBlob->GetBufferPointer();
	}

	result = D3DX11CreateEffectFromMemory(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), 0, device, &effect);
	
	if(shaderBlob)
		shaderBlob->Release();
	if(errorBlob)
		errorBlob->Release();

	D3DX11_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory( &loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO) );
	loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	loadInfo.Format = DXGI_FORMAT_FROM_FILE;

	ID3D11ShaderResourceView *sandTexture = NULL;
	D3DX11CreateShaderResourceViewFromFile( device, L"Content/Textures/Terrain/Sand.png", &loadInfo, NULL, &sandTexture, NULL );
	ID3D11ShaderResourceView *grassTexture = NULL;
	D3DX11CreateShaderResourceViewFromFile( device, L"Content/Textures/Terrain/Grass.png", &loadInfo, NULL, &grassTexture, NULL );
	ID3D11ShaderResourceView *stoneTexture = NULL;
	D3DX11CreateShaderResourceViewFromFile( device, L"Content/Textures/Terrain/Stone.png", &loadInfo, NULL, &stoneTexture, NULL );
	ID3D11ShaderResourceView *rockTexture = NULL;
	D3DX11CreateShaderResourceViewFromFile( device, L"Content/Textures/Terrain/Rock.png", &loadInfo, NULL, &rockTexture, NULL );

	effect->GetVariableByName("Texture0")->AsShaderResource()->SetResource(sandTexture);
	effect->GetVariableByName("Texture1")->AsShaderResource()->SetResource(grassTexture);
	effect->GetVariableByName("Texture2")->AsShaderResource()->SetResource(stoneTexture);
	effect->GetVariableByName("TextureCliff")->AsShaderResource()->SetResource(rockTexture);

	sandTexture->Release();
	grassTexture->Release();
	stoneTexture->Release();
	rockTexture->Release();

	ID3D11ShaderResourceView *heightmapTexture = NULL;
	D3DX11CreateShaderResourceViewFromFile( device, L"Content/Textures/Terrain/Heightmap.png", &loadInfo, NULL, &heightmapTexture, NULL );
	ID3D11ShaderResourceView *normalmapTexture = NULL;
	D3DX11CreateShaderResourceViewFromFile( device, L"Content/Textures/Terrain/Normalmap.png", &loadInfo, NULL, &normalmapTexture, NULL );

	effect->GetVariableByName("Heightmap")->AsShaderResource()->SetResource(heightmapTexture);
	effect->GetVariableByName("Normalmap")->AsShaderResource()->SetResource(normalmapTexture);

	heightmapTexture->Release();
	normalmapTexture->Release();

	effect->GetVariableByName("TerrainWidth")->AsScalar()->SetInt(width + 1);
	effect->GetVariableByName("TerrainLength")->AsScalar()->SetInt(length + 1);
	
	effect->GetVariableByName("Bumpiness")->AsScalar()->SetFloat(terrainController.getBumpiness());
	effect->GetVariableByName("MinLevelOfDetail")->AsScalar()->SetInt(detailLevels - 1);
	
	D3DMATERIAL9 material;
	material.Ambient = Vector4::one;
	material.Diffuse = Vector4::one;
	material.Specular = Vector3(0.5f);
	material.Power = 20.0f;
	material.Emissive = Vector4::zero;
	effect->GetVariableByName("Material")->SetRawValue(&material, 0, 1);

	effect->GetVariableByName("TextureRange")->AsVector()->SetFloatVector(Vector4(0.03f, 0.0f, 0.2f, 0.07f));
	effect->GetVariableByName("CliffRange")->AsVector()->SetFloatVector(Vector2(0.65f, 0.85f));
	effect->GetVariableByName("TextureResolution")->AsVector()->SetFloatVector(Vector2(32.0f, 32.0f));

	technique = effect->GetTechniqueByIndex(0);
	pass = technique->GetPassByIndex(0);

	D3DX11_PASS_SHADER_DESC vertexShaderDesc;
	D3DX11_EFFECT_SHADER_DESC effectShaderDesc;
	
	result = pass->GetVertexShaderDesc(&vertexShaderDesc);
	vertexShaderDesc.pShaderVariable->GetShaderDesc(vertexShaderDesc.ShaderIndex, &effectShaderDesc);

	D3D11_INPUT_ELEMENT_DESC vertexElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_INSTANCE_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, 20, D3D11_INPUT_PER_INSTANCE_DATA, 0}
	};

	device->CreateInputLayout(vertexElementDesc, 1, effectShaderDesc.pBytecode, effectShaderDesc.BytecodeLength, &inputLayout);
}


void TerrainView::update(const GameTime& gameTime)
{
	for (unsigned i = 0; i < detailLevels; ++i)
		numVisibleBlocks[i] = 0;

	update(0, 0, length / patchSize, width / patchSize, false);
}


void TerrainView::update(unsigned row, unsigned col, unsigned numRows, unsigned numCols, bool frustumCulling)
{
	Camera& camera = vegetationRendering.getCamera();

	if (frustumCulling)
	{		
		switch (camera.getViewFrustum().contains(getBoundingBox(row, col, numRows, numCols)))
		{
			case ContainmentType::disjoint: return;
			case ContainmentType::contains: frustumCulling = false;
		}
	}

	if (numRows > 1 && numCols > 1)
	{
		for (unsigned r = 0; r < 2; ++r)
			for (unsigned c = 0; c < 2; ++c)
				update(row + r * numRows / 2, col + c * numCols / 2, numRows / 2, numCols / 2, frustumCulling);

		return;
	}

	for (unsigned r = 0; r < numRows; ++r)
		for (unsigned c = 0; c < numCols; ++c)
		{
			unsigned blockIndex = getBlockIndex(row + r, col + c);

			TerrainBlock& block = blocks[blockIndex];

			block.update(*this, camera.getPosition());

			unsigned lod = block.getLevelOfDetail();

			visibleBlocks[lod][numVisibleBlocks[lod]++] = blockIndex;
		}
}


void TerrainView::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	for (unsigned lod = 0; lod < detailLevels; ++lod)
	{
		numInstances[lod] = 0;

		for (unsigned i = 0; i < numVisibleBlocks[lod]; ++i)
		{
			const TerrainBlock& block = blocks[visibleBlocks[lod][i]];

			TerrainInstance& instance = instances[lod][numInstances[lod]++];

			instance.blending = block.getBlending();
			instance.offset = block.getOffset();
		}
	}

	Camera& camera = vegetationRendering.getCamera();
	effect->GetVariableByName("Camera")->SetRawValue(&EffectCamera(camera), 0, 1);
	effect->GetVariableByName("Light")->SetRawValue(&vegetationRendering.getLight(), 0, 1);
	effect->GetVariableByName("World")->AsMatrix()->SetMatrix(terrainController.getWorld());
	effect->GetVariableByName("ViewProjection")->AsMatrix()->SetMatrix(camera.getView() * camera.getProjection());

	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (unsigned lod = 0; lod < detailLevels; ++lod)
		if (numInstances[lod])
		{
			/*vertexElementDesc[1].InstanceDataStepRate = numInstances[lod];
			vertexElementDesc[2].InstanceDataStepRate = numInstances[lod];*/
			
			effect->GetVariableByName("LevelOfDetail")->AsScalar()->SetInt(lod);
			pass->Apply(0, deviceContext);
			tessellator.drawPatches(device, lod, instances[lod], numInstances[lod]);
			tessellator.drawSkirts(device, lod, instances[lod], numInstances[lod]);
		}

	deviceContext->Release();
}


void TerrainView::onResetDevice(ID3D11Device* device)
{
	tessellator.onResetDevice(device);
}


void TerrainView::release()
{
	if(effect) effect->Release();
	if(inputLayout) inputLayout->Release();
	tessellator.onLostDevice();
}


unsigned TerrainView::getBlockIndex(const TerrainPoint& p) const
{
	return getBlockIndex(p.row, p.col);
}


unsigned TerrainView::getBlockIndex(unsigned row, unsigned col) const
{
	return row * (width / patchSize) + col;
}


BoundingBox TerrainView::getBoundingBox(unsigned row, unsigned col, unsigned numRows, unsigned numCols) const
{
	float bumpiness = terrainController.getBumpiness();
	const Vector3& position = terrainController.getPosition();

	float minX = position.x + col * patchSize;
	float minZ = position.z + row * patchSize;

	Vector3 min(minX, -bumpiness, minZ);
	Vector3 max(minX + numCols * patchSize, bumpiness, minZ + numRows * patchSize);

	return BoundingBox(min, max);
}
