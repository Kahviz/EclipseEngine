#pragma once
#include "ErrorHandling/ErrorMessage.h"
#include <format>
#include <string>
//Nothing

#ifdef _DEBUG
#define UGE_ASSERT(condition, message) \
        if (!(condition)) { \
            std::string AssertString = std::format( \
                "ASSERT failed because {} was null/nullptr - {}", \
                #condition, message); \
            MakeAError(AssertString); \
        }
#else
#define ASSERT(condition, message)
#endif