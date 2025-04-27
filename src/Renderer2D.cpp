#include <glad/glad.h>
#include "Renderer2D.h"
#include <glm/gtc/matrix_transform.hpp>

#include <stdlib.h>
#include <iostream>
#include <string>

Texture* Renderer2D::textures[16];

// declare static variables of Renderer2D structure

struct Renderer2DState Renderer2D::state = {};


// set one Vertex stride size
// structure of Vertex stride
// positions                                     color                                textureindex

// position.x     position.y     position.z      color.r      color.g     color.b     textureindex      texturecoords.x      texturecoords.y
unsigned int Renderer2D::VertexStrideSize = 11 * sizeof(float);
// change for modify quadcount for batch renderer
unsigned int Renderer2D::maxquadcount = 10000;

unsigned int Renderer2D::vbo;
unsigned int Renderer2D::vao;
unsigned int Renderer2D::ebo;


// memory buffer for store data for rendering
struct Vertex* Renderer2D::memorybuffer;


// unit quad Vertex positions
static glm::vec4 quadVertexposition[4];

// unit triangle Vertex positions
static glm::vec4 triangleVertexposition[4];


// initalize vbo, vao, and ebo

void Renderer2D::Init() {

	quadVertexposition[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
	quadVertexposition[1] = { -0.5f, 0.5f, 0.0f, 1.0f };
	quadVertexposition[2] = { 0.5f, 0.5f, 0.0f, 1.0f };
	quadVertexposition[3] = { 0.5f, -0.5f, 0.0f, 1.0f };

	triangleVertexposition[0] = { 0.0f, -0.5f, 0.0f, 1.0f };
	triangleVertexposition[1] = { -0.5f, -0.5f, 0.0f, 1.0f };
	triangleVertexposition[2] = { 0.0f, 0.5f, 0.0f, 1.0f };
	triangleVertexposition[3] = { 0.5f, -0.5f, 0.0f, 1.0f };

	state.white_texture = new Texture(1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);


	unsigned int data = 0xffffffff;
	state.white_texture->SetData(&data);

	// calculate buffer size
	unsigned int Vertexbuffersize = VertexStrideSize * 4 * maxquadcount;


	// create Vertex buffer
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, Vertexbuffersize, nullptr, GL_STATIC_DRAW);



	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// set Vertex input layout
	// 0 for position
	// 1 for color
	// 2 for texture index
	// 3 for texture coordinates
        // 4 for entity id which is useful for mouse picking

	// position layout
	glVertexAttribPointer(0, 3/*component count xyz*/, GL_FLOAT/*element data type*/, GL_FALSE/*normalized*/, VertexStrideSize, 0);
	glEnableVertexAttribArray(0);

	// color layout
	glVertexAttribPointer(1, 4/* component count rgba*/, GL_FLOAT/*element data type*/, GL_FALSE/*normalized*/, VertexStrideSize, (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// texture index layout
	glVertexAttribPointer(2, 1/* component count textureindex*/, GL_FLOAT/*element data type*/, GL_FALSE/*normalized*/, VertexStrideSize, (void*)(7 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// for texture coordinates
	glVertexAttribPointer(3, 2/* component count texture coordinates*/, GL_FLOAT/* element data types*/, GL_FALSE/*normalized*/, VertexStrideSize, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);


        // for entity id
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, VertexStrideSize, (void*)(10*sizeof(float)));
        glEnableVertexAttribArray(4);


	// calculate index buffer size
	const unsigned int indexarraysize = maxquadcount * 6;

	//unsigned int indices[indexarraysize];
	unsigned int indices[60000];

	unsigned int offset = 0;

	// populate indices array
	for (unsigned int i = 0; i < indexarraysize / 6; i++) {
		unsigned a = i * 6;

		indices[a] = offset;
		indices[a + 1] = offset + 1;
		indices[a + 2] = offset + 2;
		indices[a + 3] = offset;
		indices[a + 4] = offset + 2;
		indices[a + 5] = offset + 3;

		offset += 4;
	}



	// create index buffer
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);



	memorybuffer = (struct Vertex*)malloc(Vertexbuffersize);

	state.bufferbase = state.bufferptr = memorybuffer;
}


void Renderer2D::BeginScene(const glm::mat4& cameraprojectionview) {

	state.shadercontext->Bind();
	state.white_texture->Bind(0);
	// set view and projection value
	state.shadercontext->SetMat4("projectionview", cameraprojectionview);

	state.bufferptr = state.bufferbase;

	for (int i = 0; i < 16; i++) {
		if (textures[i] != nullptr) {
			textures[i]->Bind(i);
			std::string varname = std::string("tex[") + std::to_string(i) + std::string("]");
			state.shadercontext->SetInt(varname.c_str(), i);
		}
	}
}


void Renderer2D::EndScene() {
	Flush();
}


// for change shader
void Renderer2D::UseShader(Shader* a_shader) {
	state.shadercontext = a_shader;
}


void Renderer2D::SetTexture(Texture* textureid, unsigned int id) {
	textures[id] = textureid;
}


void Renderer2D::Flush() {


	unsigned int size = (unsigned int)((unsigned char*)state.bufferptr - (unsigned char*)state.bufferbase);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, memorybuffer);

	glDrawElements(GL_TRIANGLES, state.indicespointer, GL_UNSIGNED_INT, nullptr);

	// reset Renderer2D state
	state.indicespointer = 0;

	state.bufferptr = state.bufferbase;
}

void Renderer2D::DrawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color) {
	DrawLine({ p1.x, p1.y, 0.0f }, { p2.x, p2.y, 0.0f }, color);
}

