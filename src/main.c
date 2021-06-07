#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;

typedef float f32;
typedef double f64;

typedef struct {
    PFN_vkGetInstanceProcAddr vk_get_instance_proc_addr;
    PFN_vkEnumerateInstanceVersion vk_enumerate_instance_version;
    PFN_vkEnumerateInstanceExtensionProperties vk_enumerate_instance_extension_properties;
    PFN_vkEnumerateInstanceLayerProperties vk_enumerate_instance_layer_properties;
    PFN_vkCreateInstance vk_create_instance;
} EntryPoint;

typedef struct {
    PFN_vkDestroyInstance vk_destroy_instance;
    PFN_vkEnumeratePhysicalDevices vk_enumerate_physical_devices;
    PFN_vkGetPhysicalDeviceProperties vk_get_physical_device_properties;
    PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messenger;
    PFN_vkDestroyDebugUtilsMessengerEXT vk_destroy_debug_utils_messenger;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vk_get_physical_device_queue_family_properties;
    PFN_vkCreateDevice vk_create_device;
    PFN_vkCreateWin32SurfaceKHR vk_create_win32_surface;
    PFN_vkDestroySurfaceKHR vk_destroy_surface;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vk_get_physical_device_surface_support;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vk_get_physical_device_surface_capabilities;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vk_get_physical_device_surface_formats;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vk_get_physical_device_surface_present_modes;
} InstanceFunctions;

typedef struct {
    PFN_vkGetDeviceProcAddr vk_get_device_proc_addr;
    PFN_vkDestroyDevice vk_destroy_device;
    PFN_vkGetDeviceQueue vk_get_device_queue;
    PFN_vkCreateSwapchainKHR vk_create_swapchain;
    PFN_vkDestroySwapchainKHR vk_destroy_swapchain;
    PFN_vkGetSwapchainImagesKHR vk_get_swapchain_images;
    PFN_vkCreateImageView vk_create_image_view;
    PFN_vkDestroyImageView vk_destroy_image_view;
    PFN_vkCreateShaderModule vk_create_shader_module;
    PFN_vkDestroyShaderModule vk_destroy_shader_module;
    PFN_vkCreatePipelineLayout vk_create_pipeline_layout;
    PFN_vkDestroyPipelineLayout vk_destroy_pipeline_layout;
    PFN_vkCreateRenderPass vk_create_render_pass;
    PFN_vkDestroyRenderPass vk_destroy_render_pass;
    PFN_vkCreateGraphicsPipelines vk_create_graphics_pipelines;
    PFN_vkDestroyPipeline vk_destroy_pipeline;
} DeviceFunctions;

static i32 RUNNING = 0;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data
) {
    char *severity;
    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            severity = "VERBOSE";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            severity = "INFO";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            severity = "WARNING";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            severity = "ERROR";
            break;
        default:
            severity = "UNKNOWN";
    }

    char *type;
    switch (message_types) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            type = "GENERAL";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            type = "VALIDATION";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            type = "PERFORMANCE";
            break;
        default:
            type = "UNKNOWN";
    }

    OutputDebugStringA(callback_data->pMessage);

    return VK_FALSE;
}

static LRESULT CALLBACK main_window_callback(
    HWND window,
    UINT message,
    WPARAM w_param,
    LPARAM l_param
) {
    LRESULT result = 0;

    switch (message) {
        case WM_CLOSE: {
            RUNNING = 0;
        } break;
        case WM_DESTROY: {
            RUNNING = 0;
        } break;
        default: {
            result = DefWindowProcA(window, message, w_param, l_param);
        } break;
    }

    return result;
}

