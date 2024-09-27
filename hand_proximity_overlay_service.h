#ifndef INA_HAND_PROXIMITY_SERVICES_OVERLAY_SERVICE_H
#define INA_HAND_PROXIMITY_SERVICES_OVERLAY_SERVICE_H
//#include "Eigen/Dense"
//#include "core/pipeline_context.h"

#include "glfw_window.h"

class HandProximityOverlayService : public GlfwWindow
{
public:
    void ResizeWindow(int width, int height);
    void SetPosition(int x, int y);
    void Hide();

    // Shader
    void SetUniform1f(std::string uniform_name, float value0);
    void SetUniform2f(std::string uniform_name, float value0, float value1);
    void SetUniform3f(std::string uniform_name, float value, float value1, float value2);
    void SetUniform4f(std::string uniform_name, float value, float value1, float value2, float value3);
    void SetUniform1i(std::string uniform_name, int value0);

    // Drawing
    void Draw();

    // Magnifier
    void ExcludeCapture();
    void IncludeCapture();

    HandProximityOverlayService();
    ~HandProximityOverlayService();

    int Width = 500;
    int Height = 500;

private:
    void InitDrawWithShader(unsigned int program);
    void DrawWithShader();
    void SetWindowPosition();
    std::string GetFragmentShaderCodeTemplate();

    GLuint _vertex_buffer = 0;

    GLuint _program = 0;
    GLuint _texture_program = 0;
    
    int _pos_x = 0; // cursor position x
    int _pos_y = 0; // cursor position y

    float _render_shift_x = 0.0;
    float _render_shift_y = 0.0;

    // magnifier
    float _magnification = 2.0;
    //GLuint _magnifier_program = 0;
    unsigned int screen_texture = 0;
    GLubyte* screen_texture_data = NULL;
    HBITMAP hBitmap;
    //HDC hMemoryDC;
    HDC hScreenDC;

};

#endif /* INA_HAND_PROXIMITY_SERVICES_OVERLAY_SERVICE_H */