void Renderer2D::DrawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color) {
	if (state.indicespointer >= maxquadcount * 6) {
		Flush();
	}

	state.bufferptr->position = glm::vec4(p1.x, p1.y + 0.01f, p1.z, 1.0f);
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr->texcoords = glm::vec2(0.0f, 0.0f);
	state.bufferptr++;

	state.bufferptr->position = glm::vec4(p1.x, p1.y, p1.z, 1.0f);
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr->texcoords = glm::vec2(0.0f, 1.0f);
	state.bufferptr++;

	state.bufferptr->position = glm::vec4(p2.x, p2.y, p2.z, 1.0f);
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr->texcoords = glm::vec2(1.0f, 1.0f);
	state.bufferptr++;

	state.bufferptr->position = glm::vec4(p2.x, p2.y + 0.01f, p2.z, 1.0f);
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr->texcoords = glm::vec2(1.0f, 0.0f);
	state.bufferptr++;

	state.indicespointer += 6;
}

void Renderer2D::DrawQuad(const glm::mat4& translation, const glm::vec4& color) {

	// check if Vertexpointer reached limit then first apply drawcall and reset buffer
	if (state.indicespointer >= maxquadcount * 6) {
		Flush();
	}

	state.bufferptr->position = translation * quadVertexposition[0];
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr->texcoords = glm::vec2(0.0f, 0.0f);
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[1];
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr->texcoords = glm::vec2(0.0f, 1.0f);
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[2];
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr->texcoords = glm::vec2(1.0f, 1.0f);
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[3];
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr->texcoords = glm::vec2(1.0f, 0.0f);
	state.bufferptr++;

	state.indicespointer += 6;
}

void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& scale, const glm::vec4& color, glm::vec2 ref) {
	DrawQuad({ position.x, position.y, 0.0f }, scale, color, ref);
}

void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color, glm::vec2 ref) {
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

	DrawQuad(translation, color);
}


void Renderer2D::Release() {
	// delete textures
	for (uint32_t i = 2; i < 16; i++) {
		if (textures[i]) {
			delete textures[i];
		}
	}
	free(memorybuffer);
	delete state.white_texture;
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& scale, float angle, const glm::vec4& color, glm::vec2 ref) {
	DrawRotatedQuad({ position.x, position.y, 0.0f }, scale, angle, color, ref);
}


