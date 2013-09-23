#include "Triangle.h"
#include "Camera.h"
#include "VegetationRendering.h"

Triangle::Triangle(const VegetationRendering& vegetationRendering) : vegetationRendering(vegetationRendering)
{
}


Triangle::~Triangle(void)
{
}


void Triangle::init(ID3D11Device* device)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	HRESULT result = D3DX11CompileFromFileA("TriangleShaders.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, &shaderBlob, &errorBlob, 0);

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

	/*ID3D10Blob *VS, *PS;
	D3DX11CompileFromFile(L"TriangleShaders.hlsl", 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, 0, 0);
	D3DX11CompileFromFile(L"TriangleShaders.hlsl", 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, 0, 0);

	device->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &vertexShader);
	device->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pixelShader);

	deviceContext->VSSetShader(vertexShader, 0, 0);
	deviceContext->PSSetShader(pixelShader, 0, 0);*/

	TriangleVertex vertices[] =
	{
		{Vector3(0, 0.5f, 0.5f), D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
		{Vector3(0.5f, -0.5f, 0.5f), D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
		{Vector3(-0.5f, -0.5f, 0.5f), D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)}
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(TriangleVertex) * 3;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory( &initData, sizeof(initData) );
	initData.pSysMem = vertices;

	device->CreateBuffer(&vertexBufferDesc, &initData, &vertexBuffer);

	D3D11_INPUT_ELEMENT_DESC vertexElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	
	technique = effect->GetTechniqueByIndex(0);
	pass = technique->GetPassByIndex(0);

	D3DX11_PASS_SHADER_DESC vertexShaderDesc;
	D3DX11_EFFECT_SHADER_DESC effectShaderDesc;

	pass->GetVertexShaderDesc(&vertexShaderDesc);
	vertexShaderDesc.pShaderVariable->GetShaderDesc(vertexShaderDesc.ShaderIndex, &effectShaderDesc);

	//device->CreateInputLayout(vertexElementDesc, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &inputLayout);
	result = device->CreateInputLayout(vertexElementDesc, 2, effectShaderDesc.pBytecode, effectShaderDesc.BytecodeLength, &inputLayout);

	deviceContext->Release();
}


void Triangle::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	unsigned stride = sizeof(TriangleVertex);
	unsigned offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	Camera& camera = vegetationRendering.getCamera();
	effect->GetVariableByName("World")->AsMatrix()->SetMatrix(Matrix::identity);
	effect->GetVariableByName("ViewProjection")->AsMatrix()->SetMatrix(camera.getView() * camera.getProjection());

	//deviceContext->VSSetShader( vertexShader, NULL, 0 );
	//deviceContext->PSSetShader( pixelShader, NULL, 0 );

	pass->Apply(0, deviceContext);
	deviceContext->Draw(3, 0);

	deviceContext->Release();
}


void Triangle::release()
{
	//vertexShader->Release();
	//pixelShader->Release();
	inputLayout->Release();
	vertexBuffer->Release();
	effect->Release();
}