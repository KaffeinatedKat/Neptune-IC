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
    Options Settings;

    Settings = Settings.load(Settings);
    std::string input;
    std::string defaultMsg = "login with `login [profile]`\nFor help on creating profiles, do `? profiles`\n\n\n";
    std::string msg = defaultMsg;
    bool reset = true;

    while (true) {
        std::string command[4];
        if (reset) { Cli.newScreen("Neptune"); }
        reset = true;
        printf("%s", msg.c_str());
        Cli.userInput(command, Student.first_name);
        json grades_json;

        if (command[0] == "") {
            continue;
        } else if (command[0] == "login") {
            if (command[1] == "") {
                msg = "You must specify a user\n\n";
                continue;
            }
            Student.profile_name = command[1];
            Student.term = "current";
            Student.login(Student, Error); //  Try logging into profile with specified name
            if (Student.logged_in != true) { //  Print error if login unsuccessful
                msg = "\033[A\033[A\33[2K\r" + Student.error;
                reset = false;
                continue;
            }
            Cli.allClassesMenu(Student, Error, Settings, Student.grades_json);
            Student.logged_in = false;
            Student.first_name = "None";
            msg = defaultMsg;
                
        } else if (command[0] == "profiles") {
            if (command[1] == "create") {
                msg = defaultMsg + "\033[A" + Cli.newProfile(Profiles, command[2]);
            } else if (command[1] == "list") {
                msg = Cli.listProfiles(Profiles, msg) + "\n";
            } else if (command[1] == "delete") {
                reset = false;
                msg = "\033[A\033[A\33[2K\r" + Profiles.remove(Profiles, command[2]);
            }

        } else if (command[0] == "?") {
            if (command[1] == "profiles") {
                msg = profileAllHelp();
            }
        
        } else if (command[0] == "q") {
            std::exit(0);
        
        } else {
            reset = false;
            msg = "\033[A\033[A\33[2K\rCommand '" + command[0] + "' not found\n";
        
        }
    }
};