static void init_entry_point(HMODULE libvulkan, EntryPoint *entry) {
    PFN_vkGetInstanceProcAddr vk_get_instance_proc_addr =
        (PFN_vkGetInstanceProcAddr)GetProcAddress(libvulkan, "vkGetInstanceProcAddr");

    PFN_vkEnumerateInstanceVersion vk_enumerate_instance_version =
        (PFN_vkEnumerateInstanceVersion)vk_get_instance_proc_addr(
            0,
            "vkEnumerateInstanceVersion"
        );
    PFN_vkEnumerateInstanceExtensionProperties vk_enumerate_instance_extension_properties =
        (PFN_vkEnumerateInstanceExtensionProperties)vk_get_instance_proc_addr(
            0,
            "vkEnumerateInstanceExtensionProperties"
        );
    PFN_vkEnumerateInstanceLayerProperties vk_enumerate_instance_layer_properties =
        (PFN_vkEnumerateInstanceLayerProperties)vk_get_instance_proc_addr(
            0,
            "vkEnumerateInstanceLayerProperties"
        );
    PFN_vkCreateInstance vk_create_instance =
        (PFN_vkCreateInstance)vk_get_instance_proc_addr(0, "vkCreateInstance");

    entry->vk_get_instance_proc_addr = vk_get_instance_proc_addr;
    entry->vk_enumerate_instance_version = vk_enumerate_instance_version;
    entry->vk_enumerate_instance_extension_properties = vk_enumerate_instance_extension_properties;
    entry->vk_enumerate_instance_layer_properties = vk_enumerate_instance_layer_properties;
    entry->vk_create_instance = vk_create_instance;
}

static VkInstance new_instance(EntryPoint *entry, InstanceFunctions *fps) {
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = 0;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = "4+1 Engine";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    char *enabled_extension_names[3] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };
    char *enabled_layer_names = "VK_LAYER_KHRONOS_validation";

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.pNext = 0;
    debug_create_info.flags = 0;
    debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = debug_callback;
    debug_create_info.pUserData = 0;

    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = &debug_create_info;
    create_info.flags = 0;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = &enabled_layer_names;
    create_info.enabledExtensionCount = 3;
    create_info.ppEnabledExtensionNames = enabled_extension_names;

    VkInstance instance;
    VkResult result = entry->vk_create_instance(&create_info, 0, &instance);

    if (result == VK_SUCCESS) {
        PFN_vkDestroyInstance vk_destroy_instance =
            (PFN_vkDestroyInstance)entry->vk_get_instance_proc_addr(
                instance,
                "vkDestroyInstance"
            );
        PFN_vkEnumeratePhysicalDevices vk_enumerate_physical_devices =
            (PFN_vkEnumeratePhysicalDevices)entry->vk_get_instance_proc_addr(
                instance,
                "vkEnumeratePhysicalDevices"
            );
        PFN_vkGetPhysicalDeviceProperties vk_get_physical_device_properties =
            (PFN_vkGetPhysicalDeviceProperties)entry->vk_get_instance_proc_addr(
                instance,
                "vkGetPhysicalDeviceProperties"
            );
        PFN_vkCreateDebugUtilsMessengerEXT vk_create_debug_utils_messenger =
            (PFN_vkCreateDebugUtilsMessengerEXT)entry->vk_get_instance_proc_addr(
                instance,
                "vkCreateDebugUtilsMessengerEXT"
            );
        PFN_vkDestroyDebugUtilsMessengerEXT vk_destroy_debug_utils_messenger =
            (PFN_vkDestroyDebugUtilsMessengerEXT)entry->vk_get_instance_proc_addr(
                instance,
                "vkDestroyDebugUtilsMessengerEXT"
            );
        PFN_vkGetPhysicalDeviceQueueFamilyProperties vk_get_physical_device_queue_family_properties =
            (PFN_vkGetPhysicalDeviceQueueFamilyProperties)entry->vk_get_instance_proc_addr(
                instance,
                "vkGetPhysicalDeviceQueueFamilyProperties"
            );
        PFN_vkCreateDevice vk_create_device =
            (PFN_vkCreateDevice)entry->vk_get_instance_proc_addr(
                instance,
                "vkCreateDevice"
            );
        PFN_vkCreateWin32SurfaceKHR vk_create_win32_surface =
            (PFN_vkCreateWin32SurfaceKHR)entry->vk_get_instance_proc_addr(
                instance,
                "vkCreateWin32SurfaceKHR"
            );
        PFN_vkDestroySurfaceKHR vk_destroy_surface =
            (PFN_vkDestroySurfaceKHR)entry->vk_get_instance_proc_addr(
                instance,
                "vkDestroySurfaceKHR"
            );
        PFN_vkGetPhysicalDeviceSurfaceSupportKHR vk_get_physical_device_surface_support =
            (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)entry->vk_get_instance_proc_addr(
                instance,
                "vkGetPhysicalDeviceSurfaceSupportKHR"
            );
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vk_get_physical_device_surface_capabilities =
            (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)entry->vk_get_instance_proc_addr(
                instance,
                "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"
            );
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vk_get_physical_device_surface_formats =
            (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)entry->vk_get_instance_proc_addr(
                instance,
                "vkGetPhysicalDeviceSurfaceFormatsKHR"
            );
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vk_get_physical_device_surface_present_modes =
            (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)entry->vk_get_instance_proc_addr(
                instance,
                "vkGetPhysicalDeviceSurfacePresentModesKHR"
            );

        fps->vk_destroy_instance = vk_destroy_instance;
        fps->vk_enumerate_physical_devices = vk_enumerate_physical_devices;
        fps->vk_get_physical_device_properties = vk_get_physical_device_properties;
        fps->vk_create_debug_utils_messenger = vk_create_debug_utils_messenger;
        fps->vk_destroy_debug_utils_messenger = vk_destroy_debug_utils_messenger;
        fps->vk_get_physical_device_queue_family_properties = vk_get_physical_device_queue_family_properties;
        fps->vk_create_device = vk_create_device;
        fps->vk_create_win32_surface = vk_create_win32_surface;
        fps->vk_destroy_surface = vk_destroy_surface;
        fps->vk_get_physical_device_surface_support = vk_get_physical_device_surface_support;
        fps->vk_get_physical_device_surface_capabilities = vk_get_physical_device_surface_capabilities;
        fps->vk_get_physical_device_surface_formats = vk_get_physical_device_surface_formats;
        fps->vk_get_physical_device_surface_present_modes = vk_get_physical_device_surface_present_modes;

        return instance;
    } else {
        return 0;
    }
}

