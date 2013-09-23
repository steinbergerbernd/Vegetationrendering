#include "DefaultModel.h"
#include "VegetationRendering.h"

#define SHININESS 5

DefaultModel::DefaultModel(const VegetationRendering& vegetationRendering, const wchar_t* modelPath) : vegetationRendering(vegetationRendering),
	effect(0), inputLayout(0), modelPath(modelPath)
{
}


DefaultModel::~DefaultModel(void)
{
}


void DefaultModel::init(ID3D11Device* device)
{
	HRESULT result = model.Create(device, modelPath, true);

	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	result = D3DX11CompileFromFileA("Content\\Effects\\DefaultModelShader.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, &shaderBlob, &errorBlob, 0);

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

	technique = effect->GetTechniqueByIndex(0);
	pass = technique->GetPassByIndex(0);

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	int inputElementCount = sizeof(inputElementDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	D3DX11_PASS_DESC passDesc;
	pass->GetDesc(&passDesc);

	result = device->CreateInputLayout(inputElementDesc, inputElementCount,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &inputLayout);

	evWorld = effect->GetVariableByName("World")->AsMatrix();
	evViewProjection = effect->GetVariableByName("ViewProjection")->AsMatrix();
	evModelTexture = effect->GetVariableByName("ModelTexture")->AsShaderResource();

	evCameraPosition = effect->GetVariableByName("CameraPosition")->AsVector();
	evLightVector = effect->GetVariableByName("LightVector")->AsVector();
	evAmbientLight = effect->GetVariableByName("AmbientLight")->AsVector();
	evDiffuseLight = effect->GetVariableByName("DiffuseLight")->AsVector();
	evSpecularLight = effect->GetVariableByName("SpecularLight")->AsVector();
	evShininess = effect->GetVariableByName("Shininess")->AsScalar();
}


void DefaultModel::release()
{
	if(effect) effect->Release();
	if(inputLayout) inputLayout->Release();
}


void DefaultModel::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	/*deviceContext->IASetInputLayout(inputLayout);
	Camera& camera = vegetationRendering.getCamera();
	evWorldViewProjection->SetMatrix(world * camera.getView() * camera.getProjection());
	pass->Apply(0, deviceContext);
	treeMesh.Render(deviceContext, 0);*/

	ID3D11Buffer* vertexBuffer = model.GetVB11(0, 0);
	unsigned vertexStride = model.GetVertexStride(0,0);
	unsigned offset = 0;
	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &vertexStride, &offset);
	deviceContext->IASetIndexBuffer(model.GetIB11(0), model.GetIBFormat11(0), 0);

	Camera& camera = vegetationRendering.getCamera();
	const Light& light = vegetationRendering.getLight();

	evWorld->SetMatrix(world);
	evViewProjection->SetMatrix(camera.getView() * camera.getProjection());

	evCameraPosition->SetFloatVector(camera.getPosition());
	evLightVector->SetFloatVector(Vector3(light.Direction.x, light.Direction.y, light.Direction.z));
	evAmbientLight->SetFloatVector(Vector3(light.Ambient.r, light.Ambient.g, light.Ambient.b));
	evDiffuseLight->SetFloatVector(Vector3(light.Diffuse.r, light.Diffuse.g, light.Diffuse.b));
	evSpecularLight->SetFloatVector(Vector3(light.Specular.r, light.Specular.g, light.Specular.b));
	evShininess->SetFloat(SHININESS);

	for(unsigned i = 0; i < model.GetNumSubsets(0); ++i)
	{
		SDKMESH_SUBSET* subset = model.GetSubset(0, i);
		D3D11_PRIMITIVE_TOPOLOGY primitiveType = model.GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)subset->PrimitiveType);
		deviceContext->IASetPrimitiveTopology(primitiveType);
		SDKMESH_MATERIAL* material = model.GetMaterial(subset->MaterialID);
		if(material)
			evModelTexture->SetResource(material->pDiffuseRV11);
		pass->Apply(0, deviceContext);
		deviceContext->DrawIndexed((unsigned)subset->IndexCount, (unsigned)subset->IndexStart, (int)subset->VertexStart);
	}

	deviceContext->Release();
}


void DefaultModel::setWorld(Matrix world)
{
	this->world = world;
}