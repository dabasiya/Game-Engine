#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <stdio.h>
#include <iostream>
#include <vector>

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

Shader::Shader(const char* computeshaderpath) {
	std::string computecodestring = ReadFile(computeshaderpath);

	const char* computecode = computecodestring.c_str();

	unsigned int cshader;

	cshader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(cshader, 1, &computecode, nullptr);
	glCompileShader(cshader);

	GLint compiled = GL_FALSE;
	glGetShaderiv(cshader, GL_COMPILE_STATUS, &compiled);

	if (compiled == GL_FALSE) {
		GLint logLength = 0;
		glGetShaderiv(cshader, GL_INFO_LOG_LENGTH, &logLength);

		std::vector<GLchar> log(logLength);
		glGetShaderInfoLog(cshader, logLength, &logLength, log.data());

		printf("Compute shader compile error (%s):\n%s\n", computeshaderpath, log.data());
	}

	ID = glCreateProgram();
	glAttachShader(ID, cshader);
	glLinkProgram(ID);

	glDeleteShader(cshader);
}


Shader::Shader(const char* vertexpath, const char* geometrypath, const char* fragmentpath) {
	std::string vertexcodestring = ReadFile(vertexpath);
	std::string fragmentcodestring = ReadFile(fragmentpath);
	std::string geometrycodestring = ReadFile(geometrypath);

	const char* vertexcode = vertexcodestring.c_str();
	const char* fragmentcode = fragmentcodestring.c_str();
	const char* geometrycode = geometrycodestring.c_str();


	unsigned int vshader, fshader, gshader;

	vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vertexcode, nullptr);
	glCompileShader(vshader);

	GLint vertexshadercompiled;

	glGetShaderiv(vshader, GL_COMPILE_STATUS, &vertexshadercompiled);

	if (vertexshadercompiled == GL_FALSE) {
		printf("%s compile error!\n", vertexpath);
	}

	gshader = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(gshader, 1, &geometrycode, nullptr);
	glCompileShader(gshader);

	GLint geometryshadercompiled;

	glGetShaderiv(gshader, GL_COMPILE_STATUS, &geometryshadercompiled);

	if (geometryshadercompiled == GL_FALSE) {
		printf("%s compile error!\n", geometrypath);
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
	glAttachShader(ID, gshader);
	glLinkProgram(ID);


	glDeleteShader(vshader);
	glDeleteShader(gshader);
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

void Shader::SetVec4(const char* name, const glm::vec4& value) {
	glUniform4fv(GetLocation(name), 1, glm::value_ptr(value));
}

void Shader::SetInt(const char* name, int value) {
	int loc = GetLocation(name);
	glUniform1i(loc, value);
}

void Shader::SetFloat(const char* name, float value) {
	glUniform1f(GetLocation(name), value);
}

void Shader::Delete() {
	glDeleteProgram(ID);
}

Shader::~Shader() {
	Delete();
}


void Shader::SetIntArray(const char* name, int* values, unsigned int count) {
	int loc = GetLocation(name);
	std::cout << loc << std::endl;
	glUniform1iv(loc, count, values);
}

int Shader::GetUniformBlockIndex(const char* name) {
	return glGetUniformBlockIndex(ID, name);
}