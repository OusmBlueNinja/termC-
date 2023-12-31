#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstring>
#include <sstream>
#include <readline/readline.h>
#include <readline/history.h>
#include <filesystem>
#include <pwd.h>   // Add this header for getpwuid
#include <grp.h>   // Add this header for getgrgid
#include <dlfcn.h>






// ANSI escape codes for colors
const std::string greenColor = "\033[1;32m";
const std::string redColor = "\033[1;31m";
const std::string whiteColor = "\033[0m";
const std::string blueColor = "\033[1;34m";
const std::string orangeColor = "\033[1;33m"; 



void displayErrorMessage(const std::string& message, const std::string& explanation) {
    std::cout << redColor << "Error: " << message << whiteColor << "\n";
    std::cout << explanation << "\n";
}

void displayFileInfo(const std::string& filePath) {
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0) {
        std::string fileName = filePath.substr(filePath.find_last_of("/") + 1);

        struct passwd *pw = getpwuid(fileStat.st_uid);
        struct group *gr = getgrgid(fileStat.st_gid);

        bool isHidden = (fileName.size() > 0 && fileName[0] == '.');
        
        std::string timeStr = ctime(&fileStat.st_mtime);
        if (!timeStr.empty() && timeStr.back() == '\n') {
            timeStr.pop_back(); // Remove newline character
        }
        
        std::cout << std::left
                  << std::setw(11)
                  << ((fileStat.st_mode & S_IRUSR) ? greenColor + "r" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IWUSR) ? blueColor + "w" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IXUSR) ? (isHidden ? orangeColor : greenColor) + "x" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IRGRP) ? greenColor + "r" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IWGRP) ? blueColor + "w" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IXGRP) ? (isHidden ? orangeColor : greenColor) + "x" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IROTH) ? greenColor + "r" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IWOTH) ? blueColor + "w" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IXOTH) ? (isHidden ? orangeColor : greenColor) + "x" : whiteColor + "-")
                  << " " << whiteColor
                  << std::right
                  << std::setw(4) << fileStat.st_nlink << " "
                  << greenColor << std::setw(8) << pw->pw_name << whiteColor << " "
                  << greenColor << std::setw(8) << gr->gr_name << whiteColor << " "
                  << blueColor << std::setw(8) << fileStat.st_size << whiteColor << " "
                  << timeStr << " "
                  << (S_ISDIR(fileStat.st_mode) ? blueColor : (fileStat.st_mode & S_IXUSR) ? (isHidden ? orangeColor : greenColor) : whiteColor)
                  << fileName << whiteColor << " " << "\n";

    } else {
        displayErrorMessage("Error getting file information", "Unable to get information about the file.");
    }
}




void mkFile(const std::string& fileName) {
    std::ofstream outfile (fileName.c_str());
    outfile.close();
}



class PackageManager {
public:

    void installPackage(const std::string& packageName) {
        installCommand(packageName);
    }

    void listInstalledPackages() const {
        std::cout << "Installed packages:\n";
        for (const std::string& packageName : installedPackages) {
            std::cout << "- " << packageName << "\n";
        }
    }
    

private:
    std::vector<std::string> installedPackages;
    std::map<std::string, void*> loadedCommands; // Add this line

    void installCommand(const std::string& commandName) {
        std::string sourceFilePath = "commands/" + commandName + ".hpp";
        std::string destFilePath = "packages/" + commandName + ".so";

        if (std::filesystem::exists(sourceFilePath)) {
            std::string compileCommand = "g++ -shared -o " + destFilePath + " " + sourceFilePath;
            if (std::filesystem::exists(destFilePath)) {
                loadCommand(commandName);
                return;
            } 
            else if (system(compileCommand.c_str()) == 0) {
                loadCommand(commandName);
            } else {
                std::cerr << "Error compiling command: " << commandName << std::endl;
            }
        } else {
            std::cerr << "Command source not found: " << commandName << std::endl;
        }
    }

    void loadCommand(const std::string& commandName) {
        std::string libraryPath = "packages/" + commandName + ".so";
        void* libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (libraryHandle) {
            loadedCommands[commandName] = libraryHandle;
        } else {
            std::cerr << "Error loading library: " << dlerror() << std::endl;
        }
    }
};

class Shell {
public:
    Shell() {
        packageManager = PackageManager();
    }

    void run() {
        while (true) {
            char* line = readline(getPrompt().c_str());
            if (line == nullptr) {
                break; // Exit on Ctrl+D or "exit"
            }
            
            std::string command(line);
            free(line);
            if (command.empty()) {
                continue; // Empty line, prompt again
            }

            handleInput(command);
        }
    }
    void installCommand(const std::string& commandName) {
        std::string sourceFilePath = "commands/" + commandName + ".hpp";
        std::string destFilePath = "packages/" + commandName + ".so";

        if (std::filesystem::exists(sourceFilePath)) {
            std::string compileCommand = "g++ -shared -o " + destFilePath + " " + sourceFilePath;
            if (system(compileCommand.c_str()) == 0) {
                loadCommand(commandName);
            } else {
                std::cerr << "Error compiling command: " << commandName << std::endl;
            }
        } else {
            std::cerr << "Command source not found: " << commandName << std::endl;
        }
    }

    void loadCommand(const std::string& commandName) {
        std::string libraryPath = "packages/lib" + commandName + ".so";
        void* libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (libraryHandle) {
            loadedCommands[commandName] = libraryHandle;
        } else {
            std::cerr << "Error loading library: " << dlerror() << std::endl;
        }
    }

private:
    PackageManager packageManager; // Declare and initialize here
    std::map<std::string, void*> loadedCommands; // Add this line

