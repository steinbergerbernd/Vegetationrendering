#include "TessellationTerrain.h"
#include "Camera.h"
#include "VegetationRendering.h"

TessellationTerrain::TessellationTerrain(const VegetationRendering& vegetationRendering) : vegetationRendering(vegetationRendering),
	inputLayout(0), vertexBuffer(0), indexBuffer(0), effect(0)
{
}


TessellationTerrain::~TessellationTerrain(void)
{
}


void TessellationTerrain::init(ID3D11Device* device)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	HRESULT result = D3DX11CompileFromFileA("Content\\Effects\\TessellationTerrain.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, &shaderBlob, &errorBlob, 0);

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
		{Vector3(-10.0f, 0, 10.0f), Vector2(0, 0)},
		{Vector3(10.0f, 0, 10.0f), Vector2(1, 0)},
		{Vector3(10.0f, 0, -10.0f), Vector2(1, 1)},
		{Vector3(-10.0f, 0, -10.0f), Vector2(0, 1)},
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
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	int inputElementCount = sizeof(inputElementDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	
	technique = effect->GetTechniqueByIndex(0);
	pass = technique->GetPassByIndex(0);

	D3DX11_PASS_DESC passDesc;
	pass->GetDesc(&passDesc);

	result = device->CreateInputLayout(inputElementDesc, inputElementCount,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &inputLayout);

	deviceContext->Release();
}


void TessellationTerrain::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	unsigned strides = sizeof(TerrainVertex);
	unsigned offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offsets);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	Camera& camera = vegetationRendering.getCamera();
	effect->GetVariableByName("World")->AsMatrix()->SetMatrix(Matrix::identity);
	effect->GetVariableByName("ViewProjection")->AsMatrix()->SetMatrix(camera.getView() * camera.getProjection());

	pass->Apply(0, deviceContext);
	deviceContext->DrawIndexed(4, 0, 0);

	deviceContext->Release();
}


void TessellationTerrain::release()
{
	if(inputLayout) inputLayout->Release();
	if(vertexBuffer) vertexBuffer->Release();
	if(indexBuffer) indexBuffer->Release();
	if(effect) effect->Release();
}