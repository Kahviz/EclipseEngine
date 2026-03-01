#include <iostream>
#include "Runtime/Engine.h"
#include "SetupPaths/MakeFiles.h"
#include "Misc/Profiler/Profiler.h"
#include <thread>

int main() {
    MakeFiles mf;

    mf.MakeAPPDATAFolders();

#if INEDITOR == 0
    #ifdef NDEBUG
        std::cout << "You are in release mode and InEditor Flag is 0 IF you want info and Editor Gui goto GLOBALS and InEditor = 1 and Go In Debug Option... I made this so it's harder to make cheats!" << std::endl;
    #endif
#endif

    try {
        Engine engine;
        engine.EngineRun();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return -5;
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "This is the end..." << std::endl;

    return 0;
}