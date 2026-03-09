#include "MakeFiles.h"
#include "GLOBALS.h"
#include "ErrorHandling/ErrorMessage.h"

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
            #ifdef _DEBUG
                std::cout << "Folder created: " << fullPath << "\n";
            #endif // _DEBUG

        }
        catch (const fs::filesystem_error& e) {
            #ifdef _DEBUG
                std::cerr << "Error creating folder: " << e.what() << "\n";
            #endif // _DEBUG

            return "";
        }
    }
    else {
        #ifdef _DEBUG
            std::cout << "Folder already exists: " << fullPath << "\n";
        #endif // _DEBUG
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

            #ifdef _DEBUG
                std::cout << "File created: " << fullPath << "\n";
            #endif
        }
        else {
            #ifdef _DEBUG
                std::cerr << "Failed to open file for writing: " << fullPath << "\n";
            #endif
            return "";
        }
    }
    catch (const fs::filesystem_error& e) {
        #ifdef _DEBUG
            std::cerr << "Filesystem error: " << e.what() << "\n";
        #endif
        return "";
    }

    return fullPath.string();
}

bool siirraKaikki(const fs::path& lahdeHakemisto,
    const fs::path& kohdeParent) {
    try {
        if (!fs::exists(lahdeHakemisto)) return false;
        if (!fs::is_directory(lahdeHakemisto)) return false;

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
        #ifdef _DEBUG
            std::cout << "\n--- CopyDirectoryRecursive ---\n";
                    std::cout << "Source: " << sourceDir << "\n";
                    std::cout << "Target: " << targetDir << "\n";
        #endif  
        

        if (!fs::exists(sourceDir)) {
            #ifdef _DEBUG
                        std::cerr << "Source directory doesn't exist: " << sourceDir << "\n";
            #endif
            return false;
        }

        if (!fs::is_directory(sourceDir)) {
            #ifdef _DEBUG
                std::cerr << "Source is not a directory: " << sourceDir << "\n";
            #endif
            return false;
        }

        #ifdef _DEBUG
            std::cout << "Creating target directory: " << targetDir << "\n";
        #endif

        fs::create_directories(targetDir);

        int totalFiles = 0;
        int totalDirs = 0;

        for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
            try {
                const auto& sourcePath = entry.path();
                auto relativePath = fs::relative(sourcePath, sourceDir);
                auto targetPath = targetDir / relativePath;

                if (entry.is_directory()) {
                    fs::create_directories(targetPath);
                    totalDirs++;
                    #ifdef _DEBUG
                        std::cout << "Created directory: " << targetPath << "\n";
                    #endif
                }
                else if (entry.is_regular_file()) {
                    fs::copy_file(sourcePath, targetPath, fs::copy_options::overwrite_existing);
                    totalFiles++;

                    #ifdef _DEBUG
                        std::cout << "Copied file: " << sourcePath.filename()
                            << " -> " << targetPath << "\n";
                    #endif
                }
            }

            catch (const fs::filesystem_error& e) {
#ifdef _DEBUG
                std::cerr << "Error processing " << entry.path() << ": " << e.what() << "\n";

#endif
            }
        }

        #ifdef _DEBUG
            std::cout << "--- Copy completed ---\n";
            std::cout << "Total directories created: " << totalDirs << "\n";
            std::cout << "Total files copied: " << totalFiles << "\n";
        #endif
        

        return (totalFiles > 0 || totalDirs > 0);
    }
    catch (const std::exception& e) {
#ifdef _DEBUG
        std::cerr << "EXCEPTION in CopyDirectoryRecursive: " << e.what() << "\n";
#endif
        return false;
    }
}

