#pragma once

#include "Types.h"
#include "TerrainPoint.h"

class TerrainView;
class TerrainController;

class TerrainBlock
{
	typedef std::vector<float> ErrorCollection;

public:
	TerrainBlock();

	void init(const TerrainView& view, const TerrainPoint& offset);
	void update(const TerrainView& view, const Vector3& cameraPos);

	unsigned getLevelOfDetail() const;
	float getBlending() const;
	const TerrainPoint& getOffset() const;

	bool isOverWater() const { return overWater; }
	bool isUnderWater() const { return underWater; }

private:
	float getError(const TerrainController& terrain, const TerrainPoint& center, const TerrainPoint& neighbor1, const TerrainPoint& neighbor2);

	Vector3 center;
	TerrainPoint offset;
	unsigned levelOfDetail;
	float blending;
	ErrorCollection maxErrorSquared;
	bool underWater;
	bool overWater;
};