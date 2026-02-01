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

bool siirraKaikki(const fs::path& lahdeHakemisto,
    const fs::path& kohdeParent) {
    try {
        if (!fs::exists(lahdeHakemisto)) return false;
        if (!fs::is_directory(lahdeHakemisto)) return false;

        // Luo kohdehakemisto
        fs::create_directories(kohdeParent);

        for (const auto& entry : fs::directory_iterator(lahdeHakemisto)) {
            if (entry.is_regular_file()) {
                fs::path kohde = kohdeParent / entry.path().filename();
                fs::rename(entry.path(), kohde);
            }
        }

        return true;
    }
    catch (...) {
        return false;
    }
}

bool CopyDirectoryRecursive(const fs::path& sourceDir, const fs::path& targetDir) {
    try {
        std::cout << "\n--- CopyDirectoryRecursive ---\n";
        std::cout << "Source: " << sourceDir << "\n";
        std::cout << "Target: " << targetDir << "\n";

        // Tarkista lähde
        if (!fs::exists(sourceDir)) {
            std::cerr << "Source directory doesn't exist: " << sourceDir << "\n";
            return false;
        }
        if (!fs::is_directory(sourceDir)) {
            std::cerr << "Source is not a directory: " << sourceDir << "\n";
            return false;
        }

        // Luo kohdekansio jos ei ole olemassa
        std::cout << "Creating target directory: " << targetDir << "\n";
        fs::create_directories(targetDir);

        int totalFiles = 0;
        int totalDirs = 0;

        // Käy läpi kaikki tiedostot ja alihakemistot
        for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
            try {
                const auto& sourcePath = entry.path();
                auto relativePath = fs::relative(sourcePath, sourceDir);
                auto targetPath = targetDir / relativePath;

                if (entry.is_directory()) {
                    // Luo alihakemisto
                    fs::create_directories(targetPath);
                    totalDirs++;
                    std::cout << "Created directory: " << targetPath << "\n";
                }
                else if (entry.is_regular_file()) {
                    // Kopioi tiedosto
                    fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing);
                    totalFiles++;
                    std::cout << "Copied file: " << sourcePath.filename()
                        << " -> " << targetPath << "\n";
                }
            }
            catch (const fs::filesystem_error& e) {
                std::cerr << "Error processing " << entry.path() << ": " << e.what() << "\n";
                // Jatka muiden tiedostojen käsittelyä
            }
        }

        std::cout << "--- Copy completed ---\n";
        std::cout << "Total directories created: " << totalDirs << "\n";
        std::cout << "Total files copied: " << totalFiles << "\n";

        return (totalFiles > 0 || totalDirs > 0);
    }
    catch (const std::exception& e) {
        std::cerr << "EXCEPTION in CopyDirectoryRecursive: " << e.what() << "\n";
        return false;
    }
}

void MakeFiles::MakeAPPDATAFolders() {
#ifdef INEDITOR == 1
    fs::path currentDir = fs::current_path();
    fs::path parentPath = currentDir.parent_path();
    fs::path AssetTemplateFolder = parentPath / "UntilitedGameEngine";

    std::cout << "\n=== MAKE APPDATA FOLDERS ===\n";

    // Luo oma alihakemisto AppDataan
#ifdef _WIN32
    char* appDataPath = nullptr;
    size_t sz = 0;
    fs::path appDataTarget;
    if (_dupenv_s(&appDataPath, &sz, "APPDATA") == 0 && appDataPath != nullptr) {
        appDataTarget = fs::path(appDataPath) / "UntilitedGameEngine";
        free(appDataPath);
        std::cout << "APPDATA path found: " << appDataTarget << "\n";
    }
    else {
        std::cerr << "Failed to get APPDATA path\n";
        return;
    }
#else
    const char* homeDir = getenv("HOME");
    if (!homeDir) {
        std::cerr << "Failed to get HOME directory\n";
        return;
    }
    fs::path appDataTarget = fs::path(homeDir) / ".config" / "UntilitedGameEngine";
    std::cout << "HOME directory found: " << appDataTarget << "\n";
#endif

    std::cout << "DEBUG INFO:\n";
    std::cout << "currentDir: " << currentDir << "\n";
    std::cout << "parentPath: " << parentPath << "\n";
    std::cout << "AssetTemplateFolder: " << AssetTemplateFolder << "\n";
    std::cout << "AssetTemplateFolder exists: " << fs::exists(AssetTemplateFolder) << "\n";
    std::cout << "AppData Target Folder: " << appDataTarget << "\n";
    std::cout << "AppData Target exists: " << fs::exists(appDataTarget) << std::endl;

    if (fs::exists(AssetTemplateFolder) && fs::is_directory(AssetTemplateFolder)) {
        std::cout << "AssetTemplateFolder EXISTS and is a directory!\n";

        // Tulosta tarkempaa tietoa kansiosta
        std::cout << "\nDetailed content of AssetTemplateFolder:\n";
        try {
            int fileCount = 0;
            int dirCount = 0;
            for (const auto& entry : fs::recursive_directory_iterator(AssetTemplateFolder)) {
                if (entry.is_regular_file()) {
                    std::cout << "  FILE: " << entry.path() << " (size: "
                        << fs::file_size(entry.path()) << " bytes)\n";
                    fileCount++;
                }
                else if (entry.is_directory()) {
                    std::cout << "  DIR: " << entry.path() << "\n";
                    dirCount++;
                }
            }
            std::cout << "Total: " << fileCount << " files, " << dirCount << " directories\n";
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Error reading directory: " << e.what() << "\n";
        }

        std::cout << "\nAttempting to copy files recursively..\n";
        bool success = CopyDirectoryRecursive(AssetTemplateFolder, appDataTarget);

        if (success) {
            std::cout << "SUCCESS: Files copied to: " << appDataTarget << "\n";

            if (fs::exists(appDataTarget) && fs::is_directory(appDataTarget)) {
                std::cout << "\nFiles now in AppData:\n";
                try {
                    int copiedCount = 0;
                    for (const auto& entry : fs::recursive_directory_iterator(appDataTarget)) {
                        if (entry.is_regular_file()) {
                            copiedCount++;
                        }
                    }
                    std::cout << "Total files copied: " << copiedCount << "\n";
                }
                catch (const fs::filesystem_error& e) {
                    std::cerr << "Error reading target directory: " << e.what() << "\n";
                }
            }
            else {
                std::cout << "WARNING: Target directory doesn't exist or is not a directory!\n";
            }
        }
        else {
            std::cout << "FAILED to copy files!\n";
        }
    }
    else {
        std::cout << "ERROR: AssetTemplateFolder DOES NOT EXIST or is not a directory!\n";
        std::cout << "\nContents of parent directory:\n";
        for (const auto& entry : fs::directory_iterator(parentPath)) {
            std::cout << "  " << entry.path().filename()
                << (entry.is_directory() ? " [DIR]" : " [FILE]") << "\n";
        }
    }

    std::cout << "=== END MAKE APPDATA FOLDERS ===\n\n";
#endif
}