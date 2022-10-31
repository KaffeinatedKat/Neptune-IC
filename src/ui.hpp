#include <cpr/session.h>
#include <cstdio>
#include <nlohmann/detail/exceptions.hpp>
#include <string>
#include <sstream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "settings.hpp"
#include "user.hpp"
#include "help_text.cpp"

using json = nlohmann::json;

struct UI {
    void newScreen(std::string text) {
        std::system("clear");
        printf("<===== %s =====>\n", text.c_str());
    }

    void split(std::string line, std::string array[]) { //  Stolen code to split a string into an aray
    int i = 0;
    std::stringstream ssin(line);

        while (ssin.good() && i < 4) {
            ssin >> array[i];
            ++i;
        }
    }

    void userInput(std::string command[], std::string text) {
        std::string input;
        printf("\33[2K\r[%s] (?): ", text.c_str());
        std::getline(std::cin, input);
        split(input, command);
    }

    
    void classMenu(User Student, int sectionID, Options Settings) {
        int n = 0;
        int index = -1;
        std::string section;
        std::string input;
        std::string msg = "";
        cpr::Session session;
        json course_json;

        std::stringstream temp;
        temp << sectionID; //  to_string is a royal shithead, and stringstream is my solution, not the best solution but hey it works doesnt it

        if (Student.login_method == "json_file") {
            std::ifstream g("../userJson/" + Student.profile_name + "_" + temp.str() + ".json");
            course_json = json::parse(g);
        } else if (Student.login_method == "microsoft" || Student.login_method == "username") {
            session.SetUrl(cpr::Url{Student.login_url}); //  Login to IC
            session.SetParameters(Student.parameters);
            cpr::Response r = session.Post();
            session.SetUrl(cpr::Url{Student.url + "/campus/resources/portal/grades/detail/" + std::to_string(sectionID)}); //  Request class json via section_id
            cpr::Response c = session.Get();
            course_json = json::parse(c.text);
        }

        while (true) {
            newScreen("Class Overview");
            n = 0;

            //  "ClassName" ["Grade"] (Percentage)
            printf("\n%s%s [%s] (%.2f%%)%s\n", Settings.gradeColor(Settings, course_json["details"][0]["task"]["progressScore"]).c_str(), std::string(course_json["details"][0]["task"]["courseName"]).c_str(), std::string(course_json["details"][0]["task"]["progressScore"]).c_str(), float(course_json["details"][0]["task"]["progressPercent"]), Settings.color_reset.c_str());

            //  Print each grade category
            for (auto& it : course_json["details"][0]["categories"].items()) {
                try {
                    //   "Category Name" [earned/total points] (Percentage)
                    printf("\t[%d]%s %s [%.2f/%.2f] (%.2f%%)%s", n++, Settings.gradeColor(Settings, it.value()["progress"]["progressScore"]).c_str(), std::string(it.value()["name"]).c_str(), float(it.value()["progress"]["progressPointsEarned"]), float(it.value()["progress"]["progressTotalPoints"]), float(it.value()["progress"]["progressPercent"]), Settings.color_reset.c_str());
                    
                    if (index == n - 1) { //  If user input index == category index
                        printf(" >>\n");
                        expandCategory(Student, it.value(), Settings); //  Expand the category and print all its assignments
                    } else {
                        printf("\n");
                    }
                } catch (nlohmann::detail::type_error) { //  Jank lmao, if there's a type error do nothing. Empty categories cause issues otherwise
                    ;
                }

            }
            printf("\n%s\n", msg.c_str());
            std::string command[4];
            userInput(command, Student.first_name);
            if (command[0] == "b") { break; };
            try {
                index = std::stoi(command[0]);
            } catch (std::invalid_argument) {
                msg = "Input must be a number (or b)";
            }
        }
    }

