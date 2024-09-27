#include "hand_proximity_overlay_service.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <regex>


using namespace std;





void resize_callback(GLFWwindow* window, int width, int height)
{
    HandProximityOverlayService* overlayService = static_cast<HandProximityOverlayService*>(glfwGetWindowUserPointer(window));

    if (width != overlayService->Width && height != overlayService->Height) {
        //overlayService->ResizeWindow(overlayService->Width, overlayService->Height);

        glfwSetWindowSize(window, overlayService->Width, overlayService->Height);
    }
}
HandProximityOverlayService::HandProximityOverlayService() {
    InitGlfw();

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

    // create screen texture
    glGenTextures(1, &screen_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // setup screen capturing
    hScreenDC = GetDC(nullptr); // CreateDC("DISPLAY",nullptr,nullptr,nullptr);
    int width_screen = GetDeviceCaps(hScreenDC, HORZRES);
    int height_screen = GetDeviceCaps(hScreenDC, VERTRES);
    //hMemoryDC = CreateCompatibleDC(hScreenDC);
    hBitmap = CreateCompatibleBitmap(hScreenDC, width_screen, height_screen);

    // create data buffer for texture
    screen_texture_data = new GLubyte[Width * Height * 3];

    ExcludeCapture();
}

HandProximityOverlayService::~HandProximityOverlayService() {
    delete screen_texture_data;
}

void HandProximityOverlayService::InitDrawWithShader(unsigned int program) {

    DrawGlfwBegin();
    ShowGlfwWindowIfHidden();

    _program = program;

    glViewport(0, 0, Width, Height);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(_program);

    //HDC hScreenDC = GetDC(nullptr); // CreateDC("DISPLAY",nullptr,nullptr,nullptr);
    int width_screen = GetDeviceCaps(hScreenDC, HORZRES);
    int height_screen = GetDeviceCaps(hScreenDC, VERTRES);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    //HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width_screen, height_screen);

    HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));
    BitBlt(hMemoryDC, 0, 0, Width, Height, hScreenDC, _pos_x - Width / 2, _pos_y - Height / 2, SRCCOPY);
    hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));

    BITMAPINFOHEADER info;
    info.biSize = sizeof(BITMAPINFOHEADER);
    info.biWidth = Width;
    info.biHeight = -Height; // we usually want a top-down-bitmap
    info.biPlanes = 1;
    info.biBitCount = 24;
    info.biCompression = BI_RGB;
    info.biSizeImage = 0;
    info.biXPelsPerMeter = 10000; // just some value
    info.biYPelsPerMeter = 10000; // just some value
    info.biClrUsed = 0;
    info.biClrImportant = 0;
    GetDIBits(hMemoryDC, hBitmap, 0, Height, (void*)screen_texture_data, (BITMAPINFO*)&info, DIB_RGB_COLORS);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, screen_texture_data);

    SetUniform1f("magnification", _magnification);

    SetUniform2f("render_shift", _render_shift_x, _render_shift_y);
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
    SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);
}

void HandProximityOverlayService::ResizeWindow(int width, int height) {

    Width = width;
    Height = height;
    glfwMakeContextCurrent(_window);
    ResizeGlfwWindow(Width, Height);
    SetWindowPosition();

    delete screen_texture_data;
    screen_texture_data = new GLubyte[Width * Height * 3];
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screen_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

void HandProximityOverlayService::SetPosition(int x, int y) {
    _pos_x = x;
    _pos_y = y;
    SetWindowPosition();
}

void HandProximityOverlayService::Hide() {
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


    int window_pos_x = _pos_x - Width / 2;
    int window_pos_y = _pos_y - Height / 2;

    int offset_xm = window_pos_x - _monitor.Left;
    int offset_xp = window_pos_x - _monitor.Left + Width - _monitor.Width;

    int offset_ym = window_pos_y - _monitor.Top;
    int offset_yp = window_pos_y - _monitor.Top + Height - _monitor.Height;


    _render_shift_x = 0.0;
    _render_shift_y = 0.0;



    if (offset_xm < 0) {
        window_pos_x = _monitor.Left;
        _render_shift_x = -2.0 * (float)offset_xm / (float)Width;
    }
    else if(offset_xp>0){
        window_pos_x = _monitor.Left + _monitor.Width - Width;
        _render_shift_x = -2.0 * (float)offset_xp / (float)Width;
    }

    if (offset_ym < 0) {
        window_pos_y = _monitor.Top;
        _render_shift_y = 2.0 * (float)offset_ym / (float)Height;

    }
    else if (offset_yp > 0) {
        window_pos_y = _monitor.Top + _monitor.Height - Height;
        _render_shift_y = 2.0 * (float)offset_yp / (float)Height;
    }



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
uniform sampler2D screen_texture;
uniform float magnification = 2.0f;
uniform vec2 render_shift;

uniform sampler2D animation_texture;

vec4 draw(vec2 input_pos)
{
	vec4 output_color = vec4(0.0);
	vec2 uv = vec2((input_pos.x + 1.0)/2.0, (-input_pos.y + 1.0)/2.0);

    //if(uv.x>=0.0 && uv.x < 1.0 && uv.y>=0.0 && uv.y < 1.0){
    //    output_color = texture(animation_texture, uv); // animation_texture is already premultiplied
    //    output_color = vec4(0, 0, 0, 0);
    //}else{
    //    output_color = vec4(0, 0, 0, 0);
    //}
    output_color = vec4(0, 0, 0, 0);
	return output_color; // output_color is premultiplied
}


void main()
{
    vec2 shifted_pos = pos + render_shift;
    vec2 newpos = shifted_pos;
    vec4 magnifier_color = vec4(0.0);


        float inx = shifted_pos.x;
        float iny = shifted_pos.y;

        vec2 dir = shifted_pos/length(shifted_pos);
        float r = sqrt(inx*inx + iny*iny);

        float nr = 0.0;

        float l1 = 0.8;
        float l2 = 1.0;
        float slope = 1.0/magnification;

        if(r<l1){
            nr = r*slope;
        }else if(r>l2){
            nr = r;
        }else{
            nr = (r-l1)/(l2-l1)*(l2 - l1*slope) + l1*slope;
        }

        newpos = dir*nr;

        vec2 uv = vec2((newpos.x + 1.0)/2.0, (-newpos.y + 1.0)/2.0);

        if(uv.x>=0.0 && uv.x < 1.0 && uv.y>=0.0 && uv.y < 1.0){
            magnifier_color = texture(screen_texture, uv); // texture is already premultiplied
        }else{
            magnifier_color = vec4(0.0, 0.0, 0.0, 0.0);
        }

        magnifier_color.rb = magnifier_color.br; // bgr to rgb

        // cut circle
        if(shifted_pos.x*shifted_pos.x + shifted_pos.y*shifted_pos.y>1){
            magnifier_color = vec4(0.0); // this area is already premultiplied
        }

    vec4 draw_color = draw(newpos);

    color_frag = draw_color + magnifier_color * (1.0 - draw_color.a);

}

)FRAGMENTSHADER";
}
