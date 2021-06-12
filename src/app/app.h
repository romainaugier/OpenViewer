#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include <GL/gl3w.h>            // Initialize with gl3wInit()

#include <GLFW/glfw3.h>


#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}



#include <thread>

#include "menubar.h"
#include "display.h"
#include "core/loader.h"
#include "core/parser.h"

typedef struct { GLfloat R, G, B; } color_t;

int application(int argc, char** argv);