static VkDebugUtilsMessengerEXT new_debug_messenger(
    VkInstance instance,
    InstanceFunctions instance_fps
) {
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.pNext = 0;
    debug_create_info.flags = 0;
    debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = debug_callback;
    debug_create_info.pUserData = 0;

    VkDebugUtilsMessengerEXT debug_messenger;
    VkResult result = instance_fps.vk_create_debug_utils_messenger(
        instance,
        &debug_create_info,
        0,
        &debug_messenger
    );
    if (result == VK_SUCCESS) {
        return debug_messenger;
    } else {
        return 0;
    }
}

static VkDevice new_device(
    VkPhysicalDevice gpu,
    usize queue_family_index,
    InstanceFunctions instance_fps,
    HMODULE libvulkan,
    DeviceFunctions *device_fps
) {
    VkDeviceQueueCreateInfo queue_create_info;
    f32 queue_priority = 1.0f;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext = 0;
    queue_create_info.flags = 0;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures gpu_features = {0};
    char *enabled_extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    char *enabled_layer_names = "VK_LAYER_KHRONOS_validation";

    VkDeviceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.enabledLayerCount = 1;
    create_info.ppEnabledLayerNames = &enabled_layer_names;
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = &enabled_extension_names;
    create_info.pEnabledFeatures = &gpu_features;

    VkDevice device;
    VkResult result = instance_fps.vk_create_device(gpu, &create_info, 0, &device);

    if (result == VK_SUCCESS) {
        PFN_vkGetDeviceProcAddr vk_get_device_proc_addr =
            (PFN_vkGetDeviceProcAddr)GetProcAddress(libvulkan, "vkGetDeviceProcAddr");
        PFN_vkDestroyDevice vk_destroy_device =
            (PFN_vkDestroyDevice)vk_get_device_proc_addr(device, "vkDestroyDevice");
        PFN_vkGetDeviceQueue vk_get_device_queue =
            (PFN_vkGetDeviceQueue)vk_get_device_proc_addr(device, "vkGetDeviceQueue");
        PFN_vkCreateSwapchainKHR vk_create_swapchain =
            (PFN_vkCreateSwapchainKHR)vk_get_device_proc_addr(device, "vkCreateSwapchainKHR");
        PFN_vkDestroySwapchainKHR vk_destroy_swapchain =
            (PFN_vkDestroySwapchainKHR)vk_get_device_proc_addr(device, "vkDestroySwapchainKHR");
        PFN_vkGetSwapchainImagesKHR vk_get_swapchain_images =
            (PFN_vkGetSwapchainImagesKHR)vk_get_device_proc_addr(device, "vkGetSwapchainImagesKHR");
        PFN_vkCreateImageView vk_create_image_view =
            (PFN_vkCreateImageView)vk_get_device_proc_addr(device, "vkCreateImageView");
        PFN_vkDestroyImageView vk_destroy_image_view =
            (PFN_vkDestroyImageView)vk_get_device_proc_addr(device, "vkDestroyImageView");
        PFN_vkCreateShaderModule vk_create_shader_module =
            (PFN_vkCreateShaderModule)vk_get_device_proc_addr(device, "vkCreateShaderModule");
        PFN_vkDestroyShaderModule vk_destroy_shader_module =
            (PFN_vkDestroyShaderModule)vk_get_device_proc_addr(device, "vkDestroyShaderModule");
        PFN_vkCreatePipelineLayout vk_create_pipeline_layout =
            (PFN_vkCreatePipelineLayout)vk_get_device_proc_addr(device, "vkCreatePipelineLayout");
        PFN_vkDestroyPipelineLayout vk_destroy_pipeline_layout =
            (PFN_vkDestroyPipelineLayout)vk_get_device_proc_addr(device, "vkDestroyPipelineLayout");
        PFN_vkCreateRenderPass vk_create_render_pass =
            (PFN_vkCreateRenderPass)vk_get_device_proc_addr(device, "vkCreateRenderPass");
        PFN_vkDestroyRenderPass vk_destroy_render_pass =
            (PFN_vkDestroyRenderPass)vk_get_device_proc_addr(device, "vkDestroyRenderPass");
        PFN_vkCreateGraphicsPipelines vk_create_graphics_pipelines =
            (PFN_vkCreateGraphicsPipelines)vk_get_device_proc_addr(device, "vkCreateGraphicsPipelines");
        PFN_vkDestroyPipeline vk_destroy_pipeline =
            (PFN_vkDestroyPipeline)vk_get_device_proc_addr(device, "vkDestroyPipeline");

        device_fps->vk_get_device_proc_addr = vk_get_device_proc_addr;
        device_fps->vk_destroy_device = vk_destroy_device;
        device_fps->vk_get_device_queue = vk_get_device_queue;
        device_fps->vk_create_swapchain = vk_create_swapchain;
        device_fps->vk_destroy_swapchain = vk_destroy_swapchain;
        device_fps->vk_get_swapchain_images = vk_get_swapchain_images;
        device_fps->vk_create_image_view = vk_create_image_view;
        device_fps->vk_destroy_image_view = vk_destroy_image_view;
        device_fps->vk_create_shader_module = vk_create_shader_module;
        device_fps->vk_destroy_shader_module = vk_destroy_shader_module;
        device_fps->vk_create_pipeline_layout = vk_create_pipeline_layout;
        device_fps->vk_destroy_pipeline_layout = vk_destroy_pipeline_layout;
        device_fps->vk_create_render_pass = vk_create_render_pass;
        device_fps->vk_destroy_render_pass = vk_destroy_render_pass;
        device_fps->vk_create_graphics_pipelines = vk_create_graphics_pipelines;
        device_fps->vk_destroy_pipeline = vk_destroy_pipeline;
        return device;
    } else {
        return 0;
    }
}

