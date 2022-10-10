#include <cpr/session.h>
#include <cstdio>
#include <string>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "user.hpp"

using json = nlohmann::json;

struct UI {
    void newScreen() {
        std::system("clear");
        printf("<===== Neptune =====>\n");
    }

    void split(std::string line, std::string array[]) {
    int i = 0;
    std::stringstream ssin(line);

        while (ssin.good() && i < 4) {
            ssin >> array[i];
            ++i;
        }
    }

    void userInput(std::string command[], std::string text) {
        std::string input;
        printf("[%s] (?): ", text.c_str());
        std::getline(std::cin, input);
        split(input, command);
    }

    void classMenu(User Student, int sectionID) {
        int n = 0;
        int index = -1;
        std::string input;
        std::string msg = "";
        cpr::Session session;

        session.SetUrl(cpr::Url{Student.url + Student.login_path});
        session.SetParameters(Student.parameters);
        cpr::Response r = session.Post();
        session.SetUrl(cpr::Url{Student.url + "/campus/resources/portal/grades/detail/" + std::to_string(sectionID)});
        cpr::Response c = session.Get();
        json course_json = json::parse(c.text);

        while (true) {
            newScreen();
            n = 0;

            //  "ClassName" ["Grade"] (Percentage)
            printf("\n%s [%s] (%.2f%%)\n", std::string(course_json["details"][0]["task"]["courseName"]).c_str(), std::string(course_json["details"][0]["task"]["progressScore"]).c_str(), float(course_json["details"][0]["task"]["progressPercent"]));

            
            for (auto& it : course_json["details"][0]["categories"].items()) {
                printf("\t[%d] %s [%.2f/%.2f] (%.2f%%)", n++, std::string(it.value()["name"]).c_str(), float(it.value()["progress"]["progressPointsEarned"]), float(it.value()["progress"]["progressTotalPoints"]), float(it.value()["progress"]["progressPercent"]));

                if (index == n - 1) {
                    printf(" >>\n");
                    expandCategory(Student, it.value());
                } else {
                    printf("\n");
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

    void allClassesMenu(User Student, Exceptions Error, json student_info) {
        std::string msg = "";
        while (true) {
            int i = 0;
            newScreen();
            printf("[N]: %d\n\n", Student.unreadNotifs);
            for (auto& it : student_info[0]["terms"][0]["courses"].items()) {
                printf("[%d] %s\n", i++, std::string(it.value()["courseName"]).c_str());
            }
            printf("\n%s\n", msg.c_str());
            std::string command[4];
            userInput(command, Student.first_name);
            if (command[0] == "logout") {
                break;
            } else if (command[0] == "r") {
                Student = Student.login(Student, Error);
                continue;
            } else if (command[0] == "n") {
                notifications(Student);
            } else {
                auto index = Student.courses.begin();
                try {
                    std::advance(index, std::stoi(command[0])); //  Get class sectionID from index
                } catch (std::invalid_argument) {
                    msg = "Input must be a number";
                    continue;
                }
                classMenu(Student, *index);
                msg = "";
            }
        }
    };

    void expandCategory(User Student, json course_json) {
        std::string missing;
        for (auto& it : course_json["assignments"].items()) {
            missing = "    ";
            try {
                if (it.value()["missing"]) { missing = "\033[1;31m[M] \033[0m"; }
                printf("\t\t%s%s %3s%.2f/%.2f]\n", missing.c_str(), std::string(it.value()["assignmentName"]).c_str(), std::string("[").c_str(), std::stof(it.value()["scorePoints"].get<std::string>()), it.value()["totalPoints"].get<double>());
            } catch (nlohmann::detail::type_error) {
                continue;
            }
        }
    }

    void notifications(User Student) {
        cpr::Session session;
        session.SetUrl(cpr::Url{Student.url + Student.login_path});
        session.SetParameters(Student.parameters);
        cpr::Response r = session.Post();
        
        session.SetUrl(cpr::Url{Student.url + "/campus/prism?x=user.HomePage-loadNewMessagesCount&urlFilter=portal"});
        session.SetHeader(cpr::Header{{"Accept", "application/json"}});
        cpr::Response u = session.Get();
        Student.unreadNotifs = std::stoi(std::string(json::parse(u.text)["data"]["NewMessages"]["totalCount"]));

        while (true) {
            int c = 0;
            int showCount = 5;
            std::string stars = "";
            newScreen();
            printf("\n");
            for (auto& it : Student.notifs_json["data"]["NotificationList"]["Notification"].items()) {
                stars = "";
                if (it.value()["read"] == "false") { stars = "*"; };
                printf("[%d] %s%s%s\n", c++, stars.c_str(), std::string(it.value()["finalText"]).c_str(), stars.c_str());
                if (c > showCount - 1) { break; };
            }
            std::string command[4];
            userInput(command, Student.first_name);
            if (command[0] == "b") {
                break;
            }
        }
    }

    void newProfile(UserProfiles Profiles) {
        int stage = 1;
        std::string command[2];
        std::string name;
        std::string msg;
        Profiles = Profiles.load(Profiles);
        
        newScreen();
        printf("\n");

        while (true) {
            switch (stage) {
                case 1:
                    userInput(command, "Name of profile");
                    name = command[0];
                    stage = 2;
                    continue;
                case 2:
                    printf("%s", msg.c_str());
                    userInput(command, "Login method");
                    if (command[0] == "microsoft") { stage = 3; }
                    else if (command[0] == "json") { stage = 4; }
                    else if (command[0] == "?") { 
                        msg = "Login Methods: 'microsoft', 'json'\n";
                        stage = 2;
                    }
                    else { 
                        stage = 2; 
                        msg = "Invalid login method\n";
                        newScreen();
                        printf("\n[Name of profile] (?): %s\n", name.c_str());
                    }
                    continue;
                case 3:
                    Profiles.profile_json["user"][name]["login_method"] = command[0];
                    userInput(command, "IC URL");
                    Profiles.profile_json["user"][name]["campus_url"] = command[0];
                    userInput(command, "IC Login URL Path");
                    Profiles.profile_json["user"][name]["login_path"] = command[0];
                    userInput(command, "SAMLResponce");
                    Profiles.profile_json["user"][name]["saml"] = command[0];
                    printf("\nProfile successfuly created!\n");
                    userInput(command, "Press enter to return");
                    stage = 0;
                    continue;
                case 4:
                    Profiles.profile_json["user"][name]["login_method"] = command[0];
                    printf("jaysong");
                    break;
                default:
                    Profiles = Profiles.write(Profiles);
                    break;
            }
            break;
        }
    }
};