void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& scale, float angle, const glm::vec4& color, glm::vec2 ref) {

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

	DrawQuad(translation, color);
}


void Renderer2D::DrawTriangle(const glm::mat4& translation, const glm::vec4& color) {

	if (state.indicespointer > maxquadcount * 6) {
		Flush();
	}

	state.bufferptr->position = translation * triangleVertexposition[0];
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr++;

	state.bufferptr->position = translation * triangleVertexposition[1];
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr++;

	state.bufferptr->position = translation * triangleVertexposition[2];
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr++;

	state.bufferptr->position = translation * triangleVertexposition[3];
	state.bufferptr->color = color;
	state.bufferptr->textureindex = 0.0f;
	state.bufferptr++;

	state.indicespointer += 6;
}


void Renderer2D::DrawTriangle(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color, glm::vec2 ref) {
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

	DrawTriangle(translation, color);
}


void Renderer2D::DrawTriangle(const glm::vec2& position, const glm::vec2& scale, const glm::vec4& color, glm::vec2 ref) {
	DrawTriangle({ position.x, position.y, 0.0f }, scale, color, ref);
}

void Renderer2D::DrawRotatedTriangle(const glm::vec3& position, const glm::vec2& scale, float angle, const glm::vec4& color, glm::vec2 ref) {
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f }) *
		glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

	DrawTriangle(translation, color);
}



void Renderer2D::DrawRotatedTriangle(const glm::vec2& position, const glm::vec2& scale, float angle, const glm::vec4& color, glm::vec2 ref) {
	DrawRotatedTriangle({ position.x, position.y, 0.0f }, scale, angle, color, ref);
}



void Renderer2D::DrawQuad(const glm::mat4& translation, const float textureindex) {



	// check if Vertexpointer reached limit then first apply drawcall and reset buffer
	if (state.indicespointer >= maxquadcount * 6) {
		Flush();
	}

	state.bufferptr->position = translation * quadVertexposition[0];
	state.bufferptr->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	state.bufferptr->textureindex = textureindex;
	state.bufferptr->texcoords = glm::vec2(0.0f, 0.0f);
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[1];
	state.bufferptr->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	state.bufferptr->textureindex = textureindex;
	state.bufferptr->texcoords = glm::vec2(0.0f, 1.0f);
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[2];
	state.bufferptr->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	state.bufferptr->textureindex = textureindex;
	state.bufferptr->texcoords = glm::vec2(1.0f, 1.0f);
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[3];
	state.bufferptr->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	state.bufferptr->textureindex = textureindex;
	state.bufferptr->texcoords = glm::vec2(1.0f, 0.0f);
	state.bufferptr++;

	state.indicespointer += 6;


}


void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, const float textureindex, glm::vec2 ref) {

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f }) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

	DrawQuad(translation, textureindex);

}
void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& scale, const float textureindex, glm::vec2 ref) {
	DrawQuad({ position.x, position.y, 0.0f }, scale, textureindex, ref);
}



void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& scale, float angle, const float textureindex, glm::vec2 ref) {


	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

	DrawQuad(translation, textureindex);


}
void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& scale, float angle, const float textureindex, glm::vec2 ref) {
	DrawRotatedQuad({ position.x, position.y, 0.0f }, scale, angle, textureindex, ref);
}


