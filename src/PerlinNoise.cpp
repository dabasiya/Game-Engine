#include "PerlinNoise.h"
#include <vector>
#include <random>

int PerlinNoise::perm[512];

void PerlinNoise::initperm() {
	std::vector<int> p(256);
	for (int i = 0; i < 256; i++) p[i] = i;

	std::shuffle(p.begin(), p.end(), std::mt19937{ std::random_device{}() });

	for (int i = 0; i < 256; i++) {
		perm[i] = perm[i + 256] = p[i];
	}
}

float PerlinNoise::fade(float t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::lerp(float a, float b, float t) {
	return a + (b - a) * t;
}

float PerlinNoise::grad(int hash, float x, float y, float z) {
	int h = hash & 15;
	float u = h < 8 ? x : y;
	float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float PerlinNoise::Noise3D(float x, float y, float z) {
	// Get unit cube corner coordinates
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;
	int Z = (int)floor(z) & 255;

	// Relative position inside cube
	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	// Compute fade curves
	float u = fade(x);
	float v = fade(y);
	float w = fade(z);

	// Hash coordinates to get gradient indices
	int A = perm[X] + Y, AA = perm[A] + Z, AB = perm[A + 1] + Z;
	int B = perm[X + 1] + Y, BA = perm[B] + Z, BB = perm[B + 1] + Z;

	// Compute dot products with gradient
	float g1 = grad(perm[AA], x, y, z);
	float g2 = grad(perm[BA], x - 1, y, z);
	float g3 = grad(perm[AB], x, y - 1, z);
	float g4 = grad(perm[BB], x - 1, y - 1, z);
	float g5 = grad(perm[AA + 1], x, y, z - 1);
	float g6 = grad(perm[BA + 1], x - 1, y, z - 1);
	float g7 = grad(perm[AB + 1], x, y - 1, z - 1);
	float g8 = grad(perm[BB + 1], x - 1, y - 1, z - 1);

	// Interpolate
	float lerpX1 = lerp(g1, g2, u);
	float lerpX2 = lerp(g3, g4, u);
	float lerpX3 = lerp(g5, g6, u);
	float lerpX4 = lerp(g7, g8, u);

	float lerpY1 = lerp(lerpX1, lerpX2, v);
	float lerpY2 = lerp(lerpX3, lerpX4, v);

	return lerp(lerpY1, lerpY2, w);
}