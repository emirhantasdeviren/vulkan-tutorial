#include "app.h"
#include <iostream>
#include <utility>

namespace TANELORN_ENGINE_NAMESPACE {
    App::App() : window{}, renderer{window} {}

    App::~App() {}

    void App::run() {
        while (!this->window.close_requested()) {
            this->window.poll_events();
            this->renderer.draw_frame();
        }
        this->renderer.wait_idle();
    }
} // namespace TANELORN_ENGINE_NAMESPACE