    void allClassesMenu(User Student, Exceptions Error, Options Settings, json student_info) {
        std::string msg = "";
        while (true) {
            int i = 0;
            newScreen("Student Overview");
            printf("[N]: %d\n\n", Student.unreadNotifs); //  Print unread notification count

            for (auto& it : student_info[0]["terms"][0]["courses"].items()) { //  Print each class
                try {
                    //  (Grade) "Class Name"
                    printf("[%d] %s[%s] (%.2f%%) %s %s\n", i++, Settings.gradeColor(Settings, it.value()["gradingTasks"][0]["progressScore"]).c_str(), std::string(it.value()["gradingTasks"][0]["progressScore"]).c_str(), float(it.value()["gradingTasks"][0]["progressPercent"]), std::string(it.value()["courseName"]).c_str(), Settings.color_reset.c_str());
                } catch (nlohmann::detail::type_error) { //  Yet another shitty catch, classes with no grades cause issues
                    continue;
                }
            }

            printf("\n%s\n", msg.c_str());
            std::string command[4];
            userInput(command, Student.first_name);

            if (command[0] == "logout") {
                break;
            } else if (command[0] == "r") { //  'Reload', logs back into IC and fetches new information
                Student = Student.login(Student, Error);
                continue;
            } else if (command[0] == "n") { //  Notification menu
                notifications(Student, Settings);
            } else {
                auto index = Student.courses.begin(); //  'Random access' for the class vector
                try {
                    std::advance(index, std::stoi(command[0])); //  Get class sectionID from index
                } catch (std::invalid_argument) {
                    msg = "Input must be a number";
                    continue;
                }
                classMenu(Student, *index, Settings); //  Expand class from sectionID index
                msg = "";
            }
        }
    };

    void expandCategory(User Student, json course_json, Options Settings) {
        std::string missing;
        for (auto& it : course_json["assignments"].items()) {
            missing = "    ";
            try {
                if (it.value()["missing"]) { missing = Settings.missing_color + "[M] " + Settings.color_reset; } //  Oooh fancy colors
                //  "Assignment Name" [earned/total points]
                printf("\t\t%s%s %3s%.2f/%.2f]\n", missing.c_str(), std::string(it.value()["assignmentName"]).c_str(), std::string("[").c_str(), std::stof(it.value()["scorePoints"].get<std::string>()), it.value()["totalPoints"].get<double>());
            } catch (nlohmann::detail::type_error) { //  Again with the jank, ungraded assignments also cause issues
                continue;
            }
        }
    }

    void notifications(User Student, Options Settings) {
        cpr::Session session;
        int *showCount;
        std::string msg = "\n";
        showCount = new int (Settings.default_notification_count); //  How many notifications to load

        session.SetUrl(cpr::Url{Student.login_url}); //  Login to IC to fetch notification data
        session.SetParameters(Student.parameters);
        cpr::Response r = session.Post();
        
        session.SetUrl(cpr::Url{Student.url + "/campus/prism?x=user.HomePage-loadNewMessagesCount&urlFilter=portal"}); //  Load notifications
        session.SetHeader(cpr::Header{{"Accept", "application/json"}});
        cpr::Response u = session.Get();

        session.SetUrl(cpr::Url{Student.url + "/campus/prism?x=notifications.NotificationUser-updateLastViewed&urlFilter=portal"}); //  Update read notifications on IC side

        session.SetUrl(cpr::Url{Student.url + "/campus/prism?x=user.HomePage-loadNewMessagesCount&urlFilter=portal"}); //  Re-update notification count for client to account for now read notifications
        session.SetHeader(cpr::Header{{"Accept", "application/json"}});
        cpr::Response ug = session.Get();
        Student.unreadNotifs = std::stoi(std::string(json::parse(u.text)["data"]["NewMessages"]["totalCount"]));  //  New total unread notifications (usually 0 after this point, some edge cases however)


        cpr::Response n = session.Get();

        while (true) {
            int c = 0;
            std::string stars = "";
            newScreen("Notifications");
            printf("\n");
            for (auto& it : Student.notifs_json["data"]["NotificationList"]["Notification"].items()) {
                stars = "";
                if (it.value()["read"] == "false") { stars = "*"; };
                printf("[%d] %s%s%s\n", c++, stars.c_str(), std::string(it.value()["finalText"]).c_str(), stars.c_str());
                if (c > *showCount - 1) { break; };
            }
            printf("\n%s", msg.c_str());
            std::string command[4];
            userInput(command, Student.first_name);
            if (command[0] == "b") {
                delete showCount;
                break;
            } else {
                try {
                    showCount = new int (std::stoi(command[0]));
                    msg = "\n";
                } catch (std::invalid_argument) {
                    msg = "Input must be a number (or b)\n";
                }
            }
        }
    }

