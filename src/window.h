#pragma once

#include "defines.h"

#include <windows.h>

namespace TANELORN_ENGINE_NAMESPACE {
    struct FramebufferSize {
        i32 width;
        i32 height;
    };

    class Window {
    public:
        Window();
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        bool close_requested();
        void poll_events();
        FramebufferSize framebuffer_size() const;
        HWND get_raw_handle() const;
        HINSTANCE get_instance() const;

    private:
        HWND wnd;
        HINSTANCE instance;
        bool running;

        static LRESULT CALLBACK
        main_window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
    };
} // namespace TANELORN_ENGINE_NAMESPACE
