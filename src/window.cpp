#include "window.h"

#include <iostream>

namespace TANELORN_ENGINE_NAMESPACE {
    Window::Window() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        this->handle = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
    }

    Window::~Window() {
        glfwDestroyWindow(this->handle);
        glfwTerminate();
        std::cout << "Destroyed window." << std::endl;
    }

    bool Window::close_requested() {
        return glfwWindowShouldClose(this->handle);
    }

    void Window::poll_events() {
        glfwPollEvents();
    }

    FramebufferSize Window::framebuffer_size() const {
        int width;
        int height;

        glfwGetFramebufferSize(this->handle, &width, &height);

        return FramebufferSize{width, height};
    }
} // namespace TANELORN_ENGINE_NAMESPACE
