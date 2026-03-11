// Globals.h
#pragma once
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

#define INEDITOR 1 //Use 1 For Editing 0 For The Release

#define DIRECTX11 0 //1 = True, 0 = False
#define VULKAN 1 //1 = True, 0 = False

#if INEDITOR == 1
	#define PROFILER
#endif

#define AURA // Most Useful thing in this project

extern int screen_width;
extern int screen_height;
extern float zFar;
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
extern bool Typing;
extern float FOV;
