#include "ComplexTree.h"
#include "VegetationRendering.h"

ComplexTree::ComplexTree(const VegetationRendering& vegetationRendering, const wchar_t* trunkPath)
	: vegetationRendering(vegetationRendering), trunkPath(trunkPath), foliagePath(0), bumpMap(0),
	alternativeTrunkTexture(0), world(Matrix::identity), normalMap(0), instanceBuffer(0),
	inputLayout(0), inputLayoutLOD1(0), billboardTop(0), billboardSide(0), billboardFront(0),
	drawInstanceCount(TREE_INSTANCE_COUNT)
{
}


ComplexTree::ComplexTree(const VegetationRendering& vegetationRendering, const wchar_t* trunkPath, const wchar_t* foliagePath)
	: vegetationRendering(vegetationRendering), trunkPath(trunkPath), foliagePath(foliagePath), bumpMap(0),
	alternativeTrunkTexture(0), world(Matrix::identity), normalMap(0), instanceBuffer(0),
	inputLayout(0), inputLayoutLOD1(0), billboardTop(0), billboardSide(0), billboardFront(0),
	drawInstanceCount(TREE_INSTANCE_COUNT)
{
}


ComplexTree::~ComplexTree(void)
{
}


void ComplexTree::init(ID3D11Device* device)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	HRESULT result = trunkMesh.Create(device, trunkPath, true);

	boundingBoxCenter = trunkMesh.GetMeshBBoxCenter(0);
	boundingBoxExtents = trunkMesh.GetMeshBBoxExtents(0);

	ID3DBlob* errorBlob;
	ID3DBlob* shaderBlob;

	result = D3DX11CompileFromFileA("Content/Effects/Trunk.fx", 0, 0, 0, "fx_5_0", 0, 0, 0, &shaderBlob, &errorBlob, 0);

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

	for(int i = 0; i < TREE_INSTANCE_COUNT; ++i)
	{
		treePositions[i].x = (rand() / (float)RAND_MAX) * TREE_BORDER - TREE_BORDER / 2.0f;
		treePositions[i].y = 0;
		treePositions[i].z = (rand() / (float)RAND_MAX) * TREE_BORDER - TREE_BORDER / 2.0f;
	}

	D3D11_BUFFER_DESC instanceBufferDesc;
	ZeroMemory(&instanceBufferDesc, sizeof(D3D11_BUFFER_DESC));

	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(treePositions);
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA instanceData;
	ZeroMemory(&instanceData, sizeof(D3D11_SUBRESOURCE_DATA));
	instanceData.pSysMem = treePositions;

	device->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);

	technique = effect->GetTechniqueByName("Trunk");
	techniqueLOD1 = effect->GetTechniqueByName("TrunkLOD1");
	pass = technique->GetPassByIndex(0);
	passLOD1 = techniqueLOD1->GetPassByIndex(0);

	D3D11_INPUT_ELEMENT_DESC inputElementDescLOD0[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1}
	};

	int inputElementCount = sizeof(inputElementDescLOD0) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	D3DX11_PASS_DESC passDesc;
	pass->GetDesc(&passDesc);
	result = device->CreateInputLayout(inputElementDescLOD0, inputElementCount,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &inputLayout);

	D3D11_INPUT_ELEMENT_DESC inputElementDescLOD1[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	inputElementCount = sizeof(inputElementDescLOD1) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	passLOD1->GetDesc(&passDesc);
	device->CreateInputLayout(inputElementDescLOD1, inputElementCount,
		passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &inputLayoutLOD1);

	CDXUTResourceCache& resourceCache = DXUTGetGlobalResourceCache();
	resourceCache.CreateTextureFromFile(device, deviceContext, L"Content/Textures/Trees/rinde_bumpmap.dds", &bumpMap);
	resourceCache.CreateTextureFromFile(device, deviceContext, L"Content/Textures/Trees/rinde.dds", &alternativeTrunkTexture);
	resourceCache.CreateTextureFromFile(device, deviceContext, L"Content/Textures/Trees/trunk414_normalmap.dds", &normalMap);
	resourceCache.CreateTextureFromFile(device, deviceContext, L"Content/Textures/Trees/trunk414_front.dds", &billboardFront);
	resourceCache.CreateTextureFromFile(device, deviceContext, L"Content/Textures/Trees/trunk414_side.dds", &billboardSide);
	resourceCache.CreateTextureFromFile(device, deviceContext, L"Content/Textures/Trees/trunk414_top.dds", &billboardTop);

	evViewProjection = effect->GetVariableByName("ViewProjection")->AsMatrix();
	evWorld = effect->GetVariableByName("World")->AsMatrix();
	evTrunkTexture = effect->GetVariableByName("TrunkTexture")->AsShaderResource();
	evBumpMap = effect->GetVariableByName("BumpMapTexture")->AsShaderResource();
	evNormalMap = effect->GetVariableByName("NormalMapTexture")->AsShaderResource();
	evBillboardTop = effect->GetVariableByName("BillboardTop")->AsShaderResource();
	evBillboardSide = effect->GetVariableByName("BillboardSide")->AsShaderResource();
	evBillboardFront = effect->GetVariableByName("BillboardFront")->AsShaderResource();
	evEdgeFactor = effect->GetVariableByName("EdgeFactor")->AsScalar();
	evInsideFactor = effect->GetVariableByName("InsideFactor")->AsScalar();
	evMinDistance = effect->GetVariableByName("MinDistance")->AsScalar();
	evMaxDistance = effect->GetVariableByName("MaxDistance")->AsScalar();
	evBoundingBoxCenter = effect->GetVariableByName("BoundingBoxCenter")->AsScalar();
	evBoundingBoxExtents = effect->GetVariableByName("BoundingBoxExtents")->AsScalar();
	evGSCulling = effect->GetVariableByName("GSCulling")->AsScalar();
	evShowSavedCulling = effect->GetVariableByName("ShowSavedCulling")->AsScalar();
	evSavedViewProjection = effect->GetVariableByName("SavedViewProjection")->AsMatrix();

	evCameraPosition = effect->GetVariableByName("CameraPosition")->AsVector();
	evLightVector = effect->GetVariableByName("LightVector")->AsVector();
	evAmbientLight = effect->GetVariableByName("AmbientLight")->AsVector();
	evDiffuseLight = effect->GetVariableByName("DiffuseLight")->AsVector();
	evSpecularLight = effect->GetVariableByName("SpecularLight")->AsVector();
	evShininess = effect->GetVariableByName("Shininess")->AsScalar();

	evBumpMap->SetResource(bumpMap);
	evTrunkTexture->SetResource(alternativeTrunkTexture);
	evNormalMap->SetResource(normalMap);
	evBillboardTop->SetResource(billboardTop);
	evBillboardFront->SetResource(billboardFront);
	evBillboardSide->SetResource(billboardSide);
	evEdgeFactor->SetFloat(TREE_TESSELLATION_FACTOR_EDGES);
	evInsideFactor->SetFloat(TREE_TESSELLATION_FACTOR_INSIDE);
	evMinDistance->SetFloat(TREE_MIN_DISTANCE);
	evMaxDistance->SetFloat(TREE_MAX_DISTANCE);
	evBoundingBoxCenter->SetFloatArray(boundingBoxCenter, 0, 3);
	evBoundingBoxExtents->SetFloatArray(boundingBoxExtents, 0, 3);

	deviceContext->Release();
}


void ComplexTree::release()
{
	if(effect) effect->Release();
	if(inputLayout) inputLayout->Release();
	if(inputLayoutLOD1) inputLayoutLOD1->Release();
	if(bumpMap) bumpMap->Release();
	if(alternativeTrunkTexture) alternativeTrunkTexture->Release();
	if(normalMap) normalMap->Release();
	if(billboardTop) billboardTop->Release();
	if(billboardSide) billboardSide->Release();
	if(billboardFront) billboardFront->Release();
	if(instanceBuffer) instanceBuffer->Release();
}


void ComplexTree::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	ID3D11Buffer* buffers[2] = { trunkMesh.GetVB11(0, 0), instanceBuffer };
	unsigned strides[2] = { trunkMesh.GetVertexStride(0,0), sizeof(Vector3) };
	unsigned offsets[2] = {0};
	deviceContext->IASetInputLayout(inputLayout);
	deviceContext->IASetVertexBuffers(0, 2, buffers, strides, offsets);
	deviceContext->IASetIndexBuffer(trunkMesh.GetIB11(0), trunkMesh.GetIBFormat11(0), 0);

	Camera& camera = vegetationRendering.getCamera();
	const Light& light = vegetationRendering.getLight();

	D3D11_VIEWPORT viewport;
	unsigned numViewports = 1;
	deviceContext->RSGetViewports(&numViewports, &viewport);

	evGSCulling->SetBool(vegetationRendering.isGSCullingEnabled());
	evShowSavedCulling->SetBool(vegetationRendering.showSaved());
	evSavedViewProjection->SetMatrix(vegetationRendering.getSavedViewMatrix() * vegetationRendering.getSavedProjectionMatrix());
	evCameraPosition->SetFloatVector(camera.getPosition());
	evLightVector->SetFloatVector(Vector3(light.Direction.x, light.Direction.y, light.Direction.z));
	evAmbientLight->SetFloatVector(Vector3(light.Ambient.r, light.Ambient.g, light.Ambient.b));
	evDiffuseLight->SetFloatVector(Vector3(light.Diffuse.r, light.Diffuse.g, light.Diffuse.b));
	evSpecularLight->SetFloatVector(Vector3(light.Specular.r, light.Specular.g, light.Specular.b));
	evShininess->SetFloat(TREE_SHININESS);

	evWorld->SetMatrix(world);
	evViewProjection->SetMatrix(camera.getView() * camera.getProjection(TREE_NEAR_PLANE, TREE_FAR_PLANE));

	for(unsigned i = 0; i < trunkMesh.GetNumSubsets(0); ++i)
	{
		SDKMESH_SUBSET* subset = trunkMesh.GetSubset(0, i);
		//D3D11_PRIMITIVE_TOPOLOGY primitiveType = trunkMesh.GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)subset->PrimitiveType);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
		SDKMESH_MATERIAL* material = trunkMesh.GetMaterial(subset->MaterialID);
		if(material)
			evTrunkTexture->SetResource(material->pDiffuseRV11);
		pass->Apply(0, deviceContext);
		deviceContext->DrawIndexedInstanced((unsigned)subset->IndexCount, drawInstanceCount,
			(unsigned)subset->IndexStart, (int)subset->VertexStart,0);
		//deviceContext->DrawIndexed((unsigned)subset->IndexCount, (unsigned)subset->IndexStart, (int)subset->VertexStart);
	}

	/*deviceContext->IASetInputLayout(inputLayoutLOD1);
	unsigned stride = sizeof(float) * 3;
	unsigned offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &instanceBuffer, &stride, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	evWorld->SetMatrix(Matrix::createScale(Vector3(9)));

	passLOD1->Apply(0, deviceContext);
	deviceContext->Draw(INSTANCE_COUNT, 0);*/

	deviceContext->Release();
}


void ComplexTree::setWorld(Matrix world)
{
	this->world = world;
}