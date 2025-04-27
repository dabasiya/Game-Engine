#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <stdio.h>
#include <iostream>

Shader::Shader(const char* vertexpath, const char* fragmentpath) {


	std::string vertexcodestring = ReadFile(vertexpath);
	std::string fragmentcodestring = ReadFile(fragmentpath);

	const char* vertexcode = vertexcodestring.c_str();
	const char* fragmentcode = fragmentcodestring.c_str();



	unsigned int vshader, fshader;

	vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vertexcode, nullptr);
	glCompileShader(vshader);

	GLint vertexshadercompiled;

	glGetShaderiv(vshader, GL_COMPILE_STATUS, &vertexshadercompiled);

	if (vertexshadercompiled == GL_FALSE) {
		printf("%s compile error!\n", vertexpath);
	}

	fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, &fragmentcode, nullptr);
	glCompileShader(fshader);

	GLint fragmentshadercompiled;

	glGetShaderiv(fshader, GL_COMPILE_STATUS, &fragmentshadercompiled);

	if (fragmentshadercompiled == GL_FALSE) {
		printf("%s compile error!\n", fragmentpath);
	}

	ID = glCreateProgram();
	glAttachShader(ID, vshader);
	glAttachShader(ID, fshader);
	glLinkProgram(ID);

	glDeleteShader(vshader);
	glDeleteShader(fshader);

}


std::string Shader::ReadFile(const char* path) {
	std::ifstream in(path, std::ios::binary);
	if (in) {
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return (contents);
	}
	throw(errno);
}

void Shader::Bind() {
	glUseProgram(ID);
}


int Shader::GetLocation(const char* name) {
	return glGetUniformLocation(ID, name);
}


void Shader::SetMat4(const char* name, const glm::mat4& value) {
	glUniformMatrix4fv(GetLocation(name), 1/*count*/, GL_FALSE/* bool transposed */, glm::value_ptr(value));
}

void Shader::SetVec3(const char* name, const glm::vec3& value) {
	glUniform3fv(GetLocation(name), 1/*count*/, glm::value_ptr(value));
}

void Shader::SetInt(const char* name, int value) {
	glUniform1i(GetLocation(name), value);
}
