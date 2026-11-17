#include <SFML/Graphics.hpp>
#include <chrono>
#include <cwchar>
#include <dwmapi.h>
#include <filesystem>
#include <string>
#include <thread>
#include <windows.h>
#include <windowsx.h>
#include "QuarkPhysics/qworld.h"

static struct {
    int width, height;
    sf::RenderWindow sf_window; //the sfml render window
    QWorld *quark_world = new QWorld(); //the quark physics world
    /*sf::Shader background_shader;
    sf::Texture matrix_atlas;*/
    void init_physics() {
        quark_world->SetGravity(QVector(0.0f, -9.81f));

        QRigidBody *square = new QRigidBody();
        square->AddMesh(QMesh::CreateWithRect(QVector(100, 100), QVector(0, 0)));
        square->SetPosition(QVector(width / 2.0, height - 200));
        square->SetRotationDegree(45);
        quark_world->AddBody(square);

        QSoftBody *circle = new QSoftBody();
        square->AddMesh(QMesh::CreateWithCircle(50, QVector(0, 0)));
        square->SetPosition(QVector(width / 2.0 - 100, height - 400));
        quark_world->AddBody(square);

        QBody *floor = new QBody();
        floor->AddMesh(QMesh::CreateWithRect(QVector(width, 1.0), QVector(0.0f, 0.0f)))->SetPosition(QVector(width / 2.0, 1.0));
        floor->SetMode(QBody::Modes::STATIC);
        quark_world->AddBody(floor);

        QBody *wall_left = new QBody();
        wall_left->AddMesh(QMesh::CreateWithRect(QVector(1.0, height), QVector(0.0f, 0.0f)))->SetPosition(QVector(1.0, height / 2.0));
        wall_left->SetMode(QBody::Modes::STATIC);
        quark_world->AddBody(wall_left);

        QBody *wall_right = new QBody();
        wall_right->AddMesh(QMesh::CreateWithRect(QVector(1, height), QVector(0.0f, 0.0f)))->SetPosition(QVector(width, height / 2.0));
        wall_right->SetMode(QBody::Modes::STATIC);
        quark_world->AddBody(wall_right);
    }
} PurplexityBG;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    // check if the system supports shaders
    /*if (!sf::Shader::isAvailable()) {
        MessageBoxW(nullptr, L"Your system doesn't support shader programming!", L"Can't use shaders", MB_ICONERROR);
        return -1;
    }
    // load the fragment shader
    if (!PurplexityBG.background_shader.loadFromFile("shader.frag", sf::Shader::Type::Fragment)) {
        MessageBoxW(nullptr, L"Couldn't load fragment shader!", L"Can't load fragment shader", MB_ICONERROR);
        return -1;
    }
    if (!PurplexityBG.matrix_atlas.loadFromFile("matrix_atlas.png")) {
        MessageBoxW(nullptr, L"Couldn't load matrix atlas!", L"Can't load matrix atlas", MB_ICONERROR);
        return -1;
    }
    PurplexityBG.matrix_atlas.setSmooth(true);*/

    // create the window class
    const wchar_t CLASS_NAME[] = L"Purplexity BG";
    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    if (!RegisterClassW(&wc)) {
        MessageBoxW(nullptr, L"Failed to register window class!", L"Can't register class", MB_ICONERROR);
        return -1;
    }
    // get the work area of the primary screen
    RECT wa = {0};
    if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0)) {
        MessageBoxW(nullptr, L"Failed to get work area!", L"Can't get work area", MB_ICONERROR);
        return -1;
    }
    PurplexityBG.width = wa.right - wa.left;
    PurplexityBG.height = wa.bottom - wa.top;
    // create the window
    HWND hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, CLASS_NAME, L"PurplexityBG", WS_POPUP, wa.left, wa.top, PurplexityBG.width, PurplexityBG.height, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        MessageBoxW(nullptr, L"Failed to create application window!", L"Can't create window", MB_ICONERROR);
        return -1;
    }

    RegisterHotKey(hwnd, 1, MOD_CONTROL | MOD_ALT | MOD_NOREPEAT, 'Q'); // for quitting press ctrl + alt + q
    SetTimer(hwnd, 1, 17, nullptr); //'stay on desktop' behavior timer
    PurplexityBG.sf_window.create(hwnd); // create the sfml window using our native hwnd
    PurplexityBG.init_physics();
    ShowWindow(hwnd, SW_SHOWNOACTIVATE); // show window without stealing focus
    UpdateWindow(hwnd);

    MSG msg = {};
    // handle messages
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