static void *read_spv(char *file_name, usize *bytes_read) {
    void *memory = 0;
    HANDLE file_handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;

        if (GetFileSizeEx(file_handle, &file_size)) {
            u32 file_size32 = (u32)file_size.QuadPart;
            memory = VirtualAlloc(0, file_size32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

            if (memory) {
                u32 number_of_bytes_read;

                if (ReadFile(file_handle, memory, file_size32, &number_of_bytes_read, 0)
                        && (number_of_bytes_read == file_size32))
                {
                    *bytes_read = (usize)number_of_bytes_read;
                    OutputDebugStringA("File read successfully");
                } else {
                    VirtualFree(memory, 0, MEM_RELEASE);
                    memory = 0;
                }

            } else {}

        } else {}

        CloseHandle(file_handle);
    } else {}

    return memory;
}

static VkShaderModule new_shader_module(VkDevice device, DeviceFunctions device_fps, void *code, usize code_size) {
    VkShaderModuleCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.codeSize = code_size;
    create_info.pCode = (u32 *)code;

    VkShaderModule shader_module = 0;
    VkResult result = device_fps.vk_create_shader_module(device, &create_info, 0, &shader_module);

    if (result == VK_SUCCESS) {
        return shader_module;
    } else {
        return 0;
    }
}

