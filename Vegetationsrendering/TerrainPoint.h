#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

struct TerrainPoint
{
	TerrainPoint() : row(0), col(0) {}
	TerrainPoint(unsigned row, unsigned col) : row(row), col(col) {}

	operator Vector2() const { return Vector2((float)col, (float)row); }

	unsigned col;
	unsigned row;
};