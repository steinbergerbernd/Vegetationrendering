#include "GSGrass.h"
#include "VegetationRendering.h"
#include "Terrain.h"

#define MAX_RAND_VALUE 1024

GSGrass::GSGrass(const VegetationRendering& vegetationRendering) : vegetationRendering(vegetationRendering),
	effect(0), grassTextures(0), grassTexturesSRV(0), rasterizerState(0), randomTexture(0), randomTextureSRV(0),
	inputLayout(0)
{
}


GSGrass::~GSGrass(void)
{
}


void GSGrass::init(ID3D11Device* device)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	D3D11_RASTERIZER_DESC rasterizerDesc;

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.FrontCounterClockwise = true;
	rasterizerDesc.DepthBias = false;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = true;
	rasterizerDesc.AntialiasedLineEnable = false;

	device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);

	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	HRESULT result = D3DX11CompileFromFileA("Content\\Effects\\GSGrass.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, &shaderBlob, &errorBlob, 0);

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

	technique = effect->GetTechniqueByName("RenderGrass");
	pass = technique->GetPassByIndex(0);

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	int inputElementCount = sizeof(inputElementDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	D3DX11_PASS_DESC passDesc;
	pass->GetDesc(&passDesc);

	device->CreateInputLayout(inputElementDesc, inputElementCount,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &inputLayout);

	evWorld = effect->GetVariableByName("World")->AsMatrix();
	evViewProjection = effect->GetVariableByName("ViewProjection")->AsMatrix();
	evGrassTextureArray = effect->GetVariableByName("GrassTextureArray")->AsShaderResource();
	evRandomTexture = effect->GetVariableByName("RandomTexture")->AsShaderResource();
	evGrassCoverage = effect->GetVariableByName("GrassCoverage")->AsScalar();
	evGrassWidth = effect->GetVariableByName("GrassWidth")->AsScalar();
	evGrassHeight = effect->GetVariableByName("GrassHeight")->AsScalar();
	evGrassMessiness = effect->GetVariableByName("GrassMessiness")->AsScalar();

	loadTextureArray(device);
	createRandomTexture(device);

	evRandomTexture->SetResource(randomTextureSRV);
	evGrassTextureArray->SetResource(grassTexturesSRV);
	evGrassWidth->SetFloat(1);
	evGrassHeight->SetFloat(1);
	evGrassCoverage->SetInt(1);
	evGrassMessiness->SetFloat(1);

	deviceContext->Release();
}


void GSGrass::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	//deviceContext->RSSetState(rasterizerState);

	Camera& camera = vegetationRendering.getCamera();
	evWorld->SetMatrix(Matrix::identity);
	evViewProjection->SetMatrix(camera.getView() * camera.getProjection());

	Terrain& terrain = vegetationRendering.getTerrain();
	ID3D11Buffer* vBuffer = terrain.getVertexBuffer();
	unsigned strides[1] = { sizeof(Terrain::TerrainVertex) };
	unsigned offsets[1] = { 0 };
	deviceContext->IASetVertexBuffers(0, 1, &vBuffer, strides, offsets);
	deviceContext->IASetIndexBuffer(terrain.getIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	deviceContext->IASetInputLayout(inputLayout);

	pass->Apply(0, deviceContext);
	deviceContext->DrawIndexed(4, 0, 0);

	deviceContext->Release();
}


void GSGrass::loadTextureArray(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC texDesc;
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	const char* grassTextureNames[] =
	{
		"Content\\Textures\\Grass\\grass_v1_basic_tex.dds",
		"Content\\Textures\\Grass\\grass_v2_light_tex.dds",
		"Content\\Textures\\Grass\\grass_v3_dark_tex.dds",
		"Content\\Textures\\Grass\\grass_guide_v3_tex.dds",
	};
	
	int textureCount = sizeof(grassTextureNames) / sizeof(grassTextureNames[0]);
	for(int i = 0; i < textureCount; ++i)
	{
		ID3D11Texture2D* texture;
		D3DX11_IMAGE_LOAD_INFO loadInfo;
		
		loadInfo.FirstMipLevel = 0;
		loadInfo.MipLevels = 1;
		loadInfo.Usage = D3D11_USAGE_STAGING;
		loadInfo.BindFlags = 0;
		loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		loadInfo.MiscFlags = 0;
		loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		loadInfo.Filter = D3DX11_FILTER_NONE;
		loadInfo.MipFilter = D3DX11_FILTER_NONE;

		D3DX11CreateTextureFromFileA(device, grassTextureNames[i], &loadInfo, 0, (ID3D11Resource**)&texture, 0);
		texture->GetDesc(&texDesc);

		if (i == 0)
		{
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.ArraySize = textureCount;
			device->CreateTexture2D(&texDesc, 0, &grassTextures);
		}

		D3D11_MAPPED_SUBRESOURCE mappedTexture;
		for (int j = 0; j < (int)texDesc.MipLevels; ++j)
		{
			deviceContext->Map(texture, j, D3D11_MAP_READ, 0, &mappedTexture);
			deviceContext->UpdateSubresource(grassTextures, D3D11CalcSubresource(j, i, texDesc.MipLevels),
				0, mappedTexture.pData, mappedTexture.RowPitch, 0);
			deviceContext->Unmap(texture, j);
		}

		texture->Release();
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2DArray.ArraySize = textureCount;
	device->CreateShaderResourceView(grassTextures, &srvDesc, &grassTexturesSRV);

	deviceContext->Release();
}


void GSGrass::createRandomTexture(ID3D11Device* device)
{
	srand(0);
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = new float[MAX_RAND_VALUE * 4];
	initData.SysMemPitch = initData.SysMemSlicePitch = sizeof(float) * MAX_RAND_VALUE * 4;

	for(int i = 0; i < MAX_RAND_VALUE * 4; ++i)
		((float*)initData.pSysMem)[i] = (float)(rand() % 10000 - 5000);

	D3D11_TEXTURE1D_DESC texDesc;
	texDesc.Width = MAX_RAND_VALUE;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	texDesc.ArraySize = 1;
	device->CreateTexture1D(&texDesc, &initData, &randomTexture);
	delete[] initData.pSysMem;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
	srvDesc.Texture1D.MipLevels = texDesc.MipLevels;
	device->CreateShaderResourceView(randomTexture, &srvDesc, &randomTextureSRV);
}


void GSGrass::release()
{
	if(effect) effect->Release();
	if(grassTextures) grassTextures->Release();
	if(grassTexturesSRV) grassTexturesSRV->Release();
	if(rasterizerState) rasterizerState->Release();
	if(randomTexture) randomTexture->Release();
	if(randomTextureSRV) randomTextureSRV->Release();
	if(inputLayout) inputLayout->Release();
}