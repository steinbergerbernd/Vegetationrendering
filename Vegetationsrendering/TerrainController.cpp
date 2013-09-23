#include "TerrainController.h"
#include "Config.h"
#include "MathHelper.h"

#undef min
#undef max

TerrainController::TerrainController(const VegetationRendering& vegetationRendering) : vegetationRendering(vegetationRendering)
{
}


void TerrainController::init(ID3D11Device* device)
{
	width = Config::getValue<int>(ConfigKeys::terrainWidth);
	length = Config::getValue<int>(ConfigKeys::terrainLength);

	position = Vector3(width / -2.0f, 0, length / -2.0f);

	bumpiness = Config::getValue<float>(ConfigKeys::terrainBumpiness);
	quality = Config::getValue<float>(ConfigKeys::terrainQuality);

	D3DX11_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory( &loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO) );
	loadInfo.Usage = D3D11_USAGE_STAGING;
	loadInfo.Format = DXGI_FORMAT_FROM_FILE;
	loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_READ;

	ID3D11Texture2D *heightmapTexture, *normalmapTexture;

	D3DX11CreateTextureFromFile( device, L"Content/Textures/Terrain/Heightmap.png", &loadInfo, NULL, (ID3D11Resource**)(&heightmapTexture), NULL );
	D3DX11CreateTextureFromFile( device, L"Content/Textures/Terrain/Normalmap.png", &loadInfo, NULL, (ID3D11Resource**)(&normalmapTexture), NULL );

	heightmapTexture->GetDesc(&heightmapDesc);
	normalmapTexture->GetDesc(&normalmapDesc);

	std::vector<HeightmapFormat> hData = std::vector<HeightmapFormat>(heightmapDesc.Width * heightmapDesc.Height);
	std::vector<NormalmapFormat> nData = std::vector<NormalmapFormat>(normalmapDesc.Width * normalmapDesc.Height);

	ID3D11DeviceContext* deviceContext;
	device->GetImmediateContext(&deviceContext);

	D3D11_MAPPED_SUBRESOURCE heightmapData;
	deviceContext->Map(heightmapTexture, 0, D3D11_MAP_READ, 0, &heightmapData);
	memcpy((void*)&hData[0], heightmapData.pData, heightmapDesc.Width * heightmapDesc.Height * sizeof(HeightmapFormat));
	deviceContext->Unmap(heightmapTexture, 0);

	D3D11_MAPPED_SUBRESOURCE normalmapData;
	deviceContext->Map(normalmapTexture, 0, D3D11_MAP_READ, 0, &normalmapData);
	memcpy((void*)&nData[0], normalmapData.pData, normalmapDesc.Width * normalmapDesc.Height * sizeof(NormalmapFormat));
	deviceContext->Unmap(normalmapTexture, 0);

	heightData = HeightData(heightmapDesc.Width * heightmapDesc.Height);
	normalData = NormalData(normalmapDesc.Width * normalmapDesc.Height);

	std::transform(hData.begin(), hData.end(), heightData.begin(), &TerrainController::convertHeight);
	std::transform(nData.begin(), nData.end(), normalData.begin(), &TerrainController::convertNormal);
	
	heightmapTexture->Release();
	normalmapTexture->Release();
	deviceContext->Release();
}


float TerrainController::getHeight(float x, float z) const
{
	return get<float>(x, z, &TerrainController::getHeight);
}


Vector3 TerrainController::getNormal(float x, float z) const
{
	return get<Vector3>(x, z, &TerrainController::getNormal);
}


float TerrainController::getHeight(const TerrainPoint& p) const
{
	unsigned x = (unsigned)(((float)p.col / width) * heightmapDesc.Width);
	unsigned y = (unsigned)(((float)(length - p.row) / length) * heightmapDesc.Height);

	return getHeightmapData(x, y);
}


Vector3 TerrainController::getNormal(const TerrainPoint& p) const
{
	unsigned x = (unsigned)(((float)p.col / width) * normalmapDesc.Width);
	unsigned y = (unsigned)(((float)(length - p.row) / length) * normalmapDesc.Height);

	return getNormalmapData(x, y);
}


float TerrainController::getHeightmapData(unsigned x, unsigned y) const
{
	x = MathHelper::clamp(x, 0U, heightmapDesc.Width - 1U);
	y = MathHelper::clamp(y, 0U, heightmapDesc.Height - 1U);

	return (heightData[y * heightmapDesc.Width + x]) * bumpiness;
}


Vector3 TerrainController::getNormalmapData(unsigned x, unsigned y) const
{
	x = MathHelper::clamp(x, 0U, normalmapDesc.Width - 1U);
	y = MathHelper::clamp(y, 0U, normalmapDesc.Height - 1U);

	return normalData[y * normalmapDesc.Width + x];
}


float TerrainController::convertHeight(HeightmapFormat height)
{
	return (float)height / std::numeric_limits<HeightmapFormat>::max();
}


Vector3 TerrainController::convertNormal(NormalmapFormat normal)
{
	Vector3 v;

	v.x = (((unsigned char*)&normal)[2] / 255.0f) * 2.0f - 1.0f;
	v.y = (((unsigned char*)&normal)[3] / 255.0f) * 2.0f - 1.0f;
	v.z = (((unsigned char*)&normal)[1] / 255.0f) * 2.0f - 1.0f;

	v.z *= -1;

	return Vector3::normalize(v);
}


Vector3 TerrainController::getHeight(const Vector3& position) const
{
	return Vector3(position.x, getHeight(position.x, position.z), position.z);
}


Vector3 TerrainController::getNormal(const Vector3& position) const
{
	return getNormal(position.x, position.z);
}