#pragma once

struct GameTime
{
	GameTime() : elapsed(0), total(0) {}
	GameTime(float elapsed, double total) : elapsed(elapsed), total(total) {}

	float elapsed;
	double total;
};