#pragma once

#include "EntityScript.h"
#include <Texture.h>
#include "Renderer2D.h"
#include <Window.h>

#include <PerlinNoise.h>

struct fluid : EntityScript {

	unsigned int last_width = Window::Width;
	unsigned int last_height = Window::Height;

	Texture* texture;

	float time = 0.0f;

	float noisescale = 0.2f;

	unsigned char* data;

	void Start() override {
		texture = new Texture(last_width, last_height, GL_RGB8, GL_RGB, 0);
		Renderer2D::SetTexture(texture, 5);

		data = new unsigned char[last_height * last_width * 3];
	}

	void Update(float ts) override {
		if (Window::Width != last_width || Window::Height != last_height) {
			RecreateTexture();
			Renderer2D::SetTexture(texture, 5);

			delete data;
			data = new unsigned char[Window::Width * Window::Height * 3];
		}

		fluidflow();

		time += ts;
	}

	void RecreateTexture() {
		delete texture;

		texture = new Texture(Window::Width, Window::Height, GL_RGB8, GL_RGB, 0);

		last_height = Window::Height;
		last_width = Window::Width;
	}

	void fluidflow() {
		for (int y = 0; y < last_height; ++y) {
			for (int x = 0; x < last_width; ++x) {
				float nx = x * noisescale;
				float ny = y * noisescale;
				float velocity = PerlinNoise::Noise3D(x, y, time * noisescale); // Time-based noise evolution

				int index = (y * last_width + x) * 3;
				data[index] = (0.5f + 0.5f * velocity) * 255.0f;  // Red
				data[index + 1] = (0.5f - 0.5f * velocity) * 255.0f; // Green
				data[index + 2] = (1.0f - data[index]) * 255.0f; // Blue
			}
		}

		texture->Bind();
		texture->SetData(data, 0, 0, last_width, last_height);
	}
};