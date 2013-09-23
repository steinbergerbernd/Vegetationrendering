#pragma once

#include <D3D11.h>
#include <D3DX11.h>
#include <d3dx11effect.h>
#include "DXUT.h"
#include "SDKmesh.h"
#include "SDKmisc.h"

#include "GameTime.h"
#include "Types.h"

class VegetationRendering;

#define GRASS_PATCH_COUNT 5000
#define GRASS_BORDER 200
#define GRASS_SHININESS 5
#define GRASS_MIN_DISTANCE 1.0f
#define GRASS_MAX_DISTANCE 20.0f
#define GRASS_BLADES_PER_PATCH 14
#define GRASS_VERTICES_PER_BLADE 4
#define GRASS_INDEX_COUNT GRASS_BLADES_PER_PATCH * GRASS_VERTICES_PER_BLADE
#define GRASS_VERTEX_COUNT GRASS_INDEX_COUNT
#define GRASS_NEAR_PLANE 0.0f
#define GRASS_FAR_PLANE 200.0f
#define GRASS_TESSELLATION_FACTOR 7.0f

class ComplexGrass
{
public:
	ComplexGrass(const VegetationRendering& vegetationRendering);
	~ComplexGrass(void);

	void init(ID3D11Device* device);
	void draw(ID3D11Device* device, const GameTime& gameTime);

	void release();

	void toggleTessellationMode() { tessellationMode = !tessellationMode; }

	float getGrassNearPlane() const { return GRASS_NEAR_PLANE; }
	float getGrassFarPlane() const { return GRASS_FAR_PLANE; }

	//int getGrassPatchCount() const { return GRASS_PATCH_COUNT; }
	int getBorder() const { return GRASS_BORDER; }

	int getDrawInstanceCount() const { return drawInstanceCount; }
	void setDrawInstanceCount(unsigned count) { this->drawInstanceCount = count; }

	Vector3 getScale() const { return tessellationMode ? scaleTessGrass : scaleGSGrass; }

	const Vector3* const getGrassPatchPositions() const { return grassPatchPositions; }
private:
	unsigned drawInstanceCount;

	Vector3 grassPatchPositions[GRASS_PATCH_COUNT];

	struct GrassPatchVertex
	{
		Vector4 position;
		Vector3 normal;
		Vector2 texCoord;
	};

	const VegetationRendering& vegetationRendering;

	bool tessellationMode;

	Matrix world;

	Vector3 scaleTessGrass;
	Vector3 scaleGSGrass;

	CDXUTSDKMesh grassPatchMesh;
	Vector3 boundingBoxCenter;
	Vector3 boundingBoxExtents;

	ID3D11ShaderResourceView* grassLeafTexture;

	ID3D11Buffer* vertexBufferPositions;
	ID3D11Buffer* vertexBufferGrass;
	ID3D11Buffer* indexBufferGrass;

	ID3DX11Effect* effect;
	ID3DX11EffectTechnique* techniqueGS;
	ID3DX11EffectTechnique* techniqueTess;
	ID3DX11EffectPass* passGS;
	ID3DX11EffectPass* passTess;

	ID3D11InputLayout* inputLayoutGS;
	ID3D11InputLayout* inputLayoutTess;

	ID3DX11EffectMatrixVariable* evWorld;
	ID3DX11EffectMatrixVariable* evViewProjection;
	ID3DX11EffectShaderResourceVariable* evGrassLeafTexture;
	ID3DX11EffectVectorVariable* evGeometryGrassPositions;
	ID3DX11EffectScalarVariable* evGeometryGrassNormals;
	ID3DX11EffectScalarVariable* evGeometryGrassTexCoords;
	ID3DX11EffectScalarVariable* evGeometryGrassIndices;
	ID3DX11EffectScalarVariable* evMinDistance;
	ID3DX11EffectScalarVariable* evMaxDistance;
	ID3DX11EffectScalarVariable* evBoundingBoxCenter;
	ID3DX11EffectScalarVariable* evBoundingBoxExtents;
	ID3DX11EffectScalarVariable* evGSCulling;
	ID3DX11EffectScalarVariable* evShowSavedCulling;
	ID3DX11EffectMatrixVariable* evSavedViewProjection;
	ID3DX11EffectScalarVariable* evTessellationFactor;

	ID3DX11EffectVectorVariable* evCameraPosition;
	ID3DX11EffectVectorVariable* evLightVector;
	ID3DX11EffectVectorVariable* evAmbientLight;
	ID3DX11EffectVectorVariable* evDiffuseLight;
	ID3DX11EffectVectorVariable* evSpecularLight;
	ID3DX11EffectScalarVariable* evShininess;
};