void MakeFiles::MakeAPPDATAFolders() {
#ifdef INEDITOR == 1
    fs::path currentDir = fs::current_path();

    fs::path projectRoot = currentDir;

    while (!fs::exists(projectRoot / "UntilitedGameEngine") &&
        projectRoot.has_parent_path() &&
        projectRoot != projectRoot.root_path()) {
        projectRoot = projectRoot.parent_path();
    }

    fs::path AssetTemplateFolder = projectRoot / "UntilitedGameEngine";


#ifdef _DEBUG
    std::cout << "\nMAKE APPDATA FOLDERS\n";

    std::cout << "DEBUG INFO:\n";
    std::cout << "currentDir: " << currentDir << "\n";
    std::cout << "projectRoot: " << projectRoot << "\n";
    std::cout << "AssetTemplateFolder: " << AssetTemplateFolder << "\n";
    std::cout << "AssetTemplateFolder exists: " << fs::exists(AssetTemplateFolder) << "\n";
    std::cout << "AppData Target Folder: " << appDataTarget << "\n";
    std::cout << "AppData Target exists: " << fs::exists(appDataTarget) << std::endl;
#endif


    if (fs::exists(AssetTemplateFolder) && fs::is_directory(AssetTemplateFolder)) {
#ifdef _DEBUG
        std::cout << "AssetTemplateFolder EXISTS and is a directory!\n";

        std::cout << "\nDetailed content of AssetTemplateFolder:\n";
#endif

        try {
            int fileCount = 0;
            int dirCount = 0;
            for (const auto& entry : fs::recursive_directory_iterator(AssetTemplateFolder)) {
                if (entry.is_regular_file()) {
                    #ifdef _DEBUG
                        std::cout << "  FILE: " << entry.path() << " (size: "
                            << fs::file_size(entry.path()) << " bytes)\n";
                    #endif

                    fileCount++;
                }
                else if (entry.is_directory()) {
                    #ifdef _DEBUG
                        std::cout << "  DIR: " << entry.path() << "\n";
                    #endif
                    dirCount++;
                }
            }
            #ifdef _DEBUG
                std::cout << "Total: " << fileCount << " files, " << dirCount << " directories\n";
            #endif
        }
        catch (const fs::filesystem_error& e) {
            #ifdef _DEBUG
                std::cerr << "Error reading directory: " << e.what() << "\n";
            #endif
        }

        #ifdef _DEBUG
            std::cout << "\nAttempting to copy files recursively...\n";
        #endif
        bool success = CopyDirectoryRecursive(AssetTemplateFolder, appDataTarget);

        if (success) {
            #ifdef _DEBUG
                std::cout << "SUCCESS: Files copied to: " << appDataTarget << "\n";
            #endif

            if (fs::exists(appDataTarget) && fs::is_directory(appDataTarget)) {
                #ifdef _DEBUG
                    std::cout << "\nFiles now in AppData:\n";
                #endif
                try {
                    int copiedCount = 0;
                    for (const auto& entry : fs::recursive_directory_iterator(appDataTarget)) {
                        if (entry.is_regular_file()) {
                            copiedCount++;
                        }
                    }
                    #ifdef _DEBUG
                        std::cout << "Total files copied: " << copiedCount << "\n";
                    #endif
                }
                catch (const fs::filesystem_error& e) {
                    #ifdef _DEBUG
                        std::cerr << "Error reading target directory: " << e.what() << "\n";
                    #endif
                }
            }
            else {
                #ifdef _DEBUG
                    std::cout << "WARNING: Target directory doesn't exist or is not a directory!\n";
                #endif
            }
        }
        else {
            #ifdef _DEBUG
                std::cout << "FAILED to copy files!\n";
            #endif
        }
    }
    else {
        #ifdef _DEBUG
            std::cout << "ERROR: AssetTemplateFolder DOES NOT EXIST or is not a directory!\n";
            std::cout << "\nContents of parent directory:\n";
        #endif

        for (const auto& entry : fs::directory_iterator(projectRoot)) {
            #ifdef _DEBUG
                std::cout << "  " << entry.path().filename()
                                << (entry.is_directory() ? " [DIR]" : " [FILE]") << "\n";
            #endif
            
        }
    }

    #ifdef _DEBUG
        std::cout << "=== END MAKE APPDATA FOLDERS ===\n\n";
    #endif
#endif
}