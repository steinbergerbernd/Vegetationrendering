#pragma once

#include <D3D11.h>
#include <D3DX11.h>
#include <d3dx11effect.h>

#include "Types.h"
#include "Config.h"
#include "InputManager.h"
#include "Camera.h"
#include "Light.h"
#include "Triangle.h"
#include "Terrain.h"
#include "TessellationTerrain.h"
#include "GSGrass.h"
#include "DefaultModel.h"
#include "ComplexTree.h"
#include "ComplexGrass.h"

#include <iostream>
#include <fstream>

#define CAPTURE_TIME 6.0f
#define CAPTURE_INIT_TIME 2.0f

class VegetationRendering
{
public:
	VegetationRendering(void);
	~VegetationRendering(void);

	void init(ID3D11Device* device);
	void release();

	void update(const GameTime& gameTime);
	void draw(ID3D11Device* device, const GameTime& gameTime);

	void onLostDevice();
	void onResetDevice(ID3D11Device* device);

	bool isGSCullingEnabled() const { return gsCullingEnabled; }
	bool showSaved() const { return showSavedEnabled; }
	bool isInCaptureMode() const { return captureMode; }

	Matrix getSavedViewMatrix() const { return savedViewMatrix; }
	Matrix getSavedProjectionMatrix() const { return savedProjectionMatrix; }

	Camera& getCamera() const { return *camera; }
	const Light& getLight() const { return light; }
	Terrain& getTerrain() const { return *terrain; }

private:
	std::vector<unsigned> grassTestInstanceCounts;
	std::vector<unsigned> treeTestInstanceCounts;
	bool grassTests;

	float currentCaptureTime;
	bool captureMode;
	int currentCaptureView;

	int instancesIndex;
	int instancesCount;

	int capturedInstances;
	int capturedBorder;

	double frameSum;
	double frameCount;

	std::ofstream captureFile;
	std::ofstream captureSumFile;
	std::string captureFilename;

	static const char* configPath;

	ID3D11RasterizerState* rsCullNoneWireframe;
	ID3D11RasterizerState* rsCullBackWireframe;
	ID3D11RasterizerState* rsCullNoneSolid;
	ID3D11RasterizerState* rsCullBackSolid;
	ID3D11RasterizerState* rsCullBack;
	ID3D11RasterizerState* rsCullNone;

	ID3D11BlendState* bsDefault;
	ID3D11BlendState* bsAlphaToCoverage;

	Matrix savedViewMatrix;
	Matrix savedProjectionMatrix;
	bool showSavedEnabled;

	// terrain
	Terrain* terrain;
	TessellationTerrain* tessellationTerrain;

	// global objects
	InputManager* inputManager;
	Camera* camera;
	Light light;
	
	// simple test triangle
	Triangle* triangle;

	// grass objects
	GSGrass* gsGrass;
	ComplexGrass* complexGrass;
	DefaultModel* defaultGrassModel;
	DefaultModel* defaultGrassDetailedModel;

	// tree objects
	DefaultModel* separateLeafModel;
	DefaultModel* billboardLeafModel;
	DefaultModel* defaultTrunkModel;
	DefaultModel* defaultTrunkDetailedModel;

	ComplexTree* trunk;

	bool wireframeEnabled;
	bool treeModelsEnabled;
	bool gsGrassEnabled;
	bool complexGrassEnabled;
	bool defaultGrassEnabled;
	bool defaultDetailedEnabled;
	bool defaultTreesEnabled;
	bool complexTreesEnabled;
	bool gsCullingEnabled;

	void initRasterizerStates(ID3D11Device* device);
	void initBlendStates(ID3D11Device* device);
	void toggleCursor();
	void toggleWireframe();

	void startCapturing(std::string filename);
	void stopCapturing();
	void goToNextCaptureFile();
	void outputSum();
};