static VkRenderPass new_render_pass(VkDevice device, DeviceFunctions device_fps) {
    VkAttachmentDescription color_attachment;
    color_attachment.flags = 0;
    color_attachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref;
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description;
    subpass_description.flags = 0;
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.inputAttachmentCount = 0;
    subpass_description.pInputAttachments = 0;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_attachment_ref;
    subpass_description.pResolveAttachments = 0;
    subpass_description.pDepthStencilAttachment = 0;
    subpass_description.preserveAttachmentCount = 0;
    subpass_description.pPreserveAttachments = 0;

    VkRenderPassCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass_description;
    create_info.dependencyCount = 0;
    create_info.pDependencies = 0;

    VkRenderPass render_pass;
    VkResult result = device_fps.vk_create_render_pass(device, &create_info, 0, &render_pass);

    if (result == VK_SUCCESS) {
        return render_pass;
    } else {
        return 0;
    }
}

static void new_graphics_pipeline(
    VkDevice device,
    DeviceFunctions device_fps,
    VkPipelineLayout *layout,
    VkRenderPass *render_pass,
    VkPipeline *graphics_pipeline
) {
    usize vert_shader_code_size;
    void *vert_shader_code = read_spv("C:/Users/emirh/Development/C/vulkan-tutorial/build/vert.spv", &vert_shader_code_size);
    usize frag_shader_code_size;
    void *frag_shader_code = read_spv("C:/Users/emirh/Development/C/vulkan-tutorial/build/frag.spv", &frag_shader_code_size);

    VkShaderModule vert_shader_module = new_shader_module(device, device_fps, vert_shader_code, vert_shader_code_size);
    VkShaderModule frag_shader_module = new_shader_module(device, device_fps, frag_shader_code, frag_shader_code_size);

    if (vert_shader_module && frag_shader_module) {
        VkPipelineShaderStageCreateInfo vert_stage;
        vert_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_stage.pNext = 0;
        vert_stage.flags = 0;
        vert_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_stage.module = vert_shader_module;
        vert_stage.pName = "main";
        vert_stage.pSpecializationInfo = 0;

        VkPipelineShaderStageCreateInfo frag_stage;
        frag_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_stage.pNext = 0;
        frag_stage.flags = 0;
        frag_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_stage.module = frag_shader_module;
        frag_stage.pName = "main";
        frag_stage.pSpecializationInfo = 0;

        VkPipelineShaderStageCreateInfo stages[2] = {
            vert_stage,
            frag_stage
        };

        VkPipelineVertexInputStateCreateInfo vertex_input_state;
        vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state.pNext = 0;
        vertex_input_state.flags = 0;
        vertex_input_state.vertexBindingDescriptionCount = 0;
        vertex_input_state.pVertexBindingDescriptions = 0;
        vertex_input_state.vertexAttributeDescriptionCount = 0;
        vertex_input_state.pVertexAttributeDescriptions = 0;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
        input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_state.pNext = 0;
        input_assembly_state.flags = 0;
        input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_state.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = 800.0f;
        viewport.height = 600.0f;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = 800;
        scissor.extent.height = 600;

        VkPipelineViewportStateCreateInfo viewport_state;
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.pNext = 0;
        viewport_state.flags = 0;
        viewport_state.viewportCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.scissorCount = 1;
        viewport_state.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterization_state;
        rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state.pNext = 0;
        rasterization_state.flags = 0;
        rasterization_state.depthClampEnable = VK_FALSE;
        rasterization_state.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterization_state.depthBiasEnable = VK_FALSE;
        rasterization_state.depthBiasConstantFactor = 0.0f;
        rasterization_state.depthBiasClamp = 0.0f;
        rasterization_state.depthBiasSlopeFactor = 0.0f;
        rasterization_state.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample_state;
        multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state.pNext = 0;
        multisample_state.flags = 0;
        multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisample_state.sampleShadingEnable = VK_FALSE;
        multisample_state.minSampleShading = 1.0f;
        multisample_state.pSampleMask = 0;
        multisample_state.alphaToCoverageEnable = VK_FALSE;
        multisample_state.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState attachment;
        attachment.blendEnable = VK_FALSE;
        attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachment.colorBlendOp = VK_BLEND_OP_ADD;
        attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        attachment.alphaBlendOp = VK_BLEND_OP_ADD;
        attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
            | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT
            | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo color_blend_state;
        color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state.pNext = 0;
        color_blend_state.flags = 0;
        color_blend_state.logicOpEnable = VK_FALSE;
        color_blend_state.logicOp = VK_LOGIC_OP_COPY;
        color_blend_state.attachmentCount = 1;
        color_blend_state.pAttachments = &attachment;
        color_blend_state.blendConstants[0] = 0.0f;
        color_blend_state.blendConstants[1] = 0.0f;
        color_blend_state.blendConstants[2] = 0.0f;
        color_blend_state.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo layout_create_info;
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.pNext = 0;
        layout_create_info.flags = 0;
        layout_create_info.setLayoutCount = 0;
        layout_create_info.pSetLayouts = 0;
        layout_create_info.pushConstantRangeCount = 0;
        layout_create_info.pPushConstantRanges = 0;

        device_fps.vk_create_pipeline_layout(device, &layout_create_info, 0, layout);

        *render_pass = new_render_pass(device, device_fps);

        VkGraphicsPipelineCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.pNext = 0;
        create_info.flags = 0;
        create_info.stageCount = 2;
        create_info.pStages = stages;
        create_info.pVertexInputState = &vertex_input_state;
        create_info.pInputAssemblyState = &input_assembly_state;
        create_info.pTessellationState = 0;
        create_info.pViewportState = &viewport_state;
        create_info.pRasterizationState = &rasterization_state;
        create_info.pMultisampleState = &multisample_state;
        create_info.pDepthStencilState = 0;
        create_info.pColorBlendState = &color_blend_state;
        create_info.pDynamicState = 0;
        create_info.layout = *layout;
        create_info.renderPass = *render_pass;
        create_info.subpass = 0;
        create_info.basePipelineHandle = 0;
        create_info.basePipelineIndex = -1;

        device_fps.vk_create_graphics_pipelines(device, 0, 1, &create_info, 0, graphics_pipeline);

        device_fps.vk_destroy_shader_module(device, vert_shader_module, 0);
        device_fps.vk_destroy_shader_module(device, frag_shader_module, 0);
    }
}

