#pragma once

#include "defines.h"

#include <GLFW/glfw3.h>

namespace TANELORN_ENGINE_NAMESPACE {
    struct FramebufferSize {
        i32 width;
        i32 height;
    };

    struct Window {
        GLFWwindow *handle;

        Window();
        ~Window();

        bool close_requested();
        void poll_events();
        FramebufferSize framebuffer_size() const;
    };
} // namespace TANELORN_ENGINE_NAMESPACE