// enumerates over all windows from top to bottom (when (bool*)lParam is set to
// true there is an active application window on screen, when not the desktop
// was raised over all apps / there's just the desktop used for detecting win +
// d and setting z-pos accordingly
static BOOL CALLBACK EnumProc(HWND h, LPARAM lParam) {
    if (!IsWindowVisible(h) || IsIconic(h) || GetWindow(h, GW_OWNER)) return TRUE;
    if (GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) return TRUE;

    int cloaked = 0;
    DwmGetWindowAttribute(h, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
    if (cloaked) return TRUE;

    wchar_t cls[64];
    GetClassNameW(h, cls, 64);
    if (!wcscmp(cls, L"Progman") || !wcscmp(cls, L"WorkerW") || !wcscmp(cls, L"Shell_TrayWnd") || !wcscmp(cls, L"Shell_SecondaryTrayWnd")) return TRUE;

    *reinterpret_cast<bool *>(lParam) = true;
    return FALSE;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            UnregisterHotKey(hwnd, 1);
            PostQuitMessage(0);
            break;
        case WM_HOTKEY:
            if (wParam == 1) DestroyWindow(hwnd);
            break;
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE; // don't steal focus
        case WM_TIMER: {
            // enumerate all windows and when the desktop is the highest, set flag
            // topmost (so we are in front of it) else set bottom so we don't go over
            // other apps just sets position when the state changed so we don't set
            // position every frame
            static int last = -1;
            bool any = false;
            EnumWindows(EnumProc, reinterpret_cast<LPARAM>(&any));
            if (static_cast<int>(any) != last) {
                last = any;
                SetWindowPos(hwnd, any ? HWND_BOTTOM : HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        }
        case WM_PAINT: {
            RECT r;
            GetClientRect(hwnd, &r);
            static auto start_time = std::chrono::high_resolution_clock::now();
            const unsigned int width = r.right - r.left;
            const unsigned int height = r.bottom - r.top;
            /*sf::RectangleShape shape(sf::Vector2f(width, height));
            std::string uniform = "u_atlas";
            PurplexityBG.background_shader.setUniform("u_atlas", PurplexityBG.matrix_atlas);
            uniform = "u_time";
            PurplexityBG.background_shader.setUniform(uniform, std::chrono::duration<float>(std::chrono::steady_clock::now() - start_time).count());
            uniform = "u_resolution";
            PurplexityBG.background_shader.setUniform(uniform, sf::Glsl::Vec2(static_cast<float>(width), static_cast<float>(height)));
            PurplexityBG.sf_window.draw(shape, &PurplexityBG.background_shader);*/
            PurplexityBG.sf_window.clear(sf::Color(20, 20, 30));
            PurplexityBG.quark_world->Update();
            for (int i = 0; i < PurplexityBG.quark_world->GetBodyCount(); i++) {
                QAABB aabb = PurplexityBG.quark_world->GetBodyAt(i)->GetAABB();
                sf::Vector2f size(aabb.GetSize().x, aabb.GetSize().y);
                sf::RectangleShape shape(size);
                shape.setFillColor(sf::Color::White);
                auto rectCenter = aabb.GetCenterPosition();
                shape.setPosition({rectCenter.x - size.x / 2, height - (rectCenter.y + size.y / 2)});
                PurplexityBG.sf_window.draw(shape);
            }
            PurplexityBG.sf_window.display();
            ValidateRect(hwnd, nullptr);
            break;
        }

        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
