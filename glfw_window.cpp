#include "glfw_window.h"
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>


using namespace std;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

GlfwWindow::GlfwWindow() {

}

GlfwWindow::~GlfwWindow() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}


void GlfwWindow::InitGlfw() {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    _monitor_width = mode->width;
    _monitor_height = mode->height;
}

void GlfwWindow::CreateGlfwWindow(std::string name, int width, int height) {
    _window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
    if (!_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(_window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0); // do not wait for monitor refrehsing

    _clearColor = Color_f4(1.f, 0.f, 0.f, 1.f);// Eigen::Vector4f(1.0, 0.0, 0.0, 1.0);

    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.w);

    SetPositionGlfwWindow(0, 40);
}



void GlfwWindow::DrawGlfwBegin() {


    glfwMakeContextCurrent(_window);
    glfwPollEvents();

    glClear(GL_COLOR_BUFFER_BIT);

}
void GlfwWindow::DrawGlfwEnd() {

    glfwMakeContextCurrent(_window);
    glfwSwapBuffers(_window);
}



void GlfwWindow::ResizeGlfwWindow(int width, int height) {
    glfwSetWindowSize(_window, width, height);
}

void GlfwWindow::SetPositionGlfwWindow(int x, int y) {
    glfwSetWindowPos(_window, x, y);
}

void GlfwWindow::HideGlfwWindow() {
    glfwHideWindow(_window);
}

void GlfwWindow::MinimizeGlfwWindow() {
    glfwIconifyWindow(_window);
}

void GlfwWindow::RestoreGlfwWindow() {
    glfwRestoreWindow(_window);
}

void GlfwWindow::ShowGlfwWindow() {
    glfwShowWindow(_window);
}

void GlfwWindow::ShowGlfwWindowIfHidden() {
    int visible = IsGlfwWindowVisible();
    if (!visible) {
        glfwShowWindow(_window);
    }
}

void GlfwWindow::HideGlfwWindowIfShown() {
    int visible = IsGlfwWindowVisible();
    if (visible) {
        glfwHideWindow(_window);
    }
}

bool GlfwWindow::IsGlfwWindowVisible() {
    return glfwGetWindowAttrib(_window, GLFW_VISIBLE);
}

bool GlfwWindow::IsGlfwWindowMinimized() {
    return glfwGetWindowAttrib(_window, GLFW_ICONIFIED);
}


unsigned int GlfwWindow::AddShaderInternal(std::string VertexShaderCode, std::string FragmentShaderCode) {
    glfwMakeContextCurrent(_window);
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;

    GLuint program;

    GLint Result = GL_FALSE;
    int InfoLogLength;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(vertex_shader, 1, &VertexSourcePointer, NULL);
    glCompileShader(vertex_shader);


    // Check Vertex Shader
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(vertex_shader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(fragment_shader, 1, &FragmentSourcePointer, NULL);
    glCompileShader(fragment_shader);

    // Check Fragment Shader
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(fragment_shader, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Check the program
    glGetProgramiv(program, GL_LINK_STATUS, &Result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }


    return program;
}

std::string GlfwWindow::GetFullscreenVertexShaderCode() {

    return R"VERTEXSHADER(

#version 330 core
out vec2 pos;
void main()
{
  vec2 vertices[3]=vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));
  gl_Position = vec4(vertices[gl_VertexID],0,1);
  pos = vertices[gl_VertexID];
}

)VERTEXSHADER";
}
