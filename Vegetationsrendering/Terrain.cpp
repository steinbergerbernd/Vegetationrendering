#include "Terrain.h"
#include "Camera.h"
#include "VegetationRendering.h"

#define INSTANCE_COUNT_X 10
#define INSTANCE_COUNT_Y 10
#define INSTANCE_COUNT INSTANCE_COUNT_X * INSTANCE_COUNT_Y
#define INSTANCE_SIZE 500.0f

Terrain::Terrain(const VegetationRendering& vegetationRendering) : vegetationRendering(vegetationRendering),
	inputLayout(0), vertexBuffer(0), indexBuffer(0), effect(0), instanceBuffer(0), surfaceTexture(0)
{
}


Terrain::~Terrain(void)
{
}


void Terrain::init(ID3D11Device* device)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	HRESULT result = D3DX11CompileFromFileA("Content\\Effects\\SimpleTerrain.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, &shaderBlob, &errorBlob, 0);

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

	// vertex buffer
	TerrainVertex vertices[] =
	{
		{Vector3(0.0f, 0, INSTANCE_SIZE), Vector3::up, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), Vector2(0, 0)},
		{Vector3(INSTANCE_SIZE, 0, INSTANCE_SIZE), Vector3::up, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), Vector2(1, 0)},
		{Vector3(0.0f, 0, 0.0f), Vector3::up, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), Vector2(0, 1)},
		{Vector3(INSTANCE_SIZE, 0, 0.0f), Vector3::up, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), Vector2(1, 1)}
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(vertices);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory( &vertexData, sizeof(D3D11_SUBRESOURCE_DATA) );
	vertexData.pSysMem = vertices;

	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	// Instance Buffer
	TerrainInstance instances[INSTANCE_COUNT];
	for(int i = 0; i < INSTANCE_COUNT_X; ++i)
		for(int j = 0; j < INSTANCE_COUNT_Y; ++j)
			instances[i * INSTANCE_COUNT_X + j].position = Vector3((float)i*INSTANCE_SIZE, 0, (float)j*INSTANCE_SIZE);

	D3D11_BUFFER_DESC instanceBufferDesc;
	ZeroMemory(&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));

	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(instances);
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA instanceData;
	ZeroMemory(&instanceData, sizeof(D3D11_SUBRESOURCE_DATA));
	instanceData.pSysMem = instances;

	device->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);

	// index buffer
	unsigned indices[] = { 0, 1, 2, 3 };
	indexCount = sizeof(indices) / sizeof(unsigned);

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexData.pSysMem = indices;

	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	int inputElementCount = sizeof(inputElementDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	
	technique = effect->GetTechniqueByIndex(0);
	pass = technique->GetPassByIndex(0);

	D3DX11_PASS_DESC passDesc;
	pass->GetDesc(&passDesc);

	result = device->CreateInputLayout(inputElementDesc, inputElementCount,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &inputLayout);

	DXUTGetGlobalResourceCache().CreateTextureFromFile(device, deviceContext, L"Content/Textures/Terrain/Grass.dds", &surfaceTexture);
	evSurfaceTexture = effect->GetVariableByName("SurfaceTexture")->AsShaderResource();
	evWorld = effect->GetVariableByName("World")->AsMatrix();
	evViewProjection = effect->GetVariableByName("ViewProjection")->AsMatrix();

	evSurfaceTexture->SetResource(surfaceTexture);

	world = Matrix::createTranslation((-INSTANCE_COUNT_X * INSTANCE_SIZE) / 2, 0, (-INSTANCE_COUNT_Y * INSTANCE_SIZE) / 2);

	deviceContext->Release();
}


void Terrain::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	unsigned strides[2] = { sizeof(TerrainVertex), sizeof(TerrainInstance) };
	unsigned offsets[2] = { 0, 0 };
	ID3D11Buffer* buffers[2] = { vertexBuffer, instanceBuffer };
	deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	Camera& camera = vegetationRendering.getCamera();
	evWorld->SetMatrix(world);
	evViewProjection->SetMatrix(camera.getView() * camera.getProjection());

	pass->Apply(0, deviceContext);
	deviceContext->DrawIndexedInstanced(4, INSTANCE_COUNT, 0, 0, 0);
	//deviceContext->DrawInstanced(4, INSTANCE_COUNT, 0, 0);

	deviceContext->Release();
}


void Terrain::release()
{
	if(inputLayout) inputLayout->Release();
	if(vertexBuffer) vertexBuffer->Release();
	if(instanceBuffer) instanceBuffer->Release();
	if(indexBuffer) indexBuffer->Release();
	if(effect) effect->Release();
	if(surfaceTexture) surfaceTexture->Release();
}