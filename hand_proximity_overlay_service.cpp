#include "hand_proximity_overlay_service.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <regex>


using namespace std;





void resize_callback(GLFWwindow* window, int width, int height)
{
    HandProximityOverlayService* overlayService = static_cast<HandProximityOverlayService*>(glfwGetWindowUserPointer(window));

    if (width != overlayService->_screen_width && height != overlayService->_screen_height) {
        //overlayService->ResizeWindow(overlayService->Width, overlayService->Height);

        glfwSetWindowSize(window, overlayService->_screen_width, overlayService->_screen_height);
    }
}
HandProximityOverlayService::HandProximityOverlayService() {
    InitGlfw();

    // setup screen capturing
    hScreenDC = GetDC(nullptr);
    _screen_width = GetDeviceCaps(hScreenDC, HORZRES);
    _screen_height = GetDeviceCaps(hScreenDC, VERTRES);

    // custom hints for cursor
    glfwWindowHint(GLFW_FLOATING, true);
    glfwWindowHint(GLFW_DECORATED, false);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);

    CreateGlfwWindow("Cursor", 1000, 1000);
    glfwSetWindowUserPointer(_window, this);
    glfwSetWindowSizeCallback(_window, resize_callback);
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    
    std::string VertexShaderCode = GetFullscreenVertexShaderCode();
    std::string FragmentShaderCode = GetFragmentShaderCodeTemplate();

    glfwMakeContextCurrent(_window);
    _texture_program = AddShaderInternal(VertexShaderCode, FragmentShaderCode);

    // set tool window for not having a taskbar entry
    HWND hWnd = glfwGetWin32Window(_window);
    int windowStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    SetWindowLong(hWnd, GWL_EXSTYLE, windowStyle | WS_EX_TOOLWINDOW); //3d argument=style

    ExcludeCapture();

    ResizeWindowToFullScreen();
}

HandProximityOverlayService::~HandProximityOverlayService() {
    //delete screen_texture_data;
}

void HandProximityOverlayService::InitDrawWithShader(unsigned int program) {

    DrawGlfwBegin();
    ShowGlfwWindowIfHidden();

    _program = program;

    glViewport(0, 0, _screen_width, _screen_height);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(_program);
}

void HandProximityOverlayService::DrawWithShader() {
    InitDrawWithShader(_texture_program);

    glfwMakeContextCurrent(_window);
    glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
    DrawGlfwEnd();
}

void HandProximityOverlayService::Draw()
{
    InitDrawWithShader(_texture_program);
    DrawWithShader();
}

void HandProximityOverlayService::ExcludeCapture() {
    HWND hWnd = glfwGetWin32Window(_window);
    SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);

}

void HandProximityOverlayService::IncludeCapture() {
    HWND hWnd = glfwGetWin32Window(_window);
    SetWindowDisplayAffinity(hWnd, WDA_NONE);
}

void HandProximityOverlayService::ResizeWindowToFullScreen() 
{
    glfwMakeContextCurrent(_window);
    ResizeGlfwWindow(_screen_width, _screen_height);
    SetWindowPosition();
}

void HandProximityOverlayService::SetPosition(int x, int y) 
{
    float shader_x = 2 * ((float)x / _screen_width ) - 1;
    float shader_y = (2 * ((float)y / _screen_height) - 1) * -1;

    SetUniform2f("cursor_pos", shader_x, shader_y);
}

void HandProximityOverlayService::Hide() 
{
    HideGlfwWindowIfShown();
}

struct MonitorInformation
{
    int Left, Top, Width, Height;

    MonitorInformation()
    {
        Left = Top = 0;
        Width = 2560;
        Height = 1600;
    }
};

void HandProximityOverlayService::SetWindowPosition() 
{
    MonitorInformation _monitor;

    int window_pos_x = 0;
    int window_pos_y = 0;

    _render_shift_x = 0.0;
    _render_shift_y = 0.0;

    SetPositionGlfwWindow(window_pos_x, window_pos_y);
}

void HandProximityOverlayService::SetUniform1f(std::string uniform_name, float value0) {
    glfwMakeContextCurrent(_window);
    GLint uniform_location = glGetUniformLocation(_program, uniform_name.c_str());
    glUniform1f(uniform_location, value0);
}

void HandProximityOverlayService::SetUniform2f(std::string uniform_name, float value0, float value1) {
    glfwMakeContextCurrent(_window);
    GLint uniform_location = glGetUniformLocation(_program, uniform_name.c_str());
    glUniform2f(uniform_location, value0, value1);
}

void HandProximityOverlayService::SetUniform3f(std::string uniform_name, float value0, float value1, float value2) {
    glfwMakeContextCurrent(_window);
    GLint uniform_location = glGetUniformLocation(_program, uniform_name.c_str());
    glUniform3f(uniform_location, value0, value1, value2);
}

void HandProximityOverlayService::SetUniform4f(std::string uniform_name, float value0, float value1, float value2, float value3) {
    glfwMakeContextCurrent(_window);
    GLint uniform_location = glGetUniformLocation(_program, uniform_name.c_str());
    glUniform4f(uniform_location, value0, value1, value2, value3);
}

void HandProximityOverlayService::SetUniform1i(std::string uniform_name, int value0) {
    glfwMakeContextCurrent(_window);
    GLint uniform_location = glGetUniformLocation(_program, uniform_name.c_str());
    glUniform1i(uniform_location, value0);
}

std::string HandProximityOverlayService::GetFragmentShaderCodeTemplate() {

    return R"FRAGMENTSHADER(

#version 330 core
out vec4 color_frag;
in vec2 pos;
uniform vec2 cursor_pos;

vec4 gradient(in vec4 color1, in vec4 color2, in float value)
{
    return value * color1 + (1-value) * color2;
}

void main()
{
    float dist_to_cursor = distance(pos, cursor_pos);

    color_frag = vec4(0, 0, 0, 0);
    //color_frag = vec4(color_frag.b, color_frag.g, color_frag.r, color_frag.w); // bgr to rgb

    //cursor
    float cur_size = 0.10;
    if(dist_to_cursor < cur_size)
    {
        color_frag = gradient(color_frag, vec4(0,0.6,0.6,0), dist_to_cursor / cur_size);
    }  
}

)FRAGMENTSHADER";
}