int CALLBACK WinMain(HINSTANCE windows_instance, HINSTANCE prev_instance, LPSTR command_line, int show_code) {
    HMODULE libvulkan = LoadLibraryA("vulkan-1.dll");
    if (!libvulkan) {
        OutputDebugStringA("Could not load library\n");
        return 1;
    }
    EntryPoint entry;
    init_entry_point(libvulkan, &entry);

    InstanceFunctions instance_fps;
    VkInstance instance = new_instance(&entry, &instance_fps);
    if (!instance) {
        OutputDebugStringA("Could not create instance\n");
        return 1;
    }
    VkDebugUtilsMessengerEXT debug_messenger = new_debug_messenger(instance, instance_fps);
    if (!debug_messenger) {
        OutputDebugStringA("Could not create debug messenger\n");
        return 1;
    }
    VkPhysicalDevice gpu;
    u32 gpu_count;
    instance_fps.vk_enumerate_physical_devices(instance, &gpu_count, 0);
    instance_fps.vk_enumerate_physical_devices(instance, &gpu_count, &gpu);
    VkPhysicalDeviceProperties gpu_properties;
    instance_fps.vk_get_physical_device_properties(gpu, &gpu_properties);
    OutputDebugStringA(gpu_properties.deviceName);

    u32 queue_family_count;
    instance_fps.vk_get_physical_device_queue_family_properties(gpu, &queue_family_count, 0);
    VkQueueFamilyProperties *queue_families = VirtualAlloc(
        0,
        sizeof(VkQueueFamilyProperties) * queue_family_count,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    instance_fps.vk_get_physical_device_queue_family_properties(gpu, &queue_family_count, queue_families);

    usize graphics_family_index = 0;
    for (usize i = 0; i < queue_family_count; i++) {
        VkQueueFlags flags = queue_families[i].queueFlags;
        if (flags & VK_QUEUE_GRAPHICS_BIT) {
            graphics_family_index = i;
        }
    }
    DeviceFunctions device_fps;
    VkDevice device = new_device(gpu, graphics_family_index, instance_fps, libvulkan, &device_fps);
    if (!device) {
        OutputDebugStringA("Could not create device\n");
        return 1;
    }
    VkQueue queue;
    device_fps.vk_get_device_queue(device, graphics_family_index, 0, &queue);

    WNDCLASSA window_class;
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = main_window_callback;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = windows_instance;
    window_class.hIcon = 0;
    window_class.hCursor = 0;
    window_class.hbrBackground = 0;
    window_class.lpszMenuName = 0;
    window_class.lpszClassName = "HelloTriangleWindowClass";

    if (!RegisterClassA(&window_class)) {
        OutputDebugStringA("Could not register window class\n");
        return 1;
    }
    RECT client_rect;
    client_rect.left = 0;
    client_rect.right = 800;
    client_rect.top = 0;
    client_rect.bottom = 600;

    BOOL result = AdjustWindowRectEx(&client_rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0);
    i32 required_width = client_rect.right - client_rect.left;
    i32 required_height = client_rect.bottom - client_rect.top;
    HWND window = CreateWindowExA(
        0,
        window_class.lpszClassName,
        "Hello Triangle",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        required_width,
        required_height,
        0,
        0,
        windows_instance,
        0
    );

    if (!window) {
        OutputDebugStringA("Could not create window\n");
        return 1;
    }

    VkWin32SurfaceCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = 0;
    create_info.flags = 0;
    create_info.hinstance = windows_instance;
    create_info.hwnd = window;

    VkSurfaceKHR surface;
    instance_fps.vk_create_win32_surface(instance, &create_info, 0, &surface);

    VkBool32 supported;
    instance_fps.vk_get_physical_device_surface_support(gpu, graphics_family_index, surface, &supported);
    if (!supported) {
        OutputDebugStringA("This queue family does not support presentation of given surface\n");
        return 1;
    }

    VkSurfaceCapabilitiesKHR surface_capabilities;
    instance_fps.vk_get_physical_device_surface_capabilities(gpu, surface, &surface_capabilities);

    u32 surface_format_count;
    instance_fps.vk_get_physical_device_surface_formats(gpu, surface, &surface_format_count, 0);
    VkSurfaceFormatKHR *surface_formats = VirtualAlloc(0, sizeof(VkSurfaceFormatKHR) * surface_format_count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    instance_fps.vk_get_physical_device_surface_formats(gpu, surface, &surface_format_count, surface_formats);

    u32 present_mode_count;
    instance_fps.vk_get_physical_device_surface_present_modes(gpu, surface, &present_mode_count, 0);
    VkPresentModeKHR *present_modes = VirtualAlloc(0, sizeof(VkPresentModeKHR) * present_mode_count, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    instance_fps.vk_get_physical_device_surface_present_modes(gpu, surface, &present_mode_count, present_modes);

    VkSurfaceFormatKHR surface_format;
    for (usize i = 0; i < surface_format_count; i++) {
        VkSurfaceFormatKHR sf = surface_formats[i];
        if (sf.format == VK_FORMAT_B8G8R8A8_SRGB && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surface_format = sf;
            break;
        }
    }

    VkPresentModeKHR present_mode;
    for (usize i = 0; i < present_mode_count; i++) {
        VkPresentModeKHR pm = present_modes[i];
        if (pm == VK_PRESENT_MODE_MAILBOX_KHR) {
            present_mode = pm;
            break;
        }
    }

    VkExtent2D extent;
    if (surface_capabilities.currentExtent.width != 0xFFFFFFFFU) {
        extent = surface_capabilities.currentExtent;
    } else {
        u32 min_width = (surface_capabilities.maxImageExtent.width < 800) ? surface_capabilities.maxImageExtent.width : 800;
        u32 max_width = (surface_capabilities.minImageExtent.width < min_width) ? min_width : surface_capabilities.minImageExtent.width;

        u32 min_height = (surface_capabilities.maxImageExtent.height < 600) ? surface_capabilities.maxImageExtent.height : 600;
        u32 max_height = (surface_capabilities.minImageExtent.height < min_height) ? min_height : surface_capabilities.minImageExtent.height;

        extent.width = max_width;
        extent.height = max_height;
    }

    u32 image_count = surface_capabilities.minImageCount + 1;
    if (surface_capabilities.maxImageCount > 0 && image_count > surface_capabilities.maxImageCount) {
        image_count = surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_create_info;
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = 0;
    swapchain_create_info.flags = 0;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = surface_format.format;
    swapchain_create_info.imageColorSpace = surface_format.colorSpace;
    swapchain_create_info.imageExtent = extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = 0;
    swapchain_create_info.preTransform = surface_capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = 0;

    VkSwapchainKHR swapchain;
    device_fps.vk_create_swapchain(device, &swapchain_create_info, 0, &swapchain);

    u32 swapchain_image_count;
    device_fps.vk_get_swapchain_images(device, swapchain, &swapchain_image_count, 0);
    VkImage *swapchain_images = VirtualAlloc(
        0,
        sizeof(VkImage) * swapchain_image_count,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );
    device_fps.vk_get_swapchain_images(device, swapchain, &swapchain_image_count, swapchain_images);

    VkImageView *views = VirtualAlloc(
        0,
        sizeof(VkImageView) * swapchain_image_count,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE
    );

    for (usize i = 0; i < swapchain_image_count; i++) {
        VkImageViewCreateInfo view_create_info;
        view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_create_info.pNext = 0;
        view_create_info.flags = 0;
        view_create_info.image = swapchain_images[i];
        view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_create_info.format = surface_format.format;
        view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_create_info.subresourceRange.baseMipLevel = 0;
        view_create_info.subresourceRange.levelCount = 1;
        view_create_info.subresourceRange.baseArrayLayer = 0;
        view_create_info.subresourceRange.layerCount = 1;

        VkResult result = device_fps.vk_create_image_view(device, &view_create_info, 0, &views[i]);
        if (result != VK_SUCCESS) {
            OutputDebugStringA("Could not create image view\n");
            return 1;
        }
    }

    VkPipelineLayout layout;
    VkRenderPass render_pass;
    VkPipeline graphics_pipeline;
    new_graphics_pipeline(device, device_fps, &layout, &render_pass, &graphics_pipeline);

    RUNNING = 1;
    while (RUNNING) {
        MSG message;
        while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE)) {
            if (message.message == WM_QUIT) {
                RUNNING = 0;
            }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }

    // -------------------------------------------------------------------------
    device_fps.vk_destroy_pipeline(device, graphics_pipeline, 0);
    device_fps.vk_destroy_render_pass(device, render_pass, 0);
    device_fps.vk_destroy_pipeline_layout(device, layout, 0);
    for (usize i = 0; i < swapchain_image_count; i++) {
        device_fps.vk_destroy_image_view(device, views[i], 0);
    }
    device_fps.vk_destroy_swapchain(device, swapchain, 0);
    instance_fps.vk_destroy_surface(instance, surface, 0);
    device_fps.vk_destroy_device(device, 0);
    instance_fps.vk_destroy_debug_utils_messenger(instance, debug_messenger, 0);
    instance_fps.vk_destroy_instance(instance, 0);
    FreeLibrary(libvulkan);
    return 0;
}
