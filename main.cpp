#include <iostream>
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




// ANSI escape codes for colors
const std::string greenColor = "\033[1;32m";
const std::string redColor = "\033[1;31m";
const std::string whiteColor = "\033[0m";
const std::string blueColor = "\033[1;34m";
const std::string orangeColor = "\033[1;33m"; // Add this line



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

        std::cout << (S_ISDIR(fileStat.st_mode) ? blueColor + "d" : "-")
                  << ((fileStat.st_mode & S_IRUSR) ? greenColor + "r" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IWUSR) ? blueColor + "w" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IXUSR) ? orangeColor + "x" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IRGRP) ? greenColor + "r" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IWGRP) ? blueColor + "w" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IXGRP) ? orangeColor + "x" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IROTH) ? greenColor + "r" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IWOTH) ? blueColor + "w" : whiteColor + "-")
                  << ((fileStat.st_mode & S_IXOTH) ? orangeColor + "x" : whiteColor + "-")
                  << " " << fileStat.st_nlink << " "
                  << greenColor << pw->pw_name << whiteColor << " "
                  << greenColor << gr->gr_name << whiteColor << " "
                  << blueColor << fileStat.st_size << whiteColor << " "
                  << (S_ISDIR(fileStat.st_mode) ? blueColor : (fileStat.st_mode & S_IXUSR) ? orangeColor : whiteColor)
                  << fileName << whiteColor << " "
                  << ctime(&fileStat.st_mtime);

    } else {
        displayErrorMessage("Error getting file information", "Unable to get information about the file.");
    }
}









class PackageManager {
public:
    void installPackage(const std::string& packageName) {
        // Simulate package installation logic
        installedPackages.push_back(packageName);
    }

    void listInstalledPackages() const {
        std::cout << "Installed packages:\n";
        for (const std::string& packageName : installedPackages) {
            std::cout << "- " << packageName << "\n";
        }
    }

private:
    std::vector<std::string> installedPackages;
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

private:
    PackageManager packageManager;

    std::string getPrompt() {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));

        std::string greenColor = "\033[1;32m"; // Green color
        std::string blueColor = "\033[1;34m";  // Blue color
        std::string whiteColor = "\033[0m";    // Reset color

        char* username = getenv("USER"); // Get the username
        return greenColor + std::string(username) + whiteColor + ":" +
               blueColor + std::string(cwd) + whiteColor + " $ ";
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
            packageManager.listInstalledPackages();
        }
        else if (command == "install") {
            // ... (handle package installation)
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
    namespace fs = std::filesystem;

    // Open the directory
    DIR* dir;
    struct dirent* entry;
    if ((dir = opendir(dirPath.c_str())) != nullptr) {
        // Loop through the directory entries
        while ((entry = readdir(dir)) != nullptr) {
            // Exclude hidden files and directories
            if (entry->d_name[0] != '.') {
                std::string filePath = dirPath + "/" + entry->d_name;
                displayFileInfo(filePath);
            }
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
