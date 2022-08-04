#pragma once

#include "renderer.h"
#include "window.h"

namespace TANELORN_ENGINE_NAMESPACE {
    class App {
      public:
        App();
        ~App();

        void run();

      private:
        Window window;
        Renderer renderer;
    };
}
