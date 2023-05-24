#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GLFW/glfw3.h>

char *read_file(const char *path) {
  FILE *file = fopen(path, "r");
  if (!file) {
    return NULL;
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    fclose(file);
    return NULL;
  }

  long size = ftell(file);
  if (size == -1) {
    fclose(file);
    return NULL;
  }
  rewind(file);

  char *data = malloc(size + 1);
  if (!data) {
    fclose(file);
    return NULL;
  }
  data[size] = '\0';

  long read = fread(data, 1, size, file);
  fclose(file);

  if (read != size) {
    free(data);
    return NULL;
  }

  return data;
}

GLuint new_shader(const char *path, GLenum type) {
  char *data = read_file(path);
  if (!data) {
    fprintf(stderr, "error: could not read file '%s'\n", path);
    return 0;
  }

  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, (const GLchar *const *)&data, NULL);
  glCompileShader(shader);

  GLint result;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

  if (!result) {
    GLchar buffer[1024];
    glGetShaderInfoLog(shader, sizeof(buffer), NULL, buffer);
    fprintf(stderr, "error: %s\n", buffer);
    shader = 0;
  }

  free(data);
  return shader;
}

GLuint new_program(const char *vertex, const char *fragment) {
  GLuint vertex_shader = new_shader(vertex, GL_VERTEX_SHADER);
  if (!vertex_shader) {
    return 0;
  }

  GLuint fragment_shader = new_shader(fragment, GL_FRAGMENT_SHADER);
  if (!fragment_shader) {
    return 0;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  GLint result;
  glGetProgramiv(program, GL_LINK_STATUS, &result);

  if (!result) {
    GLchar buffer[1024];
    glGetProgramInfoLog(program, sizeof(buffer), NULL, buffer);
    fprintf(stderr, "error: %s\n", buffer);
    program = 0;
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  return program;
}

GLint get_uniform(GLuint program, const char *name) {
  GLint uniform = glGetUniformLocation(program, name);
  if (uniform == -1) {
    fprintf(stderr, "error: could not get location of uniform '%s'\n", name);
  }
  return uniform;
}

typedef struct {
  GLuint mouse;
  GLuint resolution;
} State;

void key_callback(GLFWwindow *window, int key, int scan, int action, int mods) {
  if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}

void mouse_callback(GLFWwindow *window, double x, double y) {
  int w, h;
  glfwGetWindowSize(window, &w, &h);

  State *state = glfwGetWindowUserPointer(window);
  glUniform2f(state->mouse, x / w, 1 - y / h);
}

void resize_callback(GLFWwindow *window, int w, int h) {
  State *state = glfwGetWindowUserPointer(window);
  glViewport(0, 0, w, h);
  glUniform2f(state->resolution, w, h);
}

int main(void) {
  if (!glfwInit()) {
    fprintf(stderr, "error: could not initialize GLFW\n");
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
  if (!window) {
    fprintf(stderr, "error: could not create window\n");
    exit(1);
  }
  glfwMakeContextCurrent(window);

  GLuint program = new_program("src/main.vert", "src/main.frag");
  if (!program) {
    exit(1);
  }
  glUseProgram(program);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  State state;
  glfwSetWindowUserPointer(window, &state);

  state.mouse = get_uniform(program, "iMouse");
  if (state.mouse == -1) {
    exit(1);
  }

  state.resolution = get_uniform(program, "iResolution");
  if (state.resolution == -1) {
    exit(1);
  }

  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetWindowSizeCallback(window, resize_callback);
  glfwSetKeyCallback(window, key_callback);

  {
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    resize_callback(window, w, h);
  }

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteProgram(program);
  glDeleteVertexArrays(1, &vao);

  glfwDestroyWindow(window);
  glfwTerminate();
}
