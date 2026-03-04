#pragma once
#include "ErrorHandling/ErrorMessage.h"
#include <format>
#include <string>

#if VULKAN == 1
    #include "Vulkan/vulkan.h"
#endif

#ifdef _DEBUG
#define UGE_ASSERT(condition, message) \
        if (!(condition)) { \
            std::string AssertString = std::format( \
                "ASSERT failed because {} was null/nullptr - {}", \
                #condition, message); \
            MakeAError(AssertString); \
        }
#define UGE_VK_ASSERT(condition, message) \
        if (condition == VK_NULL_HANDLE) { \
            std::string AssertString = std::format( \
                "VK_ASSERT failed because {} was VK_NULL_HANDLE - {}", \
                #condition, message); \
            MakeAError(AssertString); \
        }
#else
#define ASSERT(condition, message)
#endif