#pragma once

#include "Camera.h"
#include "Vector3.h"

struct EffectCamera
{
	EffectCamera(const Camera& c) : position(c.getPosition()) {}

	Vector3 position;
};