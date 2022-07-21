#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH  600
#define HEIGHT 400
#define TITLE  "Hello, OpenGL!"

#define LOG(...) do { \
	fprintf(stderr, "%s:%s():%d: ", __FILE__, __func__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	fputc('\n', stderr); \
} while(0)

#define FAIL(...) do { LOG("Error: " __VA_ARGS__); exit(1); } while(0)

#define ASSERT(cond, ...) if (!(cond)) FAIL(__VA_ARGS__)

#define GL() do { \
	GLenum err = glGetError(); \
	ASSERT(err == GL_NO_ERROR, "OpenGL error: %d", err); \
} while(0)

static GLfloat vertices[] = {
	0.5, 0.5, 0.0,    1.0, 0.0, 0.0,
	0.5, -0.5, 0.0,   0.0, 1.0, 0.0,
	-0.5, 0.5, 0.0,   0.0, 0.0, 1.0,
	-0.5, -0.5, 0.0,  0.0, 0.0, 0.0,
};

static GLuint indices[] = {
	0, 1, 2,
	1, 2, 3,
};

static const char* vert_code =
	"#version 330 core\n"

	"layout (location = 0) in vec3 pos;"
	"layout (location = 1) in vec3 in_color;"
	"out vec3 vert_color;"

	"void main() {"
		"gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);"
		"vert_color = in_color;"
	"}"
;

static const char* frag_code =
	"#version 330 core\n"

	"out vec4 color;"
	"in vec3 vert_color;"

	"void main() {"
		"color = vec4(vert_color.x, vert_color.y, vert_color.z, 1.0);"
	"}"
;

static void on_resize(GLFWwindow* win, int width, int height) {
	(void)win;
	glViewport(0, 0, width, height);
}

static GLFWwindow* start(int width, int height, const char* title) {
	ASSERT(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	GLFWwindow* win = glfwCreateWindow(width, height, title, NULL, NULL);
	ASSERT(win, "Failed to create GLFW window");
	glfwMakeContextCurrent(win);
	glfwSetFramebufferSizeCallback(win, on_resize);

	ASSERT(glewInit() == GLEW_OK, "Failed to initialize GLEW");

	return win;
}

static GLuint single_shader(GLuint type, const char* code) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &code, NULL);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char err[512];
		glGetShaderInfoLog(shader, 512, NULL, err);
		FAIL("Failed to compile %s shader: %s", (type == GL_VERTEX_SHADER ? "vertex" : "fragment"), err);
	}

	return shader;
}

static GLuint shader_program(const char* vert_code, const char* frag_code) {
	GLuint vert_shader = single_shader(GL_VERTEX_SHADER, vert_code);
	GLuint frag_shader = single_shader(GL_FRAGMENT_SHADER, frag_code);
	GLuint program = glCreateProgram();
	glAttachShader(program, vert_shader);
	glAttachShader(program, frag_shader);
	glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char err[512];
		glGetProgramInfoLog(program, 512, NULL, err);
		FAIL("Failed to link shader program: %s", err);
	}

	return program;
}

static void setup_vertex_objects(
	GLfloat* vertices, int v_size, GLuint* indices, int i_size,
	GLuint* vao, GLuint* vbo, GLuint* ebo
) {
	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);
	glGenBuffers(1, ebo);

	glBindVertexArray(*vao);

	glBindBuffer(GL_ARRAY_BUFFER, *vbo);
	glBufferData(GL_ARRAY_BUFFER, v_size, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, i_size, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void draw(GLFWwindow* win, GLuint shader_prog, GLuint vao) {
	glClearColor(0.0, 0.2, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shader_prog);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

	glfwSwapBuffers(win);
}

void export(const char* filename) {
	uint8_t buf[WIDTH * HEIGHT * 3];
	glReadBuffer(GL_FRONT); GL();
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, buf); GL();
	stbi_write_png(filename, WIDTH, HEIGHT, 3, buf, WIDTH * 3);
}

int main() {
	GLFWwindow* win = start(WIDTH, HEIGHT, TITLE);
	GLuint shader_prog = shader_program(vert_code, frag_code);
	GLuint vao, vbo, ebo;
	setup_vertex_objects(vertices, sizeof(vertices), indices, sizeof(indices), &vao, &vbo, &ebo);

	draw(win, shader_prog, vao);
	export("out.png");

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteProgram(shader_prog);

	glfwTerminate();
	return 0;
}
