// Globals.h
#pragma once
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

#define INEDITOR 1 //Laita 1

#define DIRECTX11 1 //1 = True
#define VULKAN 0

extern int screen_width;
extern int screen_height;

extern int viewport_width;
extern int viewport_height;

extern std::string appData;
extern std::string fonts;
extern std::string assets;
extern std::string ProjectName;

extern fs::path appDataTarget;

extern bool InEditor;
extern int Index;
extern bool vSync;
extern bool Running;
extern float FOV;