void Renderer2D::DrawQuad(const glm::mat4& translation, const SubTexture& a_SubTexture, float opacity, const glm::vec4& a_color) {

	// check if Vertexpointer reached limit then first apply drawcall and reset buffer
	if (state.indicespointer >= maxquadcount * 6) {
		Flush();
	}

	state.bufferptr->position = translation * quadVertexposition[0];
	state.bufferptr->color = a_color;
	state.bufferptr->color.a *= opacity;
	state.bufferptr->textureindex = (float)a_SubTexture.index;
	state.bufferptr->texcoords = { a_SubTexture.coords1.x, a_SubTexture.coords2.y };
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[1];
	state.bufferptr->color = a_color;
	state.bufferptr->color.a *= opacity;
	state.bufferptr->textureindex = (float)a_SubTexture.index;
	state.bufferptr->texcoords = a_SubTexture.coords1;
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[2];
	state.bufferptr->color = a_color;
	state.bufferptr->color.a *= opacity;
	state.bufferptr->textureindex = (float)a_SubTexture.index;
	state.bufferptr->texcoords = { a_SubTexture.coords2.x, a_SubTexture.coords1.y };
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[3];
	state.bufferptr->color = a_color;
	state.bufferptr->color.a *= opacity;
	state.bufferptr->textureindex = (float)a_SubTexture.index;
	state.bufferptr->texcoords = a_SubTexture.coords2;
	state.bufferptr++;

	state.indicespointer += 6;

}
void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& scale, const SubTexture& a_SubTexture, const glm::vec4& a_color, glm::vec2 ref) {
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::scale(glm::mat4(1.0f), { scale.x, scale.y, 1.0f }) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

	DrawQuad(translation, a_SubTexture, 1.0f, a_color);
}


void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& scale, const SubTexture& a_SubTexture, const glm::vec4& a_color, glm::vec2 ref) {
	DrawQuad({ position.x, position.y, 0.0f }, scale, a_SubTexture, a_color, ref);
}

void Renderer2D::DrawRotatedQuad(const glm::vec2& pos, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, float opacity, const glm::vec2 ref) {
	DrawRotatedQuad(glm::vec3(pos, 0.0f), scale, angle, a_subtexture, opacity, ref);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& pos, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, float opacity, const glm::vec2 ref) {
	
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos) *
		glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

	DrawQuad(translation, a_subtexture, opacity);
}

void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, const glm::vec4& color, glm::vec2 ref) {
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

	DrawQuad(translation, a_subtexture, 1.0f, color);
}


void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, const glm::vec4& color, glm::vec2 ref) {
	DrawRotatedQuad(glm::vec3(position.x, position.y, 0.0f), scale, angle, a_subtexture, color);
}



void Renderer2D::DrawEntity(float id, const glm::vec2& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, glm::vec2 ref) {
  DrawEntity(id, glm::vec3(position, 0.0f), scale, angle, a_subtexture, ref);
}


void Renderer2D::DrawEntity(float id, const glm::vec3& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, glm::vec2 ref) {
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), position) *
		glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)) *
		glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, 1.0f)) *
		glm::translate(glm::mat4(1.0f), { ref.x, ref.y, 0.0f });

  DrawQuad(id, translation, a_subtexture);
}


void Renderer2D::DrawQuad(float id, const glm::mat4& translation, const SubTexture& a_subtexture) {
  // check if Vertexpointer reached limit then first apply drawcall and reset buffer
	if (state.indicespointer >= maxquadcount * 6) {
		Flush();
	}

	state.bufferptr->position = translation * quadVertexposition[0];
    state.bufferptr->entityid = id;
	state.bufferptr->textureindex = (float)a_subtexture.index;
	state.bufferptr->color = glm::vec4(1.0f);
	state.bufferptr->texcoords = { a_subtexture.coords1.x, a_subtexture.coords2.y };
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[1];
    state.bufferptr->entityid = id;
	state.bufferptr->textureindex = (float)a_subtexture.index;
	state.bufferptr->color = glm::vec4(1.0f);
	state.bufferptr->texcoords = a_subtexture.coords1;
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[2];
    state.bufferptr->entityid = id;
	state.bufferptr->textureindex = (float)a_subtexture.index;
	state.bufferptr->color = glm::vec4(1.0f);
	state.bufferptr->texcoords = { a_subtexture.coords2.x, a_subtexture.coords1.y };
	state.bufferptr++;

	state.bufferptr->position = translation * quadVertexposition[3];
    state.bufferptr->entityid = id;
	state.bufferptr->textureindex = (float)a_subtexture.index;
	state.bufferptr->color = glm::vec4(1.0f);
	state.bufferptr->texcoords = a_subtexture.coords2;
	state.bufferptr++;

	state.indicespointer += 6;

}
