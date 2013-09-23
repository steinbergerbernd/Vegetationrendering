#include "VegetationRendering.h"

const char* VegetationRendering::configPath = "Content/Config/Config.txt";

VegetationRendering::VegetationRendering(void) : wireframeEnabled(false), gsGrassEnabled(false), complexGrassEnabled(true),
	complexTreesEnabled(false), treeModelsEnabled(false), gsCullingEnabled(false), showSavedEnabled(false),
	defaultGrassEnabled(false), defaultTreesEnabled(false), defaultDetailedEnabled(false), captureMode(false)
{
	camera = new Camera();
	inputManager = new InputManager();
	terrain = new Terrain(*this);
	tessellationTerrain = new TessellationTerrain(*this);
	gsGrass = new GSGrass(*this);
	complexGrass = new ComplexGrass(*this);
	separateLeafModel = new DefaultModel(*this, L"Content/Models/tree2.sdkmesh");
	billboardLeafModel = new DefaultModel(*this, L"Content/Models/thinTree.sdkmesh");
	trunk = new ComplexTree(*this, L"Content/Models/trunk414.sdkmesh");
	defaultGrassModel = new DefaultModel(*this, L"Content/Models/GrassPatchQuadsNormals.sdkmesh");
	defaultGrassDetailedModel = new DefaultModel(*this, L"Content/Models/GrassPatch.sdkmesh");
	defaultTrunkModel = new DefaultModel(*this, L"Content/Models/trunk414.sdkmesh");
	defaultTrunkDetailedModel = new DefaultModel(*this, L"Content/Models/Tree3200.sdkmesh");
}


VegetationRendering::~VegetationRendering()
{
	delete camera;
	delete inputManager;
	delete terrain;
	delete tessellationTerrain;
	delete gsGrass;
	delete complexGrass;
	delete separateLeafModel;
	delete billboardLeafModel;
	delete trunk;
	delete defaultGrassModel;
	delete defaultGrassDetailedModel;
	delete defaultTrunkModel;
	delete defaultTrunkDetailedModel;
}


void VegetationRendering::init(ID3D11Device* device)
{
	Config::addFile(configPath);

	camera->init();
	inputManager->init();
	terrain->init(device);
	tessellationTerrain->init(device);
	gsGrass->init(device);
	complexGrass->init(device);
	defaultGrassModel->init(device);
	defaultGrassDetailedModel->init(device);
	separateLeafModel->init(device);
	separateLeafModel->setWorld(Matrix::createTranslation(10, 0, 10));
	billboardLeafModel->init(device);
	billboardLeafModel->setWorld(Matrix::createTranslation(20, 0, 10));
	trunk->init(device);
	defaultTrunkModel->init(device);
	defaultTrunkDetailedModel->init(device);

	//trunk->setDrawInstanceCount(50);

	light.Direction = Config::getValue<Vector3>(ConfigKeys::lightDirection);
	light.Ambient = Config::getValue<Vector3>(ConfigKeys::lightAmbient);
	light.Diffuse = Config::getValue<Vector3>(ConfigKeys::lightDiffuse);
	light.Specular = Config::getValue<Vector3>(ConfigKeys::lightSpecular);

	initRasterizerStates(device);
	initBlendStates(device);

	grassTestInstanceCounts.push_back(100);
	grassTestInstanceCounts.push_back(1000);
	grassTestInstanceCounts.push_back(5000);
	grassTestInstanceCounts.push_back(10000);
	grassTestInstanceCounts.push_back(20000);
	grassTestInstanceCounts.push_back(30000);
	grassTestInstanceCounts.push_back(50000);
	//grassTestInstanceCounts.push_back(100000);

	treeTestInstanceCounts.push_back(50);
	treeTestInstanceCounts.push_back(500);
	treeTestInstanceCounts.push_back(1000);
	treeTestInstanceCounts.push_back(5000);
	treeTestInstanceCounts.push_back(8000);
	treeTestInstanceCounts.push_back(10000);
	treeTestInstanceCounts.push_back(15000);
}


void VegetationRendering::initRasterizerStates(ID3D11Device* device)
{
	D3D11_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.SlopeScaledDepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0;
	rasterizerDesc.DepthClipEnable = true;
	rasterizerDesc.ScissorEnable = false;
	rasterizerDesc.MultisampleEnable = false;
	rasterizerDesc.AntialiasedLineEnable = false;

	device->CreateRasterizerState(&rasterizerDesc, &rsCullBackSolid);

	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	device->CreateRasterizerState(&rasterizerDesc, &rsCullBackWireframe);

	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	device->CreateRasterizerState(&rasterizerDesc, &rsCullNoneWireframe);

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	device->CreateRasterizerState(&rasterizerDesc, &rsCullNoneSolid);

	rsCullNone = rsCullNoneSolid;
	rsCullBack = rsCullBackSolid;
}


