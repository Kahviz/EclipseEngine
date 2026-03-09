#pragma once
#include "ErrorHandling/ErrorMessage.h"
#include <format>
#include <string>
#include <source_location>

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

	#define UGE_ASSERT(condition, message) \
			UGE_ASSERT_CONDITION(condition, message, #condition)

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
#else
	#define UGE_ASSERT(condition, message) ((void)0)
	#define UGE_VK_ASSERT(handle, message) ((void)0)
#endif