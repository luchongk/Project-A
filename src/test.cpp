#include <windows.h>
#include <glad/glad.h>

#include <cstdio>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;

        case WM_PAINT: {
            ValidateRect(hwnd, nullptr);
        } return 0;

        case WM_SIZE: {
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        } break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

static void handleEvents(MSG *msg, bool &quit) {
    while(PeekMessage(msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(msg);
        DispatchMessage(msg);
        if(msg->message == WM_QUIT)
            quit = true;
    }
}

static bool initOpenGL(HDC dc) {
    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 10;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int pixelFormat = ChoosePixelFormat(dc, &pfd);
    if(!SetPixelFormat(dc, pixelFormat, &pfd)) {
        OutputDebugString(L"Couldn't set pixel format");
        return false;
    }

    HGLRC glContext = wglCreateContext(dc);
    wglMakeCurrent(dc, glContext);

    if(!gladLoadGL()) {
        OutputDebugString(L"Couldn't load Glad");
        return false;
    }

    glViewport(0, 0, 800, 600);

    return true;
}

static void gameUpdate() {

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Gain";

    WNDCLASS wc{};
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    SetCursor(wc.hCursor);
    wc.style = CS_OWNDC;

    if(!RegisterClass(&wc))
        return 1;

    HWND hwnd = CreateWindowW(CLASS_NAME, L"Gain", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                nullptr, nullptr, hInstance, nullptr);

    if(!hwnd)
        return 1;

    HDC dc = GetDC(hwnd);

    if(!initOpenGL(dc))
        return 1;

    ShowWindow(hwnd, nCmdShow);

    char* vertexShaderSrc = 
        "#version 460 core\n"
        "layout (location = 0) in vec3 aPos;\n\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}";

    char* fragmentShaderOrangeSrc = 
        "#version 460 core\n"
        "out vec4 FragColor;\n\n"
        "void main() {\n"
        "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}";

    char* fragmentShaderYellowSrc = 
        "#version 460 core\n"
        "out vec4 FragColor;\n\n"
        "void main() {\n"
        "    FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
        "}";

    //Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);

    /*int success = 0;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        OutputDebugStringA("ERROR VERTEX SHADER COMPILATION FAILED: ");
        OutputDebugStringA(infoLog);
        return 1;
    }*/

    //Compile orange fragment shader
    unsigned int fragmentShaderOrange = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderOrange, 1, &fragmentShaderOrangeSrc, nullptr);
    glCompileShader(fragmentShaderOrange);

    //Compile yellow fragment shader
    unsigned int fragmentShaderYellow = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderYellow, 1, &fragmentShaderYellowSrc, nullptr);
    glCompileShader(fragmentShaderYellow);

    //Link orange program
    unsigned int shaderProgramOrange = glCreateProgram();
    glAttachShader(shaderProgramOrange, vertexShader);
    glAttachShader(shaderProgramOrange, fragmentShaderOrange);
    glLinkProgram(shaderProgramOrange);

    /*success = 0;
    glGetProgramiv(shaderProgramOrange, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgramOrange, 512, nullptr, infoLog);
        OutputDebugStringA("ERROR SHADER PROGRAM LINKING FAILED: ");
        OutputDebugStringA(infoLog);
        return 1;
    }*/

    //Link yellow program
    unsigned int shaderProgramYellow = glCreateProgram();
    glAttachShader(shaderProgramYellow, vertexShader);
    glAttachShader(shaderProgramYellow, fragmentShaderYellow);
    glLinkProgram(shaderProgramYellow);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShaderOrange);
    glDeleteShader(fragmentShaderYellow);

    float vertices[]{
        -0.5f, -0.5f, 0.0f,
        0.0f, -0.5f, 0.0f,
        -0.25f, 0.5f, 0.0f,
    };

    float vertices2[] = {
        0.0f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.25f, 0.5f, 0.0f,
    };
    
    //Create VAO
    unsigned int VAO[2];
    glGenVertexArrays(2, VAO);

    unsigned int VBO[2];
    glGenBuffers(2, VBO);

    //TRIANGLE 1
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    //TRIANGLE 2
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    MSG msg{};
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    double accum = 0.0f;
    double fixedDeltaTime = 0.02f;

    bool quit = false;
    while(!quit) {
        LARGE_INTEGER newTime;
        QueryPerformanceCounter(&newTime);
        double frameTime = (newTime.QuadPart - currentTime.QuadPart) / (double)frequency.QuadPart;
        if(frameTime > 0.25f)
            frameTime = 0.25f;
        currentTime = newTime;

        handleEvents(&msg, quit);

        accum += frameTime;
        
        while(accum >= fixedDeltaTime) {
            gameUpdate();
            //printf_s("gameUpdate, accum: %f ms\n", accum * 1000);
            accum -= fixedDeltaTime;
            //printf_s("remaining %f ms\n", accum * 1000);
        }

        float lerp = (float)(accum / fixedDeltaTime);

        //* RENDERING *//
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgramOrange);
        
        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(shaderProgramYellow);
        
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SwapBuffers(dc);

        printf_s("FPS: %f\n", 1 / frameTime);
        fflush(stdout);
    }

    return 0;
}