    std::string getPrompt() {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        std::string greenColor = "\033[1;32m"; // Green color
        std::string blueColor = "\033[1;34m";  // Blue color
        std::string whiteColor = "\033[0m";    // Reset color

        char* username = getenv("USER"); // Get the username
        std::string path = std::string(cwd); // get path
        const char* homeDir = getenv("HOME");
        return whiteColor + "┌ " + greenColor + std::string(username) + whiteColor + ":" + blueColor + std::string(path) + whiteColor + "\n└ $ ";
    }


    void handleInput(const std::string& input) {
    std::vector<std::string> tokens = splitInput(input);
    if (tokens.empty()) {
        return;
    } 
    std::string command = tokens[0];
    if (command == "exit") {
        exit(0);
    }
    else if (command == "list") {
        packageManager.listInstalledPackages(); // Use packageManager here
    }
    else if (command == "install") {
        if (tokens.size() > 1) {
            std::string packageName = tokens[1];
            packageManager.installPackage(packageName); // Use packageManager here
        } else {
            displayErrorMessage("Package name not provided", "Please provide the name of the package to install.");
        }
    }
        else if (command == "clear") {
            clearScreen();
        }
        else if (command == "cd") {
            if (tokens.size() < 2 || tokens[1] == "~") {
                const char* homeDir = getenv("HOME");
                if (homeDir) {
                    if (chdir(homeDir) != 0) {
                        displayErrorMessage("Error changing to home directory", "Unable to change to the home directory.");
                    } 
                } else {
                    displayErrorMessage("Home directory not found", "The HOME environment variable is not set.");
                }
            } else {
                if (chdir(tokens[1].c_str()) != 0) {
                    displayErrorMessage("Error changing directory", "Unable to change to the specified directory.");
                }
            }
        }
        else if (command == "ll") {
            listFilesInDirectory(".");
        }
        else if (command == "ls") {
            listShort(".");
        } 
        else if (command == "rm") {
            if (tokens.size() < 2) {
                displayErrorMessage("File not Specified", "No file specified to be removed");
            } else {
                for (int i = 1; i < tokens.size(); i++) {
                    if (std::filesystem::exists(tokens[i])) {
                        //std::cout << tokens[i].c_str() << std::endl;
                        if (remove(tokens[1].c_str()) != 0) {
                            std::string errorMessage = "The specified file was not able to be removed: ";
                            displayErrorMessage("Unable to Remove File", errorMessage + tokens[i]);
                        }
                    } else {
                        displayErrorMessage("File not found", "The specified file was not found");
                    }
                }

            }

        }
        else if (command == "toutch") {
            mkFile(tokens[1].c_str());
            
        } 
        else {
            displayErrorMessage("Command not found", "The specified command is not recognized.");
        }
    }

    std::vector<std::string> splitInput(const std::string& input) {
        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }
    void listFilesInDirectory(const std::string& dirPath) {
    DIR* dir;
    struct dirent* entry;
    if ((dir = opendir(dirPath.c_str())) != nullptr) {
        std::cout << "total "; // Start the line with "total" to mimic `ls -l` behavior
        long totalSize = 0; // Total size of all files in the directory

        while ((entry = readdir(dir)) != nullptr) {
                std::string filePath = dirPath + "/" + entry->d_name;
                struct stat fileStat;
                if (stat(filePath.c_str(), &fileStat) == 0) {
                    totalSize += fileStat.st_size;
                }
            
        }

        // Print the total size in kilobytes
        std::cout << (totalSize + 1023) / 1024 << "\n";

        // Close and reopen the directory to start iterating over entries again
        closedir(dir);
        dir = opendir(dirPath.c_str());

        while ((entry = readdir(dir)) != nullptr) {
                std::string filePath = dirPath + "/" + entry->d_name;
                displayFileInfo(filePath);
            
        }

        // Close the directory
        closedir(dir);
        std::cout << "\n";
    } else {
        displayErrorMessage("Error opening directory", "Unable to open the specified directory.");
    }
}
    void clearScreen() {
        std::cout << "\033[H\033[2J"; // ANSI escape codes to clear the screen
        std::cout.flush();
    }
    
    void listShort(const std::string& dirPath) {
    DIR* dir;
    struct dirent* entry;
    if ((dir = opendir(dirPath.c_str())) != nullptr) {
        while ((entry = readdir(dir)) != nullptr) {
            // Exclude hidden files and directories
            if (entry->d_name[0] != '.') {
                std::string filePath = dirPath + "/" + entry->d_name;
                struct stat fileStat;
                if (stat(filePath.c_str(), &fileStat) == 0) {
                    std::string fileName = entry->d_name;
                    if (S_ISDIR(fileStat.st_mode)) {
                        std::cout << blueColor << fileName << "/" << whiteColor << " ";
                    } else if (fileStat.st_mode & S_IXUSR) {
                        std::cout << greenColor << fileName << "*" << whiteColor << " ";
                    } else {
                        std::cout << whiteColor << fileName << " ";
                    }
                }
            }
        }
        closedir(dir);
        std::cout << "\n";
    } else {
        displayErrorMessage("Error opening directory", "Unable to open the specified directory.");
    }
    }

};


int main() {
    Shell shell;
    shell.run();
    return 0;
}
