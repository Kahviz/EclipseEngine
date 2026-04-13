#pragma once
#include "ErrorHandling/ErrorMessage.h"
#include <format>
#include <string>
#include <source_location>
#include "GLOBALS.h"

#if VULKAN == 1
#include "Vulkan/vulkan.h"
#endif

#ifdef _DEBUG
	template<typename T>
	void UGE_ASSERT_PTR(T* ptr, const std::string& message,
		const std::source_location& location = std::source_location::current()) {
		if (ptr == nullptr) {
			std::string errorMsg = std::format("UGE_ASSERT failed: {} is nullptr!\nMessage: {}\nFile: {} (line: {})",
				ptr, message,
				location.file_name(), location.line());
			MakeAError(errorMsg);
		}
	}

	template<typename T>
	void UGE_ASSERT_CONDITION(T&& condition, const std::string& message,
		const std::string& conditionStr,
		const std::source_location& location = std::source_location::current()) {
		if (!condition) {
			std::string errorMsg = std::format("UGE_ASSERT failed: {} is false!\nMessage: {}\nFile: {} (line: {})",
				conditionStr, message,
				location.file_name(), location.line());
			MakeAError(errorMsg);
		}
	}


	template<typename Result>
	void UGE_ASSERT_VKRESULT(Result result, const std::string& message,
		const std::string& conditionStr = "",
		const std::source_location& location = std::source_location::current()) {

		#if VULKAN == 1
				if (result != VK_SUCCESS) {
					std::string errorName;
					switch (result) {
					case VK_ERROR_OUT_OF_HOST_MEMORY: errorName = "VK_ERROR_OUT_OF_HOST_MEMORY"; break;
					case VK_ERROR_OUT_OF_DEVICE_MEMORY: errorName = "VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
					case VK_ERROR_INITIALIZATION_FAILED: errorName = "VK_ERROR_INITIALIZATION_FAILED"; break;
					case VK_ERROR_DEVICE_LOST: errorName = "VK_ERROR_DEVICE_LOST"; break;
					case VK_ERROR_OUT_OF_DATE_KHR: errorName = "VK_ERROR_OUT_OF_DATE_KHR"; break;
					case VK_ERROR_SURFACE_LOST_KHR: errorName = "VK_ERROR_SURFACE_LOST_KHR"; break;
					default: errorName = std::format("VkResult code: {}", static_cast<int>(result)); break;
					}

					std::string errorMsg = std::format(
						"UGE_ASSERT_VKRESULT failed: {}\n"
						"Result: {}\n"
						"Message: {}\n"
						"Location: {} (line: {})",
						conditionStr.empty() ? errorName : conditionStr,
						errorName,
						message,
						location.file_name(),
						location.line()
					);

					MakeAError(errorMsg);
				}
		#endif
	}
	
	template<typename T>
	void UGE_ASSERT_VK_HANDLE(T handle, const std::string& message,
		const std::source_location& location = std::source_location::current()) {
		#if VULKAN == 1
				if (handle == VK_NULL_HANDLE) {
					std::string errorMsg = std::format("UGE_ASSERT failed: Vulkan handle is VK_NULL_HANDLE!\nMessage: {}\nFile: {} (line: {})",
						message,
						location.file_name(), location.line());
					MakeAError(errorMsg);
				}
		#endif
	}

	#define UGE_VK_ASSERT(handle, message) \
		UGE_ASSERT_VK_HANDLE(handle, message)

	#define UGE_ASSERT(condition, message) \
		UGE_ASSERT_CONDITION(condition, message, #condition)

	#define UGE_ASSERT_VKRESULT(VkResult, message) \
		UGE_ASSERT_VKRESULT(VkResult, message, #VkResult)
	
#else
	#define UGE_ASSERT(condition, message) ((void)0)
	#define UGE_VK_ASSERT(handle, message) ((void)0)
	#define UGE_ASSERT_VKRESULT(VkResult, message) ((void)0)
#endif