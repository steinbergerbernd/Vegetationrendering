#pragma once

#include <d3d11.h>
#include <string>
#include <fbxsdk.h>
#include <vector>
#include "FBXVertex.h"
#include "FBXMaterial.h"

class FBXModel
{
	typedef std::vector<FBXVertex> VertexCollection;
	typedef std::vector<FBXMaterial> MaterialCollection;
public:
	FBXModel(void);
	~FBXModel(void);

	static FBXModel createFromFile(ID3D11Device* device, const std::string& filename);
	void load(ID3D11Device* device, const std::string& filename);
private:
	void loadMeshes(KFbxNode* node, ID3D11Device* device, KFbxGeometryConverter& converter);

	VertexCollection getVertices(KFbxMesh* mesh);

	static Matrix convert(const KFbxXMatrix& matrix);
	static Vector4 convert(const KFbxVector4& vector);
	static Vector2 convert(const KFbxVector2& vector);
	static D3DCOLORVALUE convert(const fbxDouble3& vector, float a = 1.0f);
};

