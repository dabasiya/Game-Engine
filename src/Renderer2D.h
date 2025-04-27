#pragma once

#include "subtexture.h"
#include "texture.h"
#include "shader.h"
#include <glm/glm.hpp>



struct Vertex {
	glm::vec3 position;
	glm::vec4 color;
	float textureindex;
	glm::vec2 texcoords;
	float entityid;
};


struct Renderer2DState {

	unsigned int indicespointer = 0;
	struct Shader* shadercontext;
	Texture* white_texture;

	Vertex* bufferbase;
	Vertex* bufferptr;
};

struct Renderer2D {

	// one vertex stride size
	static unsigned int VertexStrideSize;

	// it is used for this batchrendererstate
	static struct Renderer2DState state;

	// Number of quads that can be stored in this batch renderer
	static unsigned int maxquadcount;


	// buffers for store data that needed for rendering
	static unsigned int vbo, vao, ebo;

	// stores data for rendering
	static struct Vertex* memorybuffer;

	// this function allocate memory for vertex buffer, and set element values in indices buffer
	static void Init();


	static void UseShader(Shader* a_shader);

	// this is used for render new scene
	// set projectionview matrix value in shader
	// in shader uniform variable name should be "projectionview"
	static void BeginScene(const glm::mat4& cameraprojectionview);

	// this should be called after scene renderig is done
	// so rendering is done of exist data on batch
	static void EndScene();

	// it draw vertices
	// it uses when batch is full then this function used for drawcall and reset batch state data
	static void Flush();

	// texture array

	static Texture* textures[];

	static void SetTexture(Texture* textureid, unsigned int id);


	static void DrawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color);
	static void DrawLine(const glm::vec3& p1, const glm::vec3& p2, const glm::vec4& color);


  // draw quad with entity id
  static void DrawQuad(float id, const glm::mat4& ttranslation, const SubTexture& a_subtexture);

	// functions for draw quads
	static void DrawQuad(const glm::mat4& translation, const glm::vec4& color);
	static void DrawQuad(const glm::vec2& position, const glm::vec2& scale, const glm::vec4& color, glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color, glm::vec2 ref = { 0.0f, 0.0f });


	static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& scale, float angle, const glm::vec4& color, glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& scale, float angle, const glm::vec4& color, glm::vec2 ref = { 0.0f, 0.0f });

	// for drawing textured quad 
	static void DrawQuad(const glm::mat4& translation, const float textureindex);
	static void DrawQuad(const glm::vec2& position, const glm::vec2& scale, const float textureindex, glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, const float textureindex, glm::vec2 ref = { 0.0f, 0.0f });


	// for drawing with subtexture
	static void DrawQuad(const glm::mat4& translation, const SubTexture& a_subtexture, float opacity = 1.0f, const glm::vec4& a_color = glm::vec4(1.0f));
	static void DrawQuad(const glm::vec2& position, const glm::vec2& scale, const SubTexture& a_subtexture, const glm::vec4& a_color = glm::vec4(1.0f), glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawQuad(const glm::vec3& position, const glm::vec2& scale, const SubTexture& a_subtexture, const glm::vec4& a_color = glm::vec4(1.0f), glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, float opacity, glm::vec2 ref = glm::vec2(0.0f, 0.0f));
	static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, float opacity, glm::vec2 ref = glm::vec2(0.0f, 0.0f));

	// for drawing with subtexture * color
	static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, const glm::vec4& color, glm::vec2 ref = glm::vec2(0.0f));
	static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, const glm::vec4& color, glm::vec2 ref = glm::vec2(0.0f));

	static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& scale, float angle, const float textureindex, glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& scale, float angle, const float textureindex, glm::vec2 ref = { 0.0f, 0.0f });


	// functions for draw triangles
	static void DrawTriangle(const glm::mat4& translation, const glm::vec4& color);
	static void DrawTriangle(const glm::vec2& position, const glm::vec2& scale, const glm::vec4& color, glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawTriangle(const glm::vec3& position, const glm::vec2& scale, const glm::vec4& color, glm::vec2 ref = { 0.0f, 0.0f });

	static void DrawRotatedTriangle(const glm::vec2& position, const glm::vec2& scale, float angle, const glm::vec4& color, glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawRotatedTriangle(const glm::vec3& position, const glm::vec2& scale, float angle, const glm::vec4& color, glm::vec2 ref = { 0.0f, 0.0f });




  // for draw entity in framebuffer for
	static void DrawEntity(float id, const glm::vec3& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, glm::vec2 ref = { 0.0f, 0.0f });
	static void DrawEntity(float id, const glm::vec2& position, const glm::vec2& scale, float angle, const SubTexture& a_subtexture, glm::vec2 ref = {0.0f, 0.0f});
  
  // for set vertexbuffer data
	static void SetVertexBufferData(const void* data, unsigned int size);

	// it is used for deallocate memory of allocated data
	static void Release();
};
