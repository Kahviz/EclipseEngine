#include <iostream>
#include "Runtime/Engine.h"
#include "SetupPaths/MakeFiles.h"
#include <Debugging/Profiler/Profiler.h>
#include <thread>
#include "ErrorHandling/ErrorMessage.h"
#include <format>
#include "BoronMathLibrary.h"

int main() {
    MakeAInfo("BoronEngine started!");

    MakeFiles mf;
    mf.MakeAPPDATAFolders();

    const int WaitTime = 4;

#if INEDITOR == 0
#ifdef NDEBUG
    std::cout << "\033[1;33m[INFO]\033[0m You are in release mode and InEditor Flag is 0." << std::endl;
    std::cout << "\033[1;33m[INFO]\033[0m If you want Editor Gui, set InEditor = 1 in GLOBALS and use Debug mode." << std::endl;
    std::cout << "\033[1;33m[INFO]\033[0m This makes it harder to make cheats!" << std::endl;
#endif
#endif

    try {
        Engine engine;
        engine.EngineRun();
    }
    catch (const std::exception& e) {
        std::cerr << "\033[1;31m[ERROR]\033[0m " << e.what() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return -5;
    }
    catch (...) {
        std::cerr << "\033[1;31m[ERROR]\033[0m Unknown exception occurred!" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return -6;
    }

    std::string WaitTimeString = std::format("Waiting For {} second(s) before closing!", WaitTime);

    MakeASuccess(WaitTimeString);
    std::this_thread::sleep_for(std::chrono::seconds(WaitTime));

    MakeASuccess("This is the end...");

    return 0;
}