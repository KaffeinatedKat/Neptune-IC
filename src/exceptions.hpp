#pragma once
#include <string>

struct Exceptions {
    std::string userNotFound(std::string user) {
        return "No profile for user '" + user + "'\nTry '? profiles' for more information\n";
    };

    std::string notANumber(std::string command) {
        return "Command '" + command + "' takes only numbers as input\n";
    };

    std::string noJson(std::string user) {
        return "Json files for profile '" + user + "' could not be found\n";
    }

    std::string invalidJson(std::string missingValue) {
        return "Json files appear to be invalid, value(s) '" + missingValue + "' do not exist\n";
    }
};
