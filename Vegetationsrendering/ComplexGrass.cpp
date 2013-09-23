#include "ComplexGrass.h"
#include "VegetationRendering.h"

ComplexGrass::ComplexGrass(const VegetationRendering& vegetationRendering) : vegetationRendering(vegetationRendering),
	effect(0), inputLayoutGS(0), inputLayoutTess(0), grassLeafTexture(0), vertexBufferPositions(0), vertexBufferGrass(0),
	indexBufferGrass(0), tessellationMode(false), drawInstanceCount(GRASS_PATCH_COUNT)
{
}


ComplexGrass::~ComplexGrass(void)
{
}


void ComplexGrass::init(ID3D11Device* device)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	HRESULT result = D3DX11CompileFromFileA("Content\\Effects\\ComplexGrass.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, &shaderBlob, &errorBlob, 0);

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

	techniqueGS = effect->GetTechniqueByName("RenderGrassGS");
	techniqueTess = effect->GetTechniqueByName("RenderGrassTess");
	passGS = techniqueGS->GetPassByIndex(0);
	passTess = techniqueTess->GetPassByIndex(0);

	srand(0);
	for(int i = 0; i < GRASS_PATCH_COUNT; ++i)
	{
		grassPatchPositions[i].x = (rand() / (float)RAND_MAX) * GRASS_BORDER - GRASS_BORDER / 2.0f;
		grassPatchPositions[i].y = 0;
		grassPatchPositions[i].z = (rand() / (float)RAND_MAX) * GRASS_BORDER - GRASS_BORDER / 2.0f;
	}
	
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vector3) * GRASS_PATCH_COUNT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory( &vertexData, sizeof(D3D11_SUBRESOURCE_DATA) );
	vertexData.pSysMem = grassPatchPositions;
	vertexData.SysMemPitch = vertexData.SysMemSlicePitch = 0;

	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBufferPositions);

	D3D11_INPUT_ELEMENT_DESC inputElementDescGS[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	int inputElementCount = sizeof(inputElementDescGS) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	D3DX11_PASS_DESC passDesc;
	passGS->GetDesc(&passDesc);

	device->CreateInputLayout(inputElementDescGS, inputElementCount,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &inputLayoutGS);

	D3D11_INPUT_ELEMENT_DESC inputElementDescTess[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	inputElementCount = sizeof(inputElementDescTess) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	passTess->GetDesc(&passDesc);
	device->CreateInputLayout(inputElementDescTess, inputElementCount,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &inputLayoutTess);

	evWorld = effect->GetVariableByName("World")->AsMatrix();
	evViewProjection = effect->GetVariableByName("ViewProjection")->AsMatrix();
	evGrassLeafTexture = effect->GetVariableByName("GrassLeafTexture")->AsShaderResource();
	evGeometryGrassPositions = effect->GetVariableByName("GeoPositions")->AsVector();
	evGeometryGrassNormals = effect->GetVariableByName("GeoNormals")->AsScalar();
	evGeometryGrassTexCoords = effect->GetVariableByName("GeoTexCoords")->AsScalar();
	evGeometryGrassIndices = effect->GetVariableByName("GeoIndices")->AsScalar();
	evMinDistance = effect->GetVariableByName("MinDistance")->AsScalar();
	evMaxDistance = effect->GetVariableByName("MaxDistance")->AsScalar();
	evBoundingBoxCenter = effect->GetVariableByName("BoundingBoxCenter")->AsScalar();
	evBoundingBoxExtents = effect->GetVariableByName("BoundingBoxExtents")->AsScalar();
	evGSCulling = effect->GetVariableByName("GSCulling")->AsScalar();
	evShowSavedCulling = effect->GetVariableByName("ShowSavedCulling")->AsScalar();
	evSavedViewProjection = effect->GetVariableByName("SavedViewProjection")->AsMatrix();
	evTessellationFactor = effect->GetVariableByName("TessellationFactor")->AsScalar();

	evCameraPosition = effect->GetVariableByName("CameraPosition")->AsVector();
	evLightVector = effect->GetVariableByName("LightVector")->AsVector();
	evAmbientLight = effect->GetVariableByName("AmbientLight")->AsVector();
	evDiffuseLight = effect->GetVariableByName("DiffuseLight")->AsVector();
	evSpecularLight = effect->GetVariableByName("SpecularLight")->AsVector();
	evShininess = effect->GetVariableByName("Shininess")->AsScalar();
	
	result = grassPatchMesh.Create(device, L"Content/Models/GrassPatchQuads.sdkmesh", true);
	
	float* grassMeshVertices = (float*)grassPatchMesh.GetRawVerticesAt(0);
	GrassPatchVertex grassPatchVertices[GRASS_VERTEX_COUNT];
	float positions[GRASS_VERTEX_COUNT * 4];
	float normals[GRASS_VERTEX_COUNT * 3];
	float texCoords[GRASS_VERTEX_COUNT * 2];
	
	for(int i = 0; i < GRASS_VERTEX_COUNT; ++i)
	{
		positions[i*4] = grassPatchVertices[i].position.x = grassMeshVertices[i*8];
		positions[i*4 + 1] = grassPatchVertices[i].position.y = grassMeshVertices[i*8+1];
		positions[i*4 + 2] = grassPatchVertices[i].position.z = grassMeshVertices[i*8+2];
		positions[i*4 + 3] = grassPatchVertices[i].position.w = 1;
		normals[i*3] = grassPatchVertices[i].normal.x = grassMeshVertices[i*8 + 3];
		normals[i*3 + 1] = grassPatchVertices[i].normal.y = grassMeshVertices[i*8 + 4];
		normals[i*3 + 2] = grassPatchVertices[i].normal.z = grassMeshVertices[i*8 + 5];
		texCoords[i*2] = grassPatchVertices[i].texCoord.x = grassMeshVertices[i*8 + 6];
		texCoords[i*2 + 1] = grassPatchVertices[i].texCoord.y = grassMeshVertices[i*8 + 7];
	}

	int* grassIndices = (int*)grassPatchMesh.GetRawIndicesAt(0);
	int indices[GRASS_INDEX_COUNT];
	for(int i = 0; i < GRASS_BLADES_PER_PATCH; ++i)
	{
		indices[i*4] = grassIndices[i*6 + 1];
		indices[i*4 + 1] = grassIndices[i*6 + 2];
		indices[i*4 + 2] = grassIndices[i*6];
		indices[i*4 + 3] = grassIndices[i*6 + 4];
	}

	evGeometryGrassPositions->SetFloatVectorArray(positions, 0, GRASS_VERTEX_COUNT);
	evGeometryGrassNormals->SetFloatArray(normals, 0, GRASS_VERTEX_COUNT * 3);
	evGeometryGrassTexCoords->SetFloatArray(texCoords, 0, GRASS_VERTEX_COUNT * 2);
	evGeometryGrassIndices->SetIntArray(indices, 0, GRASS_INDEX_COUNT);

	vertexBufferDesc.ByteWidth = sizeof(grassPatchVertices);
	vertexData.pSysMem = grassPatchVertices;

	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBufferGrass);

	for(int i = 0; i < GRASS_BLADES_PER_PATCH; ++i)
	{
		int i2 = indices[i*4 + 2];
		indices[i*4 + 2] = indices[i*4 + 3];
		indices[i*4 + 3] = i2;
	}

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexData.pSysMem = indices;

	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBufferGrass);

	result = DXUTGetGlobalResourceCache().CreateTextureFromFile(device, deviceContext, L"GrassBlade.dds", &grassLeafTexture);
	evGrassLeafTexture->SetResource(grassLeafTexture);

	evMinDistance->SetFloat(GRASS_MIN_DISTANCE);
	evMaxDistance->SetFloat(GRASS_MAX_DISTANCE);
	evTessellationFactor->SetFloat(GRASS_TESSELLATION_FACTOR);

	boundingBoxCenter = grassPatchMesh.GetMeshBBoxCenter(0);
	boundingBoxExtents = grassPatchMesh.GetMeshBBoxExtents(0);

	scaleGSGrass = Vector3(20, 10, 20);
	//scaleTessGrass = Vector3(2.5f, 10, 2.5f);
	scaleTessGrass = Vector3(20,10, 20);

	deviceContext->Release();
}