void VegetationRendering::initBlendStates(ID3D11Device* device)
{
	D3D11_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = false;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&blendDesc, &bsDefault);

	blendDesc.AlphaToCoverageEnable = true;
	device->CreateBlendState(&blendDesc, &bsAlphaToCoverage);
}


void VegetationRendering::release()
{
	terrain->release();
	tessellationTerrain->release();
	gsGrass->release();
	complexGrass->release();
	defaultGrassModel->release();
	defaultGrassDetailedModel->release();
	separateLeafModel->release();
	billboardLeafModel->release();
	trunk->release();
	defaultTrunkModel->release();
	defaultTrunkDetailedModel->release();

	rsCullNoneSolid->Release();
	rsCullNoneWireframe->Release();
	rsCullBackSolid->Release();
	rsCullBackWireframe->Release();

	bsDefault->Release();
	bsAlphaToCoverage->Release();
}


void VegetationRendering::update(const GameTime& gameTime)
{
	camera->update(gameTime);
	inputManager->update(gameTime);

	if (captureMode)
	{
		if (currentCaptureView < camera->getCurrentViewCounter())
		{
			outputSum();
			goToNextCaptureFile();
		}

		currentCaptureTime += gameTime.elapsed;
		if(currentCaptureTime > CAPTURE_INIT_TIME)
		{
			double frames = (double)DXUTGetFPS();
			captureFile << frames << std::endl;
			frameSum += frames;
			frameCount++;
		}

		if (camera->getCurrentViewCounter() == -1)
		{
			stopCapturing();
			instancesIndex++;
			if (instancesIndex < instancesCount)
				startCapturing(captureFilename);
			else
			{
				captureSumFile.close();
				trunk->setDrawInstanceCount(50);
			}
		}
	}

	if (Keyboard::isKeyPress('M'))
		toggleCursor();
	else if (Keyboard::isKeyPress('F'))
		toggleWireframe();
	else if (Keyboard::isKeyPress('T'))
		complexGrass->toggleTessellationMode();
	else if (Keyboard::isKeyPress('C'))
		gsCullingEnabled = !gsCullingEnabled;
	else if (Keyboard::isKeyPress('P'))
	{
		showSavedEnabled = !showSavedEnabled;
		if(showSavedEnabled)
		{
			savedViewMatrix = camera->getView();
			savedProjectionMatrix = camera->getProjection(complexGrass->getGrassNearPlane(), complexGrass->getGrassFarPlane());
		}
	}
	/*else if (Keyboard::isKeyPress('L'))
		camera->outputCameraView("CameraViews.txt");
	else if (Keyboard::isKeyPress('V'))
		camera->pauseVisitCameraViews();
	else if (Keyboard::isKeyPress('N'))
		camera->goToNextCameraView();*/
	/*else if (Keyboard::isKeyPress('7'))
	{
		instancesIndex = 0;
		instancesCount = grassTestInstanceCounts.size();
		grassTests = true;
		capturedBorder = complexGrass->getBorder();
		captureSumFile.open("GrassSum.txt", std::ios::out);
		startCapturing("GrassResults");
	}
	else if (Keyboard::isKeyPress('8'))
	{
		instancesIndex = 0;
		instancesCount = treeTestInstanceCounts.size();
		grassTests = false;
		capturedBorder = trunk->getBorder();
		captureSumFile.open("TreeSum.txt", std::ios::out);
		startCapturing("TreeResults");
	}*/
	/*else if (Keyboard::isKeyPress('1'))
		gsGrassEnabled = !gsGrassEnabled;*/
	else if (Keyboard::isKeyPress('2'))
		complexGrassEnabled = !complexGrassEnabled;
	else if (Keyboard::isKeyPress('3'))
		defaultGrassEnabled = !defaultGrassEnabled;
	else if (Keyboard::isKeyPress('4'))
		treeModelsEnabled = !treeModelsEnabled;
	else if (Keyboard::isKeyPress('5'))
		complexTreesEnabled = !complexTreesEnabled;
	else if (Keyboard::isKeyPress('6'))
		defaultTreesEnabled = !defaultTreesEnabled;
	/*else if (Keyboard::isKeyPress('Q'))
		defaultDetailedEnabled = !defaultDetailedEnabled;*/
}


