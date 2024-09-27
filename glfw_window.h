#ifndef INA_SERVICES_GLFW_WINDOW_H
#define INA_SERVICES_GLFW_WINDOW_H
//#include "Eigen/Dense"
//#include "core/pipeline_context.h"

//#include "services/gui/i_visual_debug_service.h"
#include <string>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


struct Color_f4
{
    float r, g, b, w;

    Color_f4(float _r, float _g, float _b, float _w)
    {
        r = _r;
        g = _g;
        b = _b;
        w = _w;
    }
};

class GlfwWindow
{
public:
    void ResizeGlfwWindow(int width, int height);
    void SetPositionGlfwWindow(int x, int y);
    void HideGlfwWindow();
    void MinimizeGlfwWindow();
    void RestoreGlfwWindow();
    void ShowGlfwWindow();
    void ShowGlfwWindowIfHidden();
    void HideGlfwWindowIfShown();
    bool IsGlfwWindowVisible();
    bool IsGlfwWindowMinimized();
    unsigned int AddShaderInternal(std::string VertexShaderCode, std::string FragmentShaderCode);
    std::string GetFullscreenVertexShaderCode();

    // Drawing
    void InitGlfw();
    void CreateGlfwWindow(std::string name, int width, int height);


    void DrawGlfwBegin();
    void DrawGlfwEnd();

    GlfwWindow();
    ~GlfwWindow();
    GLFWwindow* _window = NULL;
    Color_f4 _clearColor = Color_f4(0,0,0,0);

    int _monitor_width = 0;
    int _monitor_height = 0;


private:



};

#endif /* INA_SERVICES_GLFW_WINDOW_H */