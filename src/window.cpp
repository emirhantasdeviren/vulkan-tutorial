#include "window.h"

#include <iostream>

namespace TANELORN_ENGINE_NAMESPACE {
    Window::Window() : instance{GetModuleHandle(nullptr)}, running{false} {
        WNDCLASSA wc{};
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = main_window_proc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = this->instance;
        wc.hIcon = nullptr;
        wc.hCursor = nullptr;
        wc.hbrBackground = nullptr;
        wc.lpszMenuName = nullptr;
        wc.lpszClassName = "tnWindowClass";

        if (!RegisterClassA(&wc)) {
            DWORD err = GetLastError();
            std::cout << "Failed to register window class: " << err << std::endl;
        }

        RECT client_rect;
        client_rect.left = 0;
        client_rect.top = 0;
        client_rect.right = 800;
        client_rect.bottom = 600;

        if (!AdjustWindowRect(
                &client_rect, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) ^ WS_THICKFRAME, 0
            )) {
            DWORD err = GetLastError();
            std::cout << "Failed to adjust client rect: " << err << std::endl;
        }

        this->wnd = CreateWindowExA(
            0, "tnWindowClass", "Vulkan", (WS_OVERLAPPEDWINDOW | WS_VISIBLE) ^ WS_THICKFRAME,
            CW_USEDEFAULT, CW_USEDEFAULT, client_rect.right - client_rect.left,
            client_rect.bottom - client_rect.top, nullptr, nullptr, this->instance,
            static_cast<LPVOID>(this)
        );

        if (this->wnd) {
            std::cout << "Successfully created window." << std::endl;
            this->running = true;
        } else {
            DWORD err = GetLastError();
            std::cout << "Failed to create window: " << err << std::endl;
        }
    }

    Window::~Window() {
        UnregisterClassA("tnWindowClass", this->instance);
        DestroyWindow(this->wnd);
        std::cout << "Destroyed window." << std::endl;
    }

    bool Window::close_requested() {
        return !this->running;
    }

    void Window::poll_events() {
        MSG msg{};

        if (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE) > 0) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    FramebufferSize Window::framebuffer_size() const {
        RECT client_rect;
        GetClientRect(this->wnd, &client_rect);

        return FramebufferSize{client_rect.right, client_rect.bottom};
    }

    HWND Window::get_raw_handle() const {
        return this->wnd;
    }

    HINSTANCE Window::get_instance() const {
        return this->instance;
    }

    LRESULT CALLBACK
    Window::main_window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
        Window *self = reinterpret_cast<Window *>(GetWindowLongPtrA(window, GWLP_USERDATA));

        if (!self) {
            if (message == WM_NCCREATE) {
                Window *data =
                    static_cast<Window *>(reinterpret_cast<CREATESTRUCT *>(l_param)->lpCreateParams
                    );

                SetLastError(0);
                if (!SetWindowLongPtrA(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(data))) {
                    return TRUE;
                } else {
                    return FALSE;
                }
            }
        } else {
            switch (message) {
                case WM_CLOSE:
                case WM_DESTROY: {
                    self->running = false;
                } break;
                case WM_KEYDOWN: {
                    if (w_param == VK_ESCAPE) {
                        self->running = false;
                    }
                } break;
                default:
                    return DefWindowProcA(window, message, w_param, l_param);
            }
        }

        return 0;
    }
} // namespace TANELORN_ENGINE_NAMESPACE