    std::string newProfile(UserProfiles Profiles, std::string name) {
        int stage = 1;
        std::string command[2];
        std::string msg;
        Profiles.load(Profiles);

        for (auto& it : Profiles.profile_json["user"].items()) { //  Warn user if profile to be created already exists
            if (name == it.key()) {
                printf("Profile '%s' already exists, are you sure you want to override it?\n[y/n] ", name.c_str());
                getline(std::cin, *command);
                if (command[0].rfind("n", 0) == 0) {
                    return "";
                }
            }
        }

        
        newScreen("Profile Creation");

        while (true) {
            switch (stage) {
                case 1: {
                    if (name == "") {
                        return "No name specified\nTry `? profiles` for more information\n\n";
                    }
                }
                case 2: {
                    newScreen("Profile Creation");
                    printf("\n[Profile Name] (?): %s\n%s", name.c_str(), msg.c_str());
                    userInput(command, "Login method");
                    if (command[0] == "microsoft" || command[0] == "username") { stage = 3; }
                    else if (command[0] == "json") { stage = 4; }
                    else if (command[0] == "?") { 
                        msg = "Login Methods: 'microsoft', 'username', 'json'\n";
                        stage = 2;
                    } else if (command[0] == "q") {
                        return "";
                    }
                    else { 
                        stage = 2; 
                        msg = "Invalid login method\n";
                        newScreen("Profile Creation");
                    }
                    continue;
                }
                case 3: {
                    std::string *json_entries;
                    std::string *prompts;
                    std::string method = command[0];

                    //  Lists for microsoft and username json paths and names
                    if (method == "microsoft") {
                        json_entries = new std::string[3] {"campus_url", "login_path", "saml"};
                        prompts = new std::string[3] {"IC URL", "IC Login URL Path", "SAMLResponce"};
                    } else if (method == "username") {
                        json_entries = new std::string[3] {"campus_url", "username", "password"};
                        prompts = new std::string[3] {"IC URL", "Username", "Password"};
                    }
                    bool success = false;
                    Profiles.profile_json["user"][name]["login_method"] = method;
                        
                    for (int c = 0; c < 3; c++) { //  Loop through items to be set, and set each one
                        userInput(command, prompts[c]); //  Prompt user with value from prompts[]
                        if (command[0] == "?") { //  If "?" is entered at any point, help will be shown
                            newScreen("MS Profile Creation Help");
                            printf("%s\n[Return to exit]", microsoftProfileHelp().c_str());
                            getline(std::cin, *command);
                            success = false;
                            break;
                        } else {
                            Profiles.profile_json["user"][name][json_entries[c]] = command[0];  //  Set item from json_entries[] and move to the next item
                            success = true;
                        }
                    }

                    if (success) { //  If all items are successfully set, write to profiles.json and return
                        delete[] json_entries; //  Free memory before exiting
                        delete[] prompts;

                        Profiles.write(Profiles);
                        return "Profile '" + name + "' was successfully created!\n";
                    } else { //  If not, return to variable set loop till all items successfully set
                        newScreen("Profile Creation");
                        printf("\n[Profile Name] (?): %s\n[Login Method] (?): %s\n", name.c_str(), method.c_str());
                        stage = 3;
                        continue;
                    }
                }
                case 4: {
                    Profiles.profile_json["user"][name]["login_method"] = "json";
                    Profiles.write(Profiles);
                    return "Profile '" + name + "' successfully created!\nRefer to `? json` for infomation on setting up your json files\n";
                }
            }
        }
    }

    std::string listProfiles(UserProfiles Profiles, std::string message) {
        Profiles.load(Profiles);
        message = "All profiles:\n\n";

        for (auto& it : Profiles.profile_json["user"].items()) {
            message.append(it.key() + " [" + std::string(it.value()["login_method"]) + "]\n");
        }
        message.append("\n");

        return message;
    };
};
