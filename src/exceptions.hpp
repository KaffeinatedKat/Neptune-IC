#include <string>

struct Exceptions {
    std::string userNotFound(std::string user) {
        return "No profile for user '" + user + "'\nTry '? profiles' for more information\n\n";
    };

    std::string notANumber(std::string command) {
        return "Command '" + command + "' takes only numbers as input\n\n";
    };
};