#include <string>
#include <iostream>
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

    void userInput(User Student, std::string command[]) {
        std::string input;
        std::cout << "[" << Student.first_name << "] (?): ";
        std::getline(std::cin, input);
        split(input, command);
    }

    void classMenu(User Student, int sectionID) {
        int n = -1;
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

            //  "ClassName" ["Grade"] (Percentage)
            std::cout << "\n" << course_json["details"][0]["task"]["courseName"] << " [" << course_json["details"][0]["task"]["progressScore"] << "] (" << course_json["details"][0]["task"]["progressPercent"] << ")\n";

            for (auto& it : course_json["details"][0]["categories"].items()) {
                n++;
                std::cout << "\t[" << n << "] " << it.value()["name"] << "  [" << it.value()["progress"]["progressPointsEarned"] << "/" << it.value()["progress"]["progressTotalPoints"] << "] (" << it.value()["progress"]["progressPercent"] << "%)";
            
                if (index == n) {
                    printf(" >>\n");
                    expandCategory(Student, it.value());
                } else {
                    printf("\n");
                }
            }
            printf("\n%s\n", msg.c_str());
            n = -1;
            std::string command[4];
            userInput(Student, command);
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
                std::cout << "[" << i++ << "] " << it.value()["courseName"] << std::endl;
            }
            printf("\n%s\n", msg.c_str());
            std::string command[4];
            userInput(Student, command);
            if (command[0] == "logout") {
                break;
            } else if (command[0] == "r") {
                Student = Student.login(Student, Error);
                continue;
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

    void notifications(User Student, int count) {
        for (auto& it : Student.notifs_json.items()) {
            printf("");
        }       
    }

};
