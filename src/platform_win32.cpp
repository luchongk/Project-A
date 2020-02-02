#include <windows.h>
#include <glad/glad.h>

#include <cstdio>
#include <stdint.h>

typedef void (*updateFunction)();
static void updateStub() {}
typedef void (*renderFunction)(unsigned int, unsigned int);
static void renderStub(unsigned int, unsigned int) {}
struct GameCode {
    HMODULE dll;
    FILETIME dllLastWriteTime;
    updateFunction update;
    renderFunction render;
};
static GameCode gameCode{}; //? Should this have a constructor so that functions initialize to their stubs automatically?

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;

        case WM_SIZE: {
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        } break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

static FILETIME getLastWriteTime(char* filename) {
    FILETIME lastWriteTime{};

    WIN32_FIND_DATAA fileData;
    HANDLE findHandle = FindFirstFileA(filename, &fileData);
    if(findHandle != INVALID_HANDLE_VALUE) {
        FindClose(findHandle);
        lastWriteTime = fileData.ftLastWriteTime;
    }

    return lastWriteTime;
}

static GameCode reloadGameCode(char* gameDLLPath) {
    if(gameCode.dll && !FreeLibrary(gameCode.dll)) {
        printf("Couldn't free game code DLL");
        return gameCode;
    }

    GameCode result;
    result.update = updateStub;
    result.render = renderStub;

    char* DestDLLPath = ".\\bin\\game_tmp.dll";
    
    if(CopyFileA(gameDLLPath, DestDLLPath, false))
        result.dll = LoadLibraryA("..\\bin\\game_tmp.dll");
    else {
        result.dll = nullptr;
        return result;
    }

    result.dllLastWriteTime = getLastWriteTime(gameDLLPath);

    if(!result.dll)
        printf("Failed to load game code DLL\n");
    else {
        void (*init)() = (void(*)()) GetProcAddress(result.dll, "init");
        if(init)
            init();

        result.update = (updateFunction)GetProcAddress(result.dll, "update");
        if(!result.update) {
            printf("Failed to load update function\n");
            result.update = updateStub;
        }
        result.render = (renderFunction)GetProcAddress(result.dll, "render");
        if(!result.render) {
            printf("Failed to load render function\n");
            result.render = renderStub;
        }
    }

    return result;
}

static void handleEvents(MSG *msg, bool &quit) {
    while(PeekMessage(msg, nullptr, 0, 0, PM_REMOVE)) {
        switch(msg->message) {
            case WM_QUIT: {
                quit = true;
            } break;

            case WM_KEYUP: {
                uint32_t VKCode = (uint32_t)msg->wParam;

                /*switch(VKCode) {
                    case 'R':
                        gameCode = reloadGameCode(&gameCode);
                        break;
                }*/
            } break;

            default: {
                TranslateMessage(msg);
                DispatchMessage(msg);
            }
        }
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

    MSG msg{};

    LARGE_INTEGER frameTime;
    LARGE_INTEGER prevFrameTime;
    QueryPerformanceCounter(&prevFrameTime);
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    const float maxFrameTime = 0.25f;
    const float targetFrameRate = 0.033f;
    double accum = 0.0f;
    double fixedDeltaTime = 0.02f;

    char* gameDLLPath = ".\\bin\\game.dll";
    gameCode = reloadGameCode(gameDLLPath);

    bool quit = false;
    while(!quit) {      
        
        QueryPerformanceCounter(&frameTime);
        double deltaTime = (frameTime.QuadPart - prevFrameTime.QuadPart) / (double)frequency.QuadPart;
#ifdef DEBUG
        printf_s("last frame: %f ms\n", deltaTime);
        printf_s("FPS: %f\n", 1 / deltaTime);
        fflush(stdout);
#endif
        if(deltaTime > maxFrameTime)
            deltaTime = maxFrameTime;   //Framerate too low, we are falling behind in the simulation! D: Preventing spiral of death.
        prevFrameTime = frameTime;

        FILETIME lastWriteTime = getLastWriteTime(gameDLLPath);
        if(CompareFileTime(&gameCode.dllLastWriteTime, &lastWriteTime) != 0)
            gameCode = reloadGameCode(gameDLLPath);

        handleEvents(&msg, quit);

        accum += deltaTime;
        
        while(accum >= fixedDeltaTime) {
            gameCode.update();
            //printf_s("update, accum: %f ms\n", accum * 1000);
            accum -= fixedDeltaTime;
            //printf_s("remaining %f ms\n", accum * 1000);
        }

        float lerp = (float)(accum / fixedDeltaTime);

        //TODO: After learning OpenGL put rendering code into the DLL (ie. gameCode.render())
        //* RENDERING *//
        gameCode.render(shaderProgramOrange, shaderProgramYellow);

        //TODO Implement Vsync switch
        //! This should only happen if VSync is off
        LARGE_INTEGER afterRenderTime;
        QueryPerformanceCounter(&afterRenderTime);
        double afterRenderDelta = (afterRenderTime.QuadPart - frameTime.QuadPart) / (double)frequency.QuadPart;
        if(afterRenderDelta < targetFrameRate) {
            timeBeginPeriod(1);
            Sleep((targetFrameRate - afterRenderDelta) * 1000 - 1);
            timeEndPeriod(1);
        }

        SwapBuffers(dc);

        while(afterRenderDelta < targetFrameRate) {
            QueryPerformanceCounter(&afterRenderTime);
            afterRenderDelta = (afterRenderTime.QuadPart - frameTime.QuadPart) / (double)frequency.QuadPart;
        }
    }

    return 0;
}