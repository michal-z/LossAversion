#include "Pch.h"
#include "Library.h"
#include "DirectX12.h"
#include "Scene.h"


void* operator new[](size_t size, const char* /*name*/, int /*flags*/, unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return malloc(size);
}

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* /*name*/, int /*flags*/,
					 unsigned /*debugFlags*/, const char* /*file*/, int /*line*/)
{
	return _aligned_offset_malloc(size, alignment, alignmentOffset);
}

static void UpdateFrameTime(HWND window, const char* windowText, double& o_Time, float& o_TimeDelta)
{
    static double s_LastTime = -1.0;
    static double s_LastFpsTime = 0.0;
    static uint32_t s_FpsFrame = 0;

    if (s_LastTime < 0.0)
    {
        s_LastTime = GetTime();
        s_LastFpsTime = s_LastTime;
    }

    o_Time = GetTime();
    o_TimeDelta = (float)(o_Time - s_LastTime);
    s_LastTime = o_Time;

    if ((o_Time - s_LastFpsTime) >= 1.0)
    {
        double fps = s_FpsFrame / (o_Time - s_LastFpsTime);
        double avgFrameTime = (1.0 / fps) * 1000000.0;
        char text[256];
        wsprintf(text, "[%d fps  %d us] %s", (int)fps, (int)avgFrameTime, windowText);
        SetWindowText(window, text);
        s_LastFpsTime = o_Time;
        s_FpsFrame = 0;
    }
    s_FpsFrame++;
}

static HWND MakeWindow(const char* name, uint32_t resolutionX, uint32_t resolutionY, WNDPROC winproc)
{
    WNDCLASS winclass = {};
    winclass.lpfnWndProc = winproc;
    winclass.hInstance = GetModuleHandle(nullptr);
    winclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    winclass.lpszClassName = name;
    if (!RegisterClass(&winclass))
    {
        assert(0);
    }

    RECT rect = { 0, 0, (int32_t)resolutionX, (int32_t)resolutionY };
    if (!AdjustWindowRect(&rect, WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, 0))
    {
        assert(0);
    }

    HWND hwnd = CreateWindowEx(
        0, name, name,
        WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, NULL, 0);
    assert(hwnd);
    return hwnd;
}

static LRESULT CALLBACK ProcessWindowMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(window, message, wParam, lParam);
}

static constexpr char* k_Name = "Loss Aversion";
static constexpr uint32_t k_Resolution[2] = { 1280, 720 };

static int Run()
{
    SetProcessDPIAware();

    HWND window = MakeWindow(k_Name, k_Resolution[0], k_Resolution[1], ProcessWindowMessage);

    DirectX12 dx12;
    if (!dx12.Initialize(window))
    {
        // TODO: Add MessageBox
        return 1;
    }

    Scene scene(dx12);
    scene.Initialize();

	eastl::vector<int> xx;
	xx.push_back(123);

    for (;;)
    {
        MSG message = {};
        if (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
            if (message.message == WM_QUIT)
            {
                break;
            }
        }
        else
        {
            double frameTime;
            float frameTimeDelta;
            UpdateFrameTime(window, k_Name, frameTime, frameTimeDelta);
            scene.Update(frameTime, frameTimeDelta);
            scene.Draw();
            dx12.Present();
        }
    }

    return 0;
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    return Run();
}
