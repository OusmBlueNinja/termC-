#include <iostream>
#include <vector>
#include <string>

class EchoCommand {
public:
    void execute(const std::vector<std::string>& args) {
        for (const std::string& arg : args) {
            std::cout << arg << " ";
        }
        std::cout << std::endl;
    }
};

