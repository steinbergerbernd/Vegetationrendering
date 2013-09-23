#pragma once

#include <d3d9.h>

#include "Types.h"

struct Light : D3DLIGHT9
{
	Light();
	Light(const Vector3& direction, const Vector4& ambient = Vector4::zero, const Vector4& diffuse = Vector4::one, const Vector4& specular = Vector4::one);
};