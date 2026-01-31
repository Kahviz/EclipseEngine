#include <iostream>
#include "Runtime/Engine.h"
#include "SetupPaths/MakeFiles.h"

int main() {
    MakeFiles mf;
    mf.MakeAPPDATAFolders();
    try {
        Engine engine;
        engine.EngineRun();
    }
    catch (const std::exception& e) {
        std::cerr << "Virhe: " << e.what() << std::endl;
        return -5;
    }

    std::cout << "This is the end..." << std::endl;
    return 0;
}