// Globals.h
#pragma once
#include <string>
#define INEDITOR 1 //Laita 1
#define DIRECTX11 0 //1 = true

extern int screen_width;
extern int screen_height;

extern int viewport_width;
extern int viewport_height;

extern std::string appData;
extern std::string fonts;
extern std::string assets;
extern std::string ProjectName;

extern bool InEditor;
extern int Index;
extern bool vSync;
extern bool Running;
extern bool UsesDx11;
extern bool UsesVulkan;
extern float FOV;
