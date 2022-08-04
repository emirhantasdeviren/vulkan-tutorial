#pragma once

#define GLFW_INCLUDE_VULKAN

#include "defines.h"
#include "window.h"

#include <vector>

namespace TANELORN_ENGINE_NAMESPACE {
    class Renderer {
    public:
        explicit Renderer(const Window &window);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        Renderer(Renderer &&);
        Renderer &operator=(Renderer &&);

        void draw_frame();
        void wait_idle();

    private:
        void create_instance();
        void create_debug_messenger();
        void create_surface(GLFWwindow *window);
        void create_physical_device();
        void create_logical_device();
        void create_swapchain(const Window &window);
        void create_image_views();
        void create_render_pass();
        void create_graphics_pipeline();
        void create_framebuffers();
        void create_command_pool();
        void create_command_buffer();
        void create_sync_objects();

        void record_command_buffer(uint32_t image_index);

        static bool are_validation_layers_supported();
        static std::vector<const char *> get_required_instance_extensions();
        bool is_device_suitable(VkPhysicalDevice device);
        static bool check_device_extension_support(VkPhysicalDevice device);

        static VkExtent2D
        choose_extent(const VkSurfaceCapabilitiesKHR &capabilities, const Window &window);

        VkShaderModule create_shader_module(const std::vector<char> &code);

        VkInstance instance;
        VkDebugUtilsMessengerEXT debug_messenger;
        VkPhysicalDevice physical_device;
        VkDevice device;
        VkQueue graphics_queue;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchain_images;
        VkFormat swapchain_image_format;
        VkExtent2D swapchain_extent;
        std::vector<VkImageView> swapchain_image_views;
        VkRenderPass render_pass;
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
        std::vector<VkFramebuffer> framebuffers;
        VkCommandPool command_pool;
        VkCommandBuffer command_buffer;
        VkSemaphore image_available_semaphore;
        VkSemaphore render_finished_semaphore;
        VkFence in_flight_fence;

    };
} // namespace TANELORN_ENGINE_NAMESPACE
