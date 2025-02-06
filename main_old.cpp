#include <cstdint>
#include <vector>

// file opening (shaders)
#include <string>
#include <fstream>
#include <sstream>

// OpenGL
// NOTE: glad needs to be first
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "inc/buffers/vao.h"
#include "inc/buffers/vbo.h"
#include "inc/buffers/ebo.h"

float fov = 45.0f;
float near_plane = 0.1f;
float far_plane = 100.0f;

glm::vec3 camera_position = glm::vec3(15.71f, 8.86f, 38.01f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camera_rotation = glm::vec3(0.0f, 0.0f, -1.0f);

uint32_t window_width = 1200;
uint32_t window_height = 900;

GLFWwindow *window;

#include <vector>

std::vector<Vertex> vertices = {
    {glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
    {glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
    {glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
    {glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)}};

std::vector<GLuint> indices = {0, 1, 2, 0, 2, 3};

std::string get_file_contents(const char *filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }
    fprintf(stderr, "Could not open file: %s\n", filename);
    return "";
}

void resizeCallback(GLFWwindow *p_window, int width, int height)
{
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
}

int main()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ---------- WINDOW CREATION START
    window = glfwCreateWindow(window_width, window_height, "FLUID SIMULATION", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, window_width, window_height);

    glfwSetFramebufferSizeCallback(window, resizeCallback);
    // ---------- WINDOW CREATION END

    // ---------- SHADER CREATION START
    std::string vertexCode = get_file_contents("./resources/shaders/default.vert");
    std::string fragmentCode = get_file_contents("./resources/shaders/default.frag");

    const char *vertexSource = vertexCode.c_str();
    const char *fragmentSource = fragmentCode.c_str();

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    // compileErrors(vertexShader, "VERTEX");

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    // compileErrors(fragmentShader, "FRAGMENT");

    GLuint water_shader_id = glCreateProgram();
    glAttachShader(water_shader_id, vertexShader);
    glAttachShader(water_shader_id, fragmentShader);
    glLinkProgram(water_shader_id);
    // compileErrors(water_shader_id, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // ---------- SHADER CREATION END

    // CAMERA DATA
    VAO vao;
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 camera_matrix = glm::mat4(1.0f);
    float aspect_ratio = 1.0f;
    float f_fov_rad = 1.0f;

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ------------  UPDATE MATRIX START
        view = glm::lookAt(camera_position, camera_position + camera_rotation, camera_up);
        aspect_ratio = (float)window_width / (float)window_height;
        f_fov_rad = glm::radians(fov);
        projection = glm::perspective(f_fov_rad, aspect_ratio, near_plane, far_plane);

        camera_matrix = projection * view;
        // ------------  UPDATE MATRIX END

        // ------------ RENDER START
        vao.bind();
        VBO vbo(vertices);
        EBO ebo(indices);
        vao.linkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(Vertex), (void *)0);
        vao.linkAttrib(vbo, 1, 3, GL_FLOAT, sizeof(Vertex), (void *)(3 * sizeof(float)));
        vao.linkAttrib(vbo, 2, 3, GL_FLOAT, sizeof(Vertex), (void *)(6 * sizeof(float)));
        vao.linkAttrib(vbo, 3, 2, GL_FLOAT, sizeof(Vertex), (void *)(9 * sizeof(float)));
        vao.unbind();
        vbo.unbind();
        ebo.unbind();

        vbo.destroy();
        ebo.destroy();

        glUseProgram(water_shader_id);

        // FLUID
        glUniform3f(glGetUniformLocation(water_shader_id, "scale"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(water_shader_id, "rotation"), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(water_shader_id, "position"), 0.0f, 0.0f, 0.0f);

        // LIGHT
        glUniform4f(glGetUniformLocation(water_shader_id, "lightColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(water_shader_id, "lightPos"), 5.0f, 10.0f, 5.0f);

        // CAMERA
        glUniform3f(glGetUniformLocation(water_shader_id, "camPos"), camera_position.x, camera_position.y, camera_position.z);
        glUniformMatrix4fv(glGetUniformLocation(water_shader_id, "camMatrix"), 1, GL_FALSE, glm::value_ptr(camera_matrix));

        vao.bind();
        glUniform1i(glGetUniformLocation(water_shader_id, "useTexture"), 0);

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        // ------------ RENDER END

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ------ CLEANUP START
    if (glIsProgram(water_shader_id))
    {
        glDeleteProgram(water_shader_id);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    // ------ CLEANUP END

    return 0;
}

/**
#define MAX_FILEPATH_LENGTH 100
char default_vertex_shader_path[MAX_FILEPATH_LENGTH];
char default_fragment_shader_path[MAX_FILEPATH_LENGTH];
char planks_texture_path[MAX_FILEPATH_LENGTH];
char planks_specular_texture_path[MAX_FILEPATH_LENGTH];
const char *get_resources_path(char *dest, const char *resource_name)
{
    memset(&(dest[0]), 0, strlen(dest));
    FAVE::log("dest: \"%s\"", dest);
    const char *resources_dir = "./resources/";
    strcat(dest, resources_dir);
    strcat(dest, resource_name);
    FAVE::log("dest: \"%s\"", dest);
    return dest;
}
 */