void ComplexGrass::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	Camera& camera = vegetationRendering.getCamera();
	const Light& light = vegetationRendering.getLight();

	evShowSavedCulling->SetBool(vegetationRendering.showSaved());
	evSavedViewProjection->SetMatrix(vegetationRendering.getSavedViewMatrix() * vegetationRendering.getSavedProjectionMatrix());

	evGSCulling->SetBool(vegetationRendering.isGSCullingEnabled());
	evCameraPosition->SetFloatVector(camera.getPosition());
	evLightVector->SetFloatVector(Vector3(light.Direction.x, light.Direction.y, light.Direction.z));
	evAmbientLight->SetFloatVector(Vector3(light.Ambient.r, light.Ambient.g, light.Ambient.b));
	evDiffuseLight->SetFloatVector(Vector3(light.Diffuse.r, light.Diffuse.g, light.Diffuse.b));
	evSpecularLight->SetFloatVector(Vector3(light.Specular.r, light.Specular.g, light.Specular.b));
	evShininess->SetFloat(GRASS_SHININESS);

	if(!tessellationMode)
	{
		unsigned stride = sizeof(Vector3);
		unsigned offset = 0;

		deviceContext->IASetVertexBuffers(0, 1, &vertexBufferPositions, &stride, &offset);
		deviceContext->IASetInputLayout(inputLayoutGS);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		
		evWorld->SetMatrix(Matrix::createScale(scaleGSGrass));
		evViewProjection->SetMatrix(camera.getView() * camera.getProjection(GRASS_NEAR_PLANE, GRASS_FAR_PLANE));
		evBoundingBoxCenter->SetFloatArray(boundingBoxCenter * Vector3(1, scaleGSGrass.y / 2.0f, 1), 0, 3);
		evBoundingBoxExtents->SetFloatArray(boundingBoxExtents * scaleGSGrass, 0, 3);
		passGS->Apply(0, deviceContext);
		deviceContext->Draw(drawInstanceCount, 0);
	}
	else
	{
		unsigned strides[2] = { sizeof(float) * 9, sizeof(float) * 3 };
		unsigned offsets[2] = { 0, 0 };
		ID3D11Buffer* buffers[2] = { vertexBufferGrass, vertexBufferPositions };

		deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
		deviceContext->IASetIndexBuffer(indexBufferGrass, DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetInputLayout(inputLayoutTess);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

		evWorld->SetMatrix(Matrix::createScale(scaleTessGrass));
		evViewProjection->SetMatrix(camera.getView() * camera.getProjection(GRASS_NEAR_PLANE, GRASS_FAR_PLANE));
		evBoundingBoxCenter->SetFloatArray(boundingBoxCenter * Vector3(1, scaleTessGrass.y / 2.0f, 1), 0, 3);
		evBoundingBoxExtents->SetFloatArray(boundingBoxExtents * scaleTessGrass, 0, 3);

		passTess->Apply(0, deviceContext);
		//deviceContext->DrawIndexed(GRASS_INDEX_COUNT, 0, 0);
		deviceContext->DrawIndexedInstanced(GRASS_INDEX_COUNT, drawInstanceCount, 0, 0, 0);
	}

	deviceContext->Release();
}


void ComplexGrass::release()
{
	if(effect) effect->Release();
	if(inputLayoutGS) inputLayoutGS->Release();
	if(inputLayoutTess) inputLayoutTess->Release();
	if(grassLeafTexture) grassLeafTexture->Release();
	if(vertexBufferPositions) vertexBufferPositions->Release();
	if(vertexBufferGrass) vertexBufferGrass->Release();
	if(indexBufferGrass) indexBufferGrass->Release();
}
