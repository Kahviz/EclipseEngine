// Globals.cpp
#include "Globals.h"
#include <cstdlib>
#include <string>
#include <iostream>
#include "ErrorHandling/ErrorMessage.h"

int screen_width = 800;
int screen_height = 400;
float zFar = 1000.0f;
extern int viewport_width = 400;
extern int viewport_height = 200;

bool InEditor = true;
bool vSync = true;
bool Running = true;
bool Typing = false;
float FOV = 100.0f;

int Index = 0;

std::string GetAppDataPath() {
    char* buffer = nullptr;
    size_t size = 0;
    #ifdef _WIN32
        if (_dupenv_s(&buffer, &size, "APPDATA") == 0 && buffer != nullptr) {
            MakeASuccess("AppData Found!");
            std::string path(buffer);
            free(buffer);
            return path;
        }
        else {
            MakeAError("AppData Not Found in GLOBALS.h with _WIN32");
            std::exit(10);
        }
    #else
        const char* homeDir = getenv("HOME");
        if (!homeDir) {
            std::cerr << "Failed to get HOME directory\n";
            return;
        }
        fs::path appDataTarget = fs::path(homeDir) / ".config" / "UntilitedGameEngine";

        std::cout << "HOME directory found: " << appDataTarget << "\n";

        return appDataTarget;
    #endif

    return "";
}

fs::path GetAppDataDir() {
#ifdef _WIN32
    char* appDataPath = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&appDataPath, &sz, "APPDATA") == 0 && appDataPath != nullptr) {
        fs::path appdatatarget = fs::path(appDataPath) / "UntilitedGameEngine";

        #ifdef _DEBUG
                std::cout << "APPDATA path found: " << appdatatarget << "\n";
        #endif // _DEBUG

        free(appDataPath);
        return appdatatarget;
    }
    else {
        MakeAError("Failed to get APPDATA path");
        if (appDataPath) free(appDataPath);
        return fs::path();
    }
#else
    const char* homeDir = getenv("HOME");
    if (homeDir) {
        fs::path appdatatarget = fs::path(homeDir) / ".config" / "UntilitedGameEngine";
        std::cout << "HOME directory found: " << appdatatarget << "\n";
        return appdatatarget;
    }
    else {
        MakeAError("Failed to get HOME directory");
        return fs::path();
    }
#endif
}

std::string ProjectName = "ProjectTest1";
std::string appData = GetAppDataPath();
std::string assets = appData + "\\UntilitedGameEngine\\Assets";
std::string fonts = appData + "\\UntilitedGameEngine\\Fonts";

fs::path appDataTarget = GetAppDataDir();

