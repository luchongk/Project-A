#include <cstdio>

#include "platform_win32.h"

#include "linear_allocator.cpp"
#include "shader.cpp"
#include "shader_manager.cpp"

static LARGE_INTEGER frequency;     //TODO: Think of a way to unglobalize this

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

static void reloadGameCode(char* gameDLLPath, GameCode* gameCode, GameMemory* gameMemory) {
    if(gameCode->dll) {
        /*void (*onUnload)(GameMemory*) = (void(*)(GameMemory*))GetProcAddress(gameCode->dll, "onUnload");
        if(onUnload)
            onUnload(gameMemory);*/

        if(!FreeLibrary(gameCode->dll)) {
            printf("Couldn't free game code DLL");
            return;
        }
    }

    gameCode->api.update = updateStub;
    gameCode->api.render = renderStub;

    char* DestDLLPath = ".\\game_tmp.dll";
    
    if(CopyFileA(gameDLLPath, DestDLLPath, false))
        gameCode->dll = LoadLibraryA(DestDLLPath);
    else {
        DWORD err = GetLastError();
        gameCode->dll = nullptr;
        return;
    }

    gameCode->dllLastWriteTime = getLastWriteTime(gameDLLPath);

    if(!gameCode->dll)
        printf("Failed to load game code DLL\n");
    else {
        void (*onLoad)(GameMemory*) = (void(*)(GameMemory*))GetProcAddress(gameCode->dll, "onLoad");
        if(onLoad)
            onLoad(gameMemory);
        
        gameCode->api.update = (updateFunction*)GetProcAddress(gameCode->dll, "update");
        if(!gameCode->api.update) {
            printf("Failed to load update function\n");
            gameCode->api.update = updateStub;
        }
        gameCode->api.render = (renderFunction*)GetProcAddress(gameCode->dll, "render");
        if(!gameCode->api.render) {
            printf("Failed to load render function\n");
            gameCode->api.render = renderStub;
        }
    }
}

static void handleEvents(MSG *msg, PlayerInput* input) {
    while(PeekMessage(msg, nullptr, 0, 0, PM_REMOVE)) {
        switch(msg->message) {
            case WM_QUIT: {
                input->quit = true;
            } break;

            case WM_KEYDOWN: {
                uint32_t VKCode = (uint32_t)msg->wParam;
                bool wasDown = msg->lParam & 0x40000000;

                switch(VKCode) {
                    case 'P': {
                        if(!wasDown) {
                            input->pause = true;
                        }
                    } break;
                }
            } break;

            default: {
                TranslateMessage(msg);
                DispatchMessage(msg);
            }
        }
    }

    if(GetKeyState(VK_LEFT) & 0x8000)
        input->horizontal = -1;
    else if(GetKeyState(VK_RIGHT) & 0x8000)
        input->horizontal = 1;
}

inline static float getTimeElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
    assert(start.QuadPart < end.QuadPart);
    return (end.QuadPart - start.QuadPart) / (float)frequency.QuadPart;
}

inline static LARGE_INTEGER getWallClock() {
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return current;
}

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
    
    MSG msg{};

    GameMemory gameMemory{};
    gameMemory.isInitialized = false;
    gameMemory.totalSize = megabytes(64);  //TODO: Config
    gameMemory.data[0] = VirtualAlloc(nullptr, gameMemory.totalSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    gameMemory.data[1] = VirtualAlloc(nullptr, gameMemory.totalSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    gameMemory.currentDataIndex = 0;

    char* gameDLLPath = ".\\game.dll";
    GameCode gameCode{};
    reloadGameCode(gameDLLPath, &gameCode, &gameMemory);
    assert(gameCode.dll);

    timeBeginPeriod(1);
    LARGE_INTEGER frameTime;
    QueryPerformanceCounter(&frameTime);
    QueryPerformanceFrequency(&frequency);

    //TODO: Config each of these!
    constexpr float maxFrameTime = 0.25f;
    constexpr float targetFrameRate = 0.0166f;
    float accum = 0.0f;
    float deltaTime = 0.0f;
    float fixedDeltaTime = 0.02f;
    
    PlayerInput input{};

    bool quit = false;
    while(!quit) {      
        FILETIME lastWriteTime = getLastWriteTime(gameDLLPath);
        if(CompareFileTime(&gameCode.dllLastWriteTime, &lastWriteTime) != 0)
            reloadGameCode(gameDLLPath, &gameCode, &gameMemory);

        handleEvents(&msg, &input);
        quit = input.quit;

        if(deltaTime > maxFrameTime)
            deltaTime = maxFrameTime;   //Framerate too low, we are falling behind in the simulation! D: Preventing spiral of death.
        
        accum += deltaTime;
        
        bool updateRan = false;
        while(accum >= fixedDeltaTime) {
            gameCode.api.update(&gameMemory, &input);
            accum -= fixedDeltaTime;
            updateRan = true;
        }

        if(updateRan) {
            input = {};
        }

        float lerp = (float)(accum / fixedDeltaTime);

        //* RENDERING *//
        gameCode.api.render(&gameMemory);

        //TODO Implement Vsync switch
        //! This should only happen if VSync is off
        LARGE_INTEGER workCounter = getWallClock();
        float workDelta = getTimeElapsed(frameTime, workCounter);
        if(workDelta < targetFrameRate) {
            Sleep((int)((targetFrameRate - workDelta) * 1000 - 1));
            workCounter = getWallClock();
            workDelta = getTimeElapsed(frameTime, workCounter);
        }
        while(workDelta < targetFrameRate) {
            workCounter = getWallClock();
            workDelta = getTimeElapsed(frameTime, workCounter);
        }
        frameTime = workCounter;
        deltaTime = workDelta;
#if DEBUG
        printf_s("last frame: %f ms\n", deltaTime);
        printf_s("FPS: %f\n", 1 / deltaTime);
        fflush(stdout);
#endif
        SwapBuffers(dc);
    }

    return 0;
}