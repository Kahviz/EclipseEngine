#include "MakeFiles.h"
#include "GLOBALS.h"

#include <filesystem>
#include <iostream>

#include <fstream>
#include <string>


namespace fs = std::filesystem;

std::string MakeAFolder(const std::string& parentPath, const std::string& folderName) {
    fs::path fullPath = fs::path(parentPath) / folderName;

    if (!fs::exists(fullPath)) {
        try {
            fs::create_directories(fullPath);
            std::cout << "Folder created: " << fullPath << "\n";
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Error creating folder: " << e.what() << "\n";
            return "";
        }
    }
    else {
        std::cout << "Folder already exists: " << fullPath << "\n";
    }

    return fullPath.string();
}


std::string MakeAFile(const std::string& parentPath, const std::string& fileName, const std::string& content) {
    fs::path fullPath = fs::path(parentPath) / fileName;

    try {
        if (!fs::exists(fullPath.parent_path())) {
            fs::create_directories(fullPath.parent_path());
        }

        std::ofstream file(fullPath);

        if (file.is_open()) {
            file << content;
            file.close();
            std::cout << "File created: " << fullPath << "\n";
        }
        else {
            std::cerr << "Failed to open file for writing: " << fullPath << "\n";
            return "";
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << "\n";
        return "";
    }

    return fullPath.string();
}

void MakeFiles::MakeAPPDATAFolders() {
	#ifdef INEDITOR == 1
        std::string UntilitedFolder = MakeAFolder(appData,"UntilitedGameEngine2");
        std::cout << UntilitedFolder << std::endl;

        std::string AssetsFolder = MakeAFolder(UntilitedFolder, "Assets");
        std::string FontsFolder = MakeAFolder(UntilitedFolder, "Fonts");
        std::string IconsFolder = MakeAFolder(UntilitedFolder, "Icons");
        std::string SavingsFolder = MakeAFolder(UntilitedFolder, "Savings");
	#endif
}