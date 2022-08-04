#include "renderer.h"

#include <iostream>
#include <fstream>

const std::vector<const char *> validation_layers = {"VK_LAYER_KHRONOS_validation"};
const std::vector<const char *> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef TN_RELEASE
constexpr bool enable_validations = false;
#else
constexpr bool enable_validations = true;
#endif

struct QueueFamilyIndices {
    uint32_t graphics_family;
    bool graphics_family_found;

    bool is_complete() const {
        return this->graphics_family_found;
    }
};

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

static QueueFamilyIndices find_queue_families(VkPhysicalDevice device) {
    QueueFamilyIndices indices{};

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
            indices.graphics_family_found = true;
            break;
        }
    }

    return indices;
}

static SwapchainSupportDetails
query_swapchain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapchainSupportDetails details{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device, surface, &format_count, details.formats.data()
        );
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device, surface, &present_mode_count, details.present_modes.data()
        );
    }

    return details;
}

static VkSurfaceFormatKHR
choose_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats) {
    for (const VkSurfaceFormatKHR &surface_format : available_formats) {
        if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB
            && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return surface_format;
        }
    }

    return available_formats[0];
}

static VkPresentModeKHR
choose_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes) {
    for (const VkPresentModeKHR &present_mode : available_present_modes) {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static std::vector<char> read_spv(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        std::cout << "Could not open the file: " << filename << std::endl;

        return std::vector<char>();
    } else {
        usize file_size = file.tellg();
        std::vector<char> buffer(file_size);
        file.seekg(0);
        file.read(buffer.data(), file_size);
        file.close();

        return buffer;
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data
) {
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            std::cerr << "[VERBOSE]: ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            std::cerr << "[INFO]:    ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            std::cerr << "[WARNING]: ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            std::cerr << "[ERROR]:   ";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
            break;
    }
    std::cerr << callback_data->pMessage << std::endl;

    return VK_FALSE;
}

namespace TANELORN_ENGINE_NAMESPACE {
    Renderer::Renderer(const Window &window) {
        this->create_instance();
#ifndef TN_RELEASE
        this->create_debug_messenger();
#endif
        this->create_surface(window.handle);
        this->create_physical_device();
        this->create_logical_device();
        this->create_swapchain(window);
        this->create_image_views();
        this->create_render_pass();
        this->create_graphics_pipeline();
        this->create_framebuffers();
        this->create_command_pool();
        this->create_command_buffer();
        this->create_sync_objects();
    }

    Renderer::~Renderer() {
        vkDestroySemaphore(this->device, this->image_available_semaphore, nullptr);
        vkDestroySemaphore(this->device, this->render_finished_semaphore, nullptr);
        vkDestroyFence(this->device, this->in_flight_fence, nullptr);
        std::cout << "Destroyed sync objects.\n";
        vkDestroyCommandPool(this->device, this->command_pool, nullptr);
        std::cout << "Destroyed command pool.\n";
        for (const VkFramebuffer &framebuffer : this->framebuffers) {
            vkDestroyFramebuffer(this->device, framebuffer, nullptr);
            std::cout << "Destroyed framebuffer.\n";
        }
        vkDestroyPipeline(this->device, this->pipeline, nullptr);
        std::cout << "Destroyed pipeline.\n";
        vkDestroyPipelineLayout(this->device, this->pipeline_layout, nullptr);
        std::cout << "Destroyed pipeline layout.\n";
        vkDestroyRenderPass(this->device, this->render_pass, nullptr);
        std::cout << "Destroyed render pass.\n";
        for (const VkImageView &image_view : this->swapchain_image_views) {
            vkDestroyImageView(this->device, image_view, nullptr);
            std::cout << "Destroyed image view.\n";
        }
        vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
        std::cout << "Destroyed swapchain.\n";
        vkDestroySurfaceKHR(this->instance, this->surface, nullptr);
        std::cout << "Destroyed surface.\n";
        vkDestroyDevice(this->device, nullptr);
        std::cout << "Destroyed logical device.\n";
        if (enable_validations) {
            PFN_vkDestroyDebugUtilsMessengerEXT func =
                reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                    vkGetInstanceProcAddr(this->instance, "vkDestroyDebugUtilsMessengerEXT")
                );

            if (func) {
                func(this->instance, this->debug_messenger, nullptr);
                std::cout << "Destroyed debug messenger.\n";
            }
        }
        vkDestroyInstance(this->instance, nullptr);
        std::cout << "Destroyed instance." << std::endl;
    }

    Renderer::Renderer(Renderer &&other) : instance{other.instance} {
        other.instance = VK_NULL_HANDLE;
    }

    Renderer &Renderer::operator=(Renderer &&other) {
        if (this != &other) {
            this->instance = other.instance;
            other.instance = VK_NULL_HANDLE;
        }

        return *this;
    }

    void Renderer::draw_frame() {
        vkWaitForFences(this->device, 1, &this->in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(this->device, 1, &this->in_flight_fence);

        uint32_t image_index;
        vkAcquireNextImageKHR(
            this->device, this->swapchain, UINT64_MAX, this->image_available_semaphore,
            VK_NULL_HANDLE, &image_index
        );

        vkResetCommandBuffer(this->command_buffer, 0);
        this->record_command_buffer(image_index);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore wait_semaphores[] = {this->image_available_semaphore};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &this->command_buffer;

        VkSemaphore signal_semaphores[] = {this->render_finished_semaphore};
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;

        VkResult res = vkQueueSubmit(this->graphics_queue, 1, &submit_info, this->in_flight_fence);

        if (res == VK_SUCCESS) {
            std::cout << "Successfully submitted draw command buffer." << std::endl;
        }

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = signal_semaphores;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &this->swapchain;
        present_info.pImageIndices = &image_index;
        present_info.pResults = nullptr;

        vkQueuePresentKHR(this->graphics_queue, &present_info);
    }

    void Renderer::wait_idle() {
        vkDeviceWaitIdle(this->device);
    }

    void Renderer::create_instance() {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Hello Triangle";
        app_info.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
        app_info.pEngineName = "Tanelorn Engine";
        app_info.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        std::vector<const char *> extensions = Renderer::get_required_instance_extensions();

        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        if (enable_validations) {
            if (Renderer::are_validation_layers_supported()) {
                std::cout << "Validations enabled." << std::endl;
                create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
                create_info.ppEnabledLayerNames = validation_layers.data();

                VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
                debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                debug_create_info.pNext = nullptr;
                debug_create_info.flags = 0;
                debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                                    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debug_create_info.pfnUserCallback = debug_callback;
                debug_create_info.pUserData = nullptr;

                create_info.pNext = static_cast<void *>(&debug_create_info);
            }
        } else {
            create_info.enabledLayerCount = 0;
            create_info.ppEnabledLayerNames = nullptr;
        }

        VkResult res = vkCreateInstance(&create_info, nullptr, &(this->instance));

        if (res == VK_SUCCESS) {
            std::cout << "Successfully created instance" << std::endl;
        }
    }

    void Renderer::create_debug_messenger() {
        VkDebugUtilsMessengerCreateInfoEXT create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                  | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
        create_info.pUserData = nullptr;

        PFN_vkCreateDebugUtilsMessengerEXT func =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(this->instance, "vkCreateDebugUtilsMessengerEXT")
            );

        if (func) {
            if (func(this->instance, &create_info, nullptr, &(this->debug_messenger))
                == VK_SUCCESS) {
                std::cout << "Successfully created debug messenger." << std::endl;
            };
        } else {
            std::cout << "Error: Extension not present." << std::endl;
        }
    }

    void Renderer::create_physical_device() {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(this->instance, &device_count, nullptr);

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(this->instance, &device_count, devices.data());

        for (const VkPhysicalDevice &device : devices) {
            if (Renderer::is_device_suitable(device)) {
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(device, &props);
                std::cout << "Selected device: " << props.deviceName << std::endl;
                this->physical_device = device;
                break;
            }
        }
    }

    void Renderer::create_logical_device() {
        QueueFamilyIndices indices = find_queue_families(this->physical_device);

        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = indices.graphics_family;
        queue_create_info.queueCount = 1;

        float queue_priority = 1.0f;
        queue_create_info.pQueuePriorities = &queue_priority;

        VkPhysicalDeviceFeatures device_features{};

        VkDeviceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.pQueueCreateInfos = &queue_create_info;
        create_info.queueCreateInfoCount = 1;
        create_info.pEnabledFeatures = &device_features;
        create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        create_info.ppEnabledExtensionNames = device_extensions.data();

        VkResult res =
            vkCreateDevice(this->physical_device, &create_info, nullptr, &(this->device));
        if (res == VK_SUCCESS) {
            std::cout << "Successfully created logical device." << std::endl;
            vkGetDeviceQueue(this->device, indices.graphics_family, 0, &(this->graphics_queue));
        }
    }

    void Renderer::create_surface(GLFWwindow *window) {
        VkResult res = glfwCreateWindowSurface(this->instance, window, nullptr, &(this->surface));
        if (res == VK_SUCCESS) {
            std::cout << "Successfully created surface." << std::endl;
        } else {
            std::cout << "Failed to create surface: " << res << std::endl;
        }
    }

    void Renderer::create_swapchain(const Window &window) {
        SwapchainSupportDetails swapchain_support =
            query_swapchain_support(this->physical_device, this->surface);

        VkSurfaceFormatKHR surface_format = choose_surface_format(swapchain_support.formats);
        VkPresentModeKHR present_mode = choose_present_mode(swapchain_support.present_modes);
        VkExtent2D extent = this->choose_extent(swapchain_support.capabilities, window);

        uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0
            && image_count > swapchain_support.capabilities.maxImageCount) {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = this->surface;
        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
        create_info.preTransform = swapchain_support.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        VkResult res = vkCreateSwapchainKHR(this->device, &create_info, nullptr, &this->swapchain);
        if (res == VK_SUCCESS) {
            std::cout << "Successfully created swapchain." << std::endl;

            uint32_t swapchain_image_count;
            vkGetSwapchainImagesKHR(this->device, this->swapchain, &swapchain_image_count, nullptr);
            this->swapchain_images.resize(swapchain_image_count);
            vkGetSwapchainImagesKHR(
                this->device, this->swapchain, &swapchain_image_count, this->swapchain_images.data()
            );

            this->swapchain_image_format = surface_format.format;
            this->swapchain_extent = extent;
        }
    }

    void Renderer::create_image_views() {
        this->swapchain_image_views.resize(this->swapchain_images.size());

        for (uint32_t i = 0; i < this->swapchain_images.size(); i++) {
            VkImageViewCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = this->swapchain_images[i];
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = this->swapchain_image_format;
            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            VkResult res = vkCreateImageView(
                this->device, &create_info, nullptr, this->swapchain_image_views.data() + i
            );

            if (res == VK_SUCCESS) {
                std::cout << "Successfully created image view." << std::endl;
            }
        }
    }

    void Renderer::create_render_pass() {
        VkAttachmentDescription color_attachment{};
        color_attachment.format = this->swapchain_image_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &color_attachment;
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        create_info.dependencyCount = 1;
        create_info.pDependencies = &dependency;

        VkResult res = vkCreateRenderPass(this->device, &create_info, nullptr, &this->render_pass);
        if (res == VK_SUCCESS) {
            std::cout << "Successfully created render pass." << std::endl;
        }
    }

    VkShaderModule Renderer::create_shader_module(const std::vector<char> &code) {
        VkShaderModuleCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = code.size();
        create_info.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shader_module;
        VkResult res = vkCreateShaderModule(this->device, &create_info, nullptr, &shader_module);

        if (res == VK_SUCCESS) {
            std::cout << "Successfully created shader module." << std::endl;
            return shader_module;
        } else {
            std::cout << "Failed to create shader module: " << res << '.' << std::endl;
            return VK_NULL_HANDLE;
        }
    }

    void Renderer::create_graphics_pipeline() {
        std::vector<char> vert_shader_code = read_spv("../shaders/vert.spv");
        std::vector<char> frag_shader_code = read_spv("../shaders/frag.spv");

        VkShaderModule vert_shader_module = this->create_shader_module(vert_shader_code);
        VkShaderModule frag_shader_module = this->create_shader_module(frag_shader_code);

        VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
        vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_stage_info.module = vert_shader_module;
        vert_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
        frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_stage_info.module = frag_shader_module;
        frag_shader_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo shader_stages[2] = {
            vert_shader_stage_info, frag_shader_stage_info};

        std::vector<VkDynamicState> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
        dynamic_state.pDynamicStates = dynamic_states.data();

        VkPipelineVertexInputStateCreateInfo vertex_input_state{};
        vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state.vertexBindingDescriptionCount = 0;
        vertex_input_state.pVertexBindingDescriptions = nullptr;
        vertex_input_state.vertexAttributeDescriptionCount = 0;
        vertex_input_state.pVertexAttributeDescriptions = nullptr;

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                                | VK_COLOR_COMPONENT_B_BIT
                                                | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment.blendEnable = VK_FALSE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f;
        color_blending.blendConstants[1] = 0.0f;
        color_blending.blendConstants[2] = 0.0f;
        color_blending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 0;
        pipeline_layout_create_info.pSetLayouts = nullptr;
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(
                this->device, &pipeline_layout_create_info, nullptr, &this->pipeline_layout
            )
            == VK_SUCCESS) {
            std::cout << "Successfully created pipeline layout." << std::endl;
        }

        VkGraphicsPipelineCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.stageCount = 2;
        create_info.pStages = shader_stages;
        create_info.pVertexInputState = &vertex_input_state;
        create_info.pInputAssemblyState = &input_assembly;
        create_info.pTessellationState = nullptr;
        create_info.pViewportState = &viewport_state;
        create_info.pRasterizationState = &rasterizer;
        create_info.pMultisampleState = &multisampling;
        create_info.pDepthStencilState = nullptr;
        create_info.pColorBlendState = &color_blending;
        create_info.pDynamicState = &dynamic_state;
        create_info.layout = this->pipeline_layout;
        create_info.renderPass = this->render_pass;
        create_info.subpass = 0;
        create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = -1;

        VkResult res = vkCreateGraphicsPipelines(
            this->device, VK_NULL_HANDLE, 1, &create_info, nullptr, &this->pipeline
        );

        if (res == VK_SUCCESS) {
            std::cout << "Successfully created pipeline." << std::endl;
        }

        vkDestroyShaderModule(this->device, frag_shader_module, nullptr);
        vkDestroyShaderModule(this->device, vert_shader_module, nullptr);
    }

    void Renderer::create_framebuffers() {
        this->framebuffers.resize(this->swapchain_image_views.size());

        for (usize i = 0; i < this->swapchain_image_views.size(); i++) {
            VkImageView attachments[] = {this->swapchain_image_views[i]};

            VkFramebufferCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            create_info.renderPass = this->render_pass;
            create_info.attachmentCount = 1;
            create_info.pAttachments = attachments;
            create_info.width = this->swapchain_extent.width;
            create_info.height = this->swapchain_extent.height;
            create_info.layers = 1;

            VkResult res = vkCreateFramebuffer(
                this->device, &create_info, nullptr, this->framebuffers.data() + i
            );

            if (res == VK_SUCCESS) {
                std::cout << "Successfully created framebuffer." << std::endl;
            }
        }
    }

    void Renderer::create_command_pool() {
        QueueFamilyIndices indices = find_queue_families(this->physical_device);

        VkCommandPoolCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        create_info.queueFamilyIndex = indices.graphics_family;

        VkResult res =
            vkCreateCommandPool(this->device, &create_info, nullptr, &this->command_pool);

        if (res == VK_SUCCESS) {
            std::cout << "Successfully created command pool." << std::endl;
        }
    }

    void Renderer::create_command_buffer() {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = this->command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        VkResult res = vkAllocateCommandBuffers(this->device, &alloc_info, &this->command_buffer);

        if (res == VK_SUCCESS) {
            std::cout << "Successfully created command buffer." << std::endl;
        }
    }

    void Renderer::create_sync_objects() {
        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(
                this->device, &semaphore_info, nullptr, &this->image_available_semaphore
            ) == VK_SUCCESS
            && vkCreateSemaphore(
                   this->device, &semaphore_info, nullptr, &this->render_finished_semaphore
               ) == VK_SUCCESS
            && vkCreateFence(this->device, &fence_info, nullptr, &this->in_flight_fence)
                   == VK_SUCCESS) {
            std::cout << "Successfully created sync objects." << std::endl;
        }
    }

    void Renderer::record_command_buffer(uint32_t image_index) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0;
        begin_info.pInheritanceInfo = nullptr;

        VkResult res = vkBeginCommandBuffer(this->command_buffer, &begin_info);

        if (res == VK_SUCCESS) {
            std::cout << "Successfully begun recording command buffer." << std::endl;
        }

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = this->render_pass;
        render_pass_info.framebuffer = this->framebuffers[image_index];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = this->swapchain_extent;

        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(this->command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(this->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(this->swapchain_extent.width);
        viewport.height = static_cast<float>(this->swapchain_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(this->command_buffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = this->swapchain_extent;
        vkCmdSetScissor(this->command_buffer, 0, 1, &scissor);
        vkCmdDraw(this->command_buffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(this->command_buffer);

        res = vkEndCommandBuffer(this->command_buffer);
        if (res == VK_SUCCESS) {
            std::cout << "Successfully ended recording command buffer." << std::endl;
        }
    }

    bool Renderer::are_validation_layers_supported() {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const char *layer_name : validation_layers) {
            bool layer_found = false;

            for (const VkLayerProperties &layer_props : available_layers) {
                if (strcmp(layer_name, layer_props.layerName) == 0) {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char *> Renderer::get_required_instance_extensions() {
        uint32_t extension_count;
        const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        std::vector<const char *> extensions(glfw_extensions, glfw_extensions + extension_count);

        if (enable_validations) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool Renderer::is_device_suitable(VkPhysicalDevice device) {
        QueueFamilyIndices indices = find_queue_families(device);

        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        bool extensions_supported = Renderer::check_device_extension_support(device);

        bool swapchain_adequate = false;
        if (extensions_supported) {
            SwapchainSupportDetails swapchain_support =
                query_swapchain_support(device, this->surface);
            swapchain_adequate =
                !(swapchain_support.formats.empty() || swapchain_support.present_modes.empty());
        }

        return props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && features.geometryShader
               && indices.is_complete() && swapchain_adequate;
    }

    bool Renderer::check_device_extension_support(VkPhysicalDevice device) {
        uint32_t extension_count = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> props(extension_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, props.data());

        for (const char *extension_name : device_extensions) {
            bool found = false;

            for (const VkExtensionProperties &prop : props) {
                if (strcmp(extension_name, prop.extensionName) == 0) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return false;
            }
        }

        return true;
    }

    VkExtent2D
    Renderer::choose_extent(const VkSurfaceCapabilitiesKHR &capabilities, const Window &window) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            FramebufferSize size = window.framebuffer_size();

            VkExtent2D actual_extent = {
                static_cast<uint32_t>(size.width), static_cast<uint32_t>(size.height)};

            actual_extent.width = actual_extent.width < capabilities.minImageExtent.width
                                      ? capabilities.minImageExtent.width
                                  : actual_extent.width > capabilities.maxImageExtent.width
                                      ? capabilities.maxImageExtent.width
                                      : actual_extent.width;
            actual_extent.height = actual_extent.height < capabilities.minImageExtent.height
                                       ? capabilities.minImageExtent.height
                                   : actual_extent.height > capabilities.maxImageExtent.height
                                       ? capabilities.maxImageExtent.height
                                       : actual_extent.height;

            return actual_extent;
        }
    }
} // namespace TANELORN_ENGINE_NAMESPACE