void VegetationRendering::draw(ID3D11Device* device, const GameTime& gameTime)
{
	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	D3D11_VIEWPORT viewport;
	unsigned numViewports = 1;
	deviceContext->RSGetViewports(&numViewports, &viewport);

	camera->setAspectRatio(viewport.Width / viewport.Height);

	deviceContext->RSSetState(rsCullBack);
	deviceContext->OMSetBlendState(bsDefault, Vector4(0,0,0,0), 0xFFFFFFFF);
	terrain->draw(device, gameTime);
	//tessellationTerrain->draw(device, gameTime);
	if (complexTreesEnabled) trunk->draw(device, gameTime);

	deviceContext->RSSetState(rsCullNone);
	if(treeModelsEnabled)
	{
		separateLeafModel->draw(device, gameTime);
		billboardLeafModel->draw(device, gameTime);
	}

	if (defaultTreesEnabled)
	{
		for(int i = 0; i < trunk->getDrawInstanceCount(); ++i)
		{
			Matrix trunkWorld = Matrix::createTranslation(trunk->getTreePositions()[i]);
			/*if(defaultDetailedEnabled)
			{
				defaultTrunkDetailedModel->setWorld(trunkWorld);
				defaultTrunkDetailedModel->draw(device, gameTime);
			}
			else
			{*/
			defaultTrunkModel->setWorld(trunkWorld);
			defaultTrunkModel->draw(device, gameTime);
			//}
		}
	}
	
	if (complexGrassEnabled) complexGrass->draw(device, gameTime);

	if (defaultGrassEnabled)
	{
		for(int i = 0; i < complexGrass->getDrawInstanceCount(); ++i)
		{
			Matrix grassWorld = Matrix::createScale(complexGrass->getScale()) * Matrix::createTranslation(complexGrass->getGrassPatchPositions()[i]);
			if(defaultDetailedEnabled)
			{
				defaultGrassDetailedModel->setWorld(grassWorld);
				defaultGrassDetailedModel->draw(device, gameTime);
			}
			else
			{
				defaultGrassModel->setWorld(grassWorld);
				defaultGrassModel->draw(device, gameTime);
			}
		}
	}

	deviceContext->OMSetBlendState(bsAlphaToCoverage, Vector4(0,0,0,0), 0xFFFFFFFF);
	if (gsGrassEnabled) gsGrass->draw(device, gameTime);

	deviceContext->Release();
}


void VegetationRendering::onLostDevice()
{
}


void VegetationRendering::onResetDevice(ID3D11Device* device)
{
	inputManager->onResetDevice();
}


void VegetationRendering::toggleWireframe()
{
	if(wireframeEnabled)
	{
		rsCullBack = rsCullBackSolid;
		rsCullNone = rsCullNoneSolid;
	}
	else
	{
		rsCullBack = rsCullBackWireframe;
		rsCullNone = rsCullNoneWireframe;
	}
	wireframeEnabled = !wireframeEnabled;
}


void VegetationRendering::toggleCursor()
{
	Mouse::setVisible(!Mouse::isVisible());
	Mouse::setPosition(Window::getCenter());
}


void VegetationRendering::startCapturing(std::string filename)
{
	captureFilename = filename;
	captureMode = true;
	currentCaptureView = -1;
	if (grassTests)
	{
		capturedInstances = grassTestInstanceCounts[instancesIndex];
		complexGrass->setDrawInstanceCount(capturedInstances);
		camera->visitCameraViews("GrassViews.txt", CAPTURE_TIME);
	}
	else
	{
		capturedInstances = treeTestInstanceCounts[instancesIndex];
		trunk->setDrawInstanceCount(capturedInstances);
		camera->visitCameraViews("TreeViews.txt", CAPTURE_TIME);
	}
	goToNextCaptureFile();
}


void VegetationRendering::stopCapturing()
{
	outputSum();
	captureSumFile << std::endl;
	captureMode = false;
	captureFile.close();
}


void VegetationRendering::goToNextCaptureFile()
{
	++currentCaptureView;
	currentCaptureTime = 0;
	captureFile.close();

	frameSum = 0;
	frameCount = 0;

	std::string filename;

	std::stringstream strConverter;
	strConverter << captureFilename << currentCaptureView << "_" << capturedInstances << "_" << capturedBorder << ".txt";
	filename = strConverter.str();

	captureFile.open(filename, std::ios::out);
}


void VegetationRendering::outputSum()
{
	captureFile << "Summe: " << frameSum << std::endl;
	captureFile << "Anzahl: " << frameCount << std::endl;
	double average = frameSum / frameCount;
	captureFile << "Durchschnitt : " << average << std::endl;

	captureSumFile << average << ";";
}