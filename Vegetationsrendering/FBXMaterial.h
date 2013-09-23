#pragma once

#include <d3d9.h>
#include <D3D11.h>

struct FBXMaterial
{
	D3DMATERIAL9 material;
	ID3D11ShaderResourceView* texture;
};