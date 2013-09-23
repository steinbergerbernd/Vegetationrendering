#include "FBXModel.h"


FBXModel::FBXModel(void)
{
}


FBXModel::~FBXModel(void)
{
}


FBXModel FBXModel::createFromFile(ID3D11Device* device, const std::string& filename)
{
	FBXModel model;

	model.load(device, filename);

	return model;
}


void FBXModel::load(ID3D11Device* device, const std::string& filename)
{
	KFbxSdkManager* sdkManager = KFbxSdkManager::Create();
	KFbxIOSettings* ios = KFbxIOSettings::Create(sdkManager, IOSROOT);
	sdkManager->SetIOSettings(ios);

	// Create an importer using our sdk manager.
	KFbxImporter* importer = KFbxImporter::Create(sdkManager, "");

	importer->Initialize(filename.c_str(), -1, sdkManager->GetIOSettings());

	// Create a new scene so it can be populated by the imported file.
	KFbxScene* scene = KFbxScene::Create(sdkManager, "");

	// Import the contents of the file into the scene.
	importer->Import(scene);

	KFbxNode* rootBone = 0;
	KFbxNode* rootNode = scene->GetRootNode();

	loadMeshes(rootNode, device, KFbxGeometryConverter(sdkManager));

	sdkManager->Destroy();
}


void FBXModel::loadMeshes(KFbxNode* node, ID3D11Device* device, KFbxGeometryConverter& converter)
{
	const char* name = node->GetName();

	if (node->GetNodeAttribute())
	{
		KFbxNodeAttribute::EAttributeType attributeType = node->GetNodeAttribute()->GetAttributeType();

		if (attributeType == KFbxNodeAttribute::eMESH)
		{
			KFbxMesh* kfbxMesh = converter.TriangulateMesh((KFbxMesh*)node->GetNodeAttribute());

			VertexCollection vertices = getVertices(kfbxMesh);
			//MaterialCollection materials = getMaterials(device, node);

			unsigned long numFaces = kfbxMesh->GetPolygonCount();
			unsigned long numVertices = kfbxMesh->GetControlPointsCount();
				
			/*Mesh mesh = Mesh::create(device, numFaces, numVertices, FBXVertex::vertexElements, D3DXMESH_MANAGED | D3DXMESH_32BIT);

			mesh.getVertexBuffer().setData(vertices);
			mesh.getIndexBuffer().setData(kfbxMesh->GetPolygonVertices(), kfbxMesh->GetPolygonVertexCount());

			KFbxLayerElementArrayTemplate<int>* materialIndices;
			kfbxMesh->GetMaterialIndices(&materialIndices);

			unsigned long* buffer = mesh.lockAttributeBuffer();

			for(int i = 0; i < kfbxMesh->GetPolygonCount(); ++i)
				buffer[i] = materialIndices->GetAt(i);
			
			mesh.unlockAttributeBuffer();
				
			Mesh::Adjacency adjacency = mesh.generateAdjacency();

			mesh.clean(D3DXCLEAN_SIMPLIFICATION, adjacency);
			mesh.optimizeInplace(D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, adjacency);
			mesh.computeNormals(adjacency);

			meshes[name] = FBXMesh(mesh, materials, name);*/
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++)
		loadMeshes(node->GetChild(i), device, converter);
}


FBXModel::VertexCollection FBXModel::getVertices(KFbxMesh* mesh)
{
	const KFbxLayer* layer = mesh->GetLayer(0);
	
	const KFbxLayerElementNormal* normals = layer->GetNormals();
	const KFbxLayerElementUV* texCoord = layer->GetUVs();

	VertexCollection vertices = VertexCollection(mesh->GetControlPointsCount());

	// positions
	for (unsigned i = 0; i < vertices.size(); i++)
		vertices[i].position = convert(mesh->GetControlPointAt(i));

	// normals
	if (normals)
	{
		for (unsigned i = 0; i < vertices.size(); i++)
			vertices[i].normal = Vector3::normalize(convert(normals->GetDirectArray().GetAt(i)));
	}

	// UV
	if (texCoord)
	{
		for (int i = 0; i < mesh->GetPolygonCount(); ++i)
			for (int j = 0; j < mesh->GetPolygonSize(i); ++j)
				vertices[mesh->GetPolygonVertex(i, j)].texCoord = convert(texCoord->GetDirectArray().GetAt(mesh->GetTextureUVIndex(i,j)));
	}

	return vertices;
}


Matrix FBXModel::convert(const KFbxXMatrix& matrix)
{
	Matrix m;

	for (int row = 0; row < 4; ++row)
		for (int col = 0; col < 4; ++col)
			m[row * 4 + col] = (float)matrix.Get(row, col);

	return m;
}


Vector4 FBXModel::convert(const KFbxVector4& vector)
{
	return Vector4((float)vector.GetAt(0), (float)vector.GetAt(1), (float)vector.GetAt(2), (float)vector.GetAt(3));
}


D3DCOLORVALUE FBXModel::convert(const fbxDouble3& vector, float a)
{
	return Vector4((float)vector[0], (float)vector[1], (float)vector[2], a);
}


Vector2 FBXModel::convert(const KFbxVector2& vector)
{
	return Vector2((float)vector.GetAt(0), (float)vector.GetAt(1));
}