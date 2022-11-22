#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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
	// 0.5, 0.5, 0.0,    1.0, 0.0, 0.0,  1.0, 1.0,
	// 0.5, -0.5, 0.0,   0.0, 1.0, 0.0,  1.0, 0.0,
	// -0.5, 0.5, 0.0,   0.0, 0.0, 0.0,  0.0, 1.0,
	// -0.5, -0.5, 0.0,  0.0, 0.0, 1.0,  0.0, 0.0,

	1.0, 1.0, 0.0,    1.0, 0.0, 0.0,  1.0, 1.0,
	1.0, -1.0, 0.0,   0.0, 1.0, 0.0,  1.0, 0.0,
	-1.0, 1.0, 0.0,   0.0, 0.0, 0.0,  0.0, 1.0,
	-1.0, -1.0, 0.0,  0.0, 0.0, 1.0,  0.0, 0.0,
};

static GLuint indices[] = {
	0, 1, 2, // Triangle 1
	1, 2, 3, // Triangle 2
};

static const char* vert_code =
	"#version 330 core\n"

	"layout (location = 0) in vec3 pos;"
	"layout (location = 1) in vec3 in_color;"
	"layout (location = 2) in vec2 in_tex_coord;"

	"out vec3 vert_color;"
	"out vec2 tex_coord;"

	"void main() {"
		"gl_Position = vec4(pos, 1.0);"
		"vert_color = in_color;"
		"tex_coord = in_tex_coord;"
	"}"
;

static const char* frag_code =
	"#version 330 core\n"

	"out vec4 color;"

	"in vec3 vert_color;"
	"in vec2 tex_coord;"

	"uniform sampler2D texture1;"

	"void main() {"
		"color = texture(texture1, tex_coord);"
	"}"
;

static void on_resize(GLFWwindow* win, int width, int height) {
	(void)win;
	// glViewport(0, 0, width, height);
	glViewport(0, 0, 600, 600);
}

static void on_key(GLFWwindow* win, int key, int scancode, int action, int mode) {
	(void)scancode;
	(void)mode;

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		glfwSetWindowShouldClose(win, GLFW_TRUE);
}

static GLFWwindow* start(int width, int height, const char* title) {
	ASSERT(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	// GLFWmonitor* mon = glfwGetPrimaryMonitor();
	// const GLFWvidmode* mode = glfwGetVideoMode(mon);
	// GLFWwindow* win = glfwCreateWindow(mode->width, mode->height, title, mon, NULL);
	GLFWwindow* win = glfwCreateWindow(width, height, title, NULL, NULL);
	ASSERT(win, "Failed to create GLFW window");
	glfwMakeContextCurrent(win);

	glfwSetFramebufferSizeCallback(win, on_resize);
	glfwSetKeyCallback(win, on_key);

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

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

static GLuint setup_texture(const char* image_path) {
	GLuint txt;
	glGenTextures(1, &txt);
	glBindTexture(GL_TEXTURE_2D, txt);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int w, h, num_ch;
	uint8_t* img = stbi_load(image_path, &w, &h, &num_ch, 3);
	ASSERT(img, "Failed to load '%s'", image_path);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(img);
	return txt;
}

static void export(const char* filename) {
	uint8_t buf[WIDTH * HEIGHT * 3];
	glReadBuffer(GL_FRONT); GL();
	glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, buf); GL();
	stbi_write_png(filename, WIDTH, HEIGHT, 3, buf, WIDTH * 3);
}

int main(int argc, char** argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [IMAGE]\n", argv[0]);
	}

	GLFWwindow* win = start(WIDTH, HEIGHT, TITLE);
	GLuint shader_prog = shader_program(vert_code, frag_code);
	GLuint vao, vbo, ebo;
	setup_vertex_objects(vertices, sizeof(vertices), indices, sizeof(indices), &vao, &vbo, &ebo);
	GLuint txt = setup_texture("res/brick.png");

	while (!glfwWindowShouldClose(win)) {
		glClearColor(0.0, 0.2, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, txt);

		glUseProgram(shader_prog);
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		glfwPollEvents();
		glfwSwapBuffers(win);
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteProgram(shader_prog);

	glfwTerminate();
	return 0;
}

