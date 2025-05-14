#pragma once


struct PerlinNoise {
	static int perm[512];

	static void initperm();

	static float fade(float t);

	static float lerp(float a, float b, float t);

	static float Noise3D(float x, float y, float z);

	static float grad(int hash, float x, float y, float z);
};