#pragma once

#include <D3D11.h>
#include <d3dx11effect.h>
#include "DXUT.h"
#include "SDKmesh.h"
#include "SDKmisc.h"
#include "Types.h"

class VegetationRendering;

#define TREE_MIN_DISTANCE 2.0f
#define TREE_MAX_DISTANCE 20.0f
#define TREE_TESSELLATION_FACTOR_EDGES 5.0f
#define TREE_TESSELLATION_FACTOR_INSIDE 10.0f
#define TREE_SHININESS 5
#define TREE_BORDER 100
#define TREE_INSTANCE_COUNT 200
#define TREE_NEAR_PLANE 0.0f
#define TREE_FAR_PLANE 200.0f

class ComplexTree
{
public:
	ComplexTree(const VegetationRendering& vegetationRendering, const wchar_t* trunkPath);
	ComplexTree(const VegetationRendering& vegetationRendering, const wchar_t* trunkPath, const wchar_t* foliagePath);
	~ComplexTree(void);

	void init(ID3D11Device* device);
	void release();

	void draw(ID3D11Device* device, const GameTime& gameTime);

	void setWorld(Matrix world);

	//int getTreeInstanceCount() const { return TREE_INSTANCE_COUNT; }
	int getBorder() const { return TREE_BORDER; }

	const Vector3* const getTreePositions() const { return treePositions; }

	int getDrawInstanceCount() const { return drawInstanceCount; }
	void setDrawInstanceCount(unsigned count) { this->drawInstanceCount = count; }
private:
	unsigned drawInstanceCount;

	const VegetationRendering& vegetationRendering;

	Vector3 treePositions[TREE_INSTANCE_COUNT];

	const wchar_t* trunkPath;
	const wchar_t* foliagePath;

	Matrix world;

	CDXUTSDKMesh trunkMesh;
	Vector3 boundingBoxCenter;
	Vector3 boundingBoxExtents;

	ID3D11Buffer* instanceBuffer;

	ID3D11ShaderResourceView* alternativeTrunkTexture;
	ID3D11ShaderResourceView* bumpMap;
	ID3D11ShaderResourceView* normalMap;
	ID3D11ShaderResourceView* billboardTop;
	ID3D11ShaderResourceView* billboardSide;
	ID3D11ShaderResourceView* billboardFront;

	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* technique;
	ID3DX11EffectTechnique* techniqueLOD1;
	ID3DX11EffectPass* pass;
	ID3DX11EffectPass* passLOD1;
	ID3D11InputLayout* inputLayout;
	ID3D11InputLayout* inputLayoutLOD1;

	ID3DX11EffectMatrixVariable* evViewProjection;
	ID3DX11EffectMatrixVariable* evWorld;
	ID3DX11EffectShaderResourceVariable* evTrunkTexture;
	ID3DX11EffectShaderResourceVariable* evBumpMap;
	ID3DX11EffectShaderResourceVariable* evNormalMap;
	ID3DX11EffectShaderResourceVariable* evBillboardTop;
	ID3DX11EffectShaderResourceVariable* evBillboardSide;
	ID3DX11EffectShaderResourceVariable* evBillboardFront;
	ID3DX11EffectScalarVariable* evEdgeFactor;
	ID3DX11EffectScalarVariable* evInsideFactor;
	ID3DX11EffectScalarVariable* evMinDistance;
	ID3DX11EffectScalarVariable* evMaxDistance;
	ID3DX11EffectScalarVariable* evBoundingBoxCenter;
	ID3DX11EffectScalarVariable* evBoundingBoxExtents;
	ID3DX11EffectScalarVariable* evGSCulling;
	ID3DX11EffectScalarVariable* evShowSavedCulling;
	ID3DX11EffectMatrixVariable* evSavedViewProjection;

	ID3DX11EffectVectorVariable* evCameraPosition;
	ID3DX11EffectVectorVariable* evLightVector;
	ID3DX11EffectVectorVariable* evAmbientLight;
	ID3DX11EffectVectorVariable* evDiffuseLight;
	ID3DX11EffectVectorVariable* evSpecularLight;
	ID3DX11EffectScalarVariable* evShininess;
};

