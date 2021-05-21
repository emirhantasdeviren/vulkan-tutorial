#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

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

        device_fps->vk_get_device_proc_addr = vk_get_device_proc_addr;
        device_fps->vk_destroy_device = vk_destroy_device;
        device_fps->vk_get_device_queue = vk_get_device_queue;
        device_fps->vk_create_swapchain = vk_create_swapchain;
        device_fps->vk_destroy_swapchain = vk_destroy_swapchain;
        device_fps->vk_get_swapchain_images = vk_get_swapchain_images;
        device_fps->vk_create_image_view = vk_create_image_view;
        device_fps->vk_destroy_image_view = vk_destroy_image_view;
        return device;
    } else {
        return 0;
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
