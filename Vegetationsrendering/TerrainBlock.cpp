#include "TerrainView.h"
#include "TerrainBlock.h"

#undef max
#undef min

TerrainBlock::TerrainBlock() : levelOfDetail(0), blending(0), underWater(false), overWater(false)
{
}


void TerrainBlock::init(const TerrainView& view, const TerrainPoint& offset)
{
	float maxError = 0.0f;

	unsigned blockSize = view.getPatchSize();
	const TerrainController& terrain = view.getTerrainController();

	maxErrorSquared = ErrorCollection(view.getDetailLevels(), 0.0f);
	this->offset = offset;

	center = Vector3(blockSize / 2.0f, 0, blockSize / 2.0f) + Vector3((float)offset.col, 0, (float)offset.row);

	center += terrain.getPosition();

	TerrainPoint p[3];

	for (unsigned row = 0; row < blockSize; ++row)
		for (unsigned col = 0; col < blockSize; ++col)
		{
			int r = row % 2;
			int c = col % 2;

			if (!r && !c)
				continue;

			p[0] = TerrainPoint(row, col);
			p[1] = TerrainPoint(row + r, col + c);
			p[2] = TerrainPoint(row - r, col - c);

			for (int i = 0; i < 3; ++i)
			{
				p[i].row += offset.row;
				p[i].col += offset.col;
			}

			maxError = std::max(maxError, getError(terrain, p[0], p[1], p[2]));

			if (!underWater && terrain.getHeight(p[0]) < 0.0f)
				underWater = true;

			if (!overWater && terrain.getHeight(p[0]) > 0.0f)
				overWater = true;
		}

	maxError = std::min(maxError, 0.1f);

	for (unsigned i = 1; i < maxErrorSquared.size(); ++i)
		maxErrorSquared[i] = std::pow(maxError * MathHelper::pow2(i - 1), 2);
}


void TerrainBlock::update(const TerrainView& view, const Vector3& cameraPos)
{
	const TerrainController& controller = view.getTerrainController();

	unsigned blockSize = view.getPatchSize();

	float distanceSquared = (center - cameraPos).lengthSquared();
	float qualitySquared = std::pow(controller.getQuality(), 2);

	levelOfDetail = 0;

	for (unsigned i = 1; i < view.getDetailLevels() && distanceSquared > maxErrorSquared[i] * qualitySquared; ++i)
		levelOfDetail = i;

	if (levelOfDetail < view.getDetailLevels() - 1)
	{
		float D0 = maxErrorSquared[levelOfDetail] * qualitySquared;
		float D1 = maxErrorSquared[levelOfDetail + 1] * qualitySquared;

		blending = (distanceSquared - D0) / (D1 - D0);
	}
}


float TerrainBlock::getError(const TerrainController& t, const TerrainPoint& c, const TerrainPoint& n1, const TerrainPoint& n2)
{
	return std::abs(t.getHeight(c) - (t.getHeight(n1) + t.getHeight(n2)) / 2.0f);
}


unsigned TerrainBlock::getLevelOfDetail() const
{
	return levelOfDetail;
}


const TerrainPoint& TerrainBlock::getOffset() const
{
	return offset;
}


float TerrainBlock::getBlending() const
{
	return blending;
}