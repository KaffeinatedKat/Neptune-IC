#include <iostream>
#include <cstdlib>
#include <nlohmann/detail/exceptions.hpp>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "ui.hpp"
#include "help_text.cpp"

using json = nlohmann::json;

int main() {
    User Student;
    Exceptions Error;
    UserProfiles Profiles;
    UI Cli;
    std::string input;
    std::string msg = "login with `login [profile]`\nFor help on creating profiles, do `? profiles`\n\n";

    while (true) {
        std::string command[4];
        Cli.newScreen();
        printf("%s", msg.c_str());
        Cli.userInput(command, Student.first_name);
        json grades_json;

        if (command[0] == "login") {
            if (command[1] == "") {
                msg = "You must specify a user\n\n";
                continue;
            }
            Student.profile_name = command[1];
            Student = Student.login(Student, Error);
            if (Student.logged_in != true) {
                msg = Student.error;
                continue;
            }

            Cli.allClassesMenu(Student, Error, Student.grades_json);
            Student.logged_in = false;
            Student.first_name = "None";
            msg = "login with `login [profile]`\nFor help on creating profiles, do `? profiles`\n\n";

        } else if (command[0] == "profiles") {
            if (command[1] == "create") {
                Cli.newProfile(Profiles);
            } else if (command[1] == "list") {
                msg = Cli.listProfiles(Profiles, msg);
            }

        } else if (command[0] == "?") {
            if (command[1] == "profiles") {
                msg = profileAllHelp();
            }
        
        } else if (command[0] == "q") {
            std::exit(0);
        
        } else {
            msg = "Command '" + command[0] + "' not found\n\n";
        
        }
    }
};
