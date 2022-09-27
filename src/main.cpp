#include <iostream>
#include <cstdlib>
#include <nlohmann/detail/exceptions.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "help_text.cpp"

using json = nlohmann::json;

struct User;
void newScreen();
void split(std::string line, std::string array[]);
void userInput(User Student, std::string command[]);

struct Exceptions {
    void notLoggedIn(std::string command) {
        std::cout << "Neptune: You must login to use '" << command << "'\n";
    };

    void userNotFound(std::string user) {
        std::cout << "Neptune: No profile for user '" << user << "'\nTry '? profiles' for more information\n";
    };

    void notANumber(std::string command) {
        std::cout << "Neptune: Command '" << command << "' takes only numbers as input\n";
    };
};



struct User {
    json grades_json, student_json;
    bool logged_in = false;
    cpr::Parameters parameters;
    std::string first_name = "None";
    std::string url;
    std::string login_path;
    std::list<int> courses;

    User login(Exceptions Error, std::string command[]) {
        User Student;
        std::ifstream f("../profiles.json");
        json profiles = json::parse(f);
        json student_json, grades_json;
                        

        try {
            std::string login_method = profiles["user"][command[1]]["login_method"].get<std::string>();

            if (login_method == "json_file") {
                std::ifstream g("../" + profiles["user"][command[1]]["grades_json"].get<std::string>());
                std::ifstream s("../" + profiles["user"][command[1]]["student_json"].get<std::string>());
                Student.grades_json = json::parse(g);
                Student.student_json = json::parse(s);
                Student.logged_in = true;
            } else if (login_method == "microsoft") {
                cpr::Session session;
                std::string saml = profiles["user"][command[1]]["saml"].get<std::string>(); //  SAMLResponse
                Student.login_path = profiles["user"][command[1]]["login_path"].get<std::string>(); //  SAMLResponse POST path
                Student.url = profiles["user"][command[1]]["campus_url"].get<std::string>(); //  Infinite Campus URL
                Student.parameters = cpr::Parameters{{"SAMLResponse", saml}};
                
                session.SetUrl(cpr::Url{Student.url + Student.login_path});
                session.SetParameters(Student.parameters);
                cpr::Response r = session.Post();       
                session.SetUrl(cpr::Url{Student.url + "/campus/resources/portal/grades"});
                cpr::Response g = session.Get();
                session.SetUrl(cpr::Url{Student.url + "/campus/api/portal/students"});
                cpr::Response s = session.Get();

                Student.grades_json = json::parse(g.text);
                Student.student_json = json::parse(s.text);
                Student.logged_in = true;
            }

            if (Student.logged_in) {
                for (auto& it : Student.grades_json[0]["terms"][0]["courses"].items()) { //  Add each course's ID into a list to access each class via an index
                    Student.courses.push_back(it.value()["sectionID"]);
                }

                Student.first_name = Student.student_json[0]["firstName"];
                std::cout << "Neptune: Successfully logged in as " << Student.first_name << std::endl;
            }
        } catch (nlohmann::detail::type_error) {
            Error.userNotFound(command[1]);
        }
        return Student;
    };

    void classMenu(User Student, int sectionID) {
        int n = -1;
        int index;
        std::string input;
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
            std::cout << course_json["details"][0]["task"]["courseName"] << " [" << course_json["details"][0]["task"]["progressScore"] << "] (" << course_json["details"][0]["task"]["progressPercent"] << ")\n";

            for (auto& it : course_json["details"][0]["categories"].items()) {
                n++;
                std::cout << "\t[" << n << "] " << it.value()["name"] << "  [" << it.value()["progress"]["progressPointsEarned"] << "/" << it.value()["progress"]["progressTotalPoints"] << "] (" << it.value()["progress"]["progressPercent"] << "%)";
            
                if (index == n) {
                    std::cout << " >>\n";
                    expandCategory(Student, it.value());
                } else {
                    std::cout << "\n";
                }
            }
            std::cout << std::endl;
            
            n = -1;
            std::string command[4];
            userInput(Student, command);
            if (command[0] == "b") { break; };
            index = std::stoi(command[0]);
        }
    }

    void allClassesMenu(User Student, json student_info) {
        while (true) {
            int i = 0;
            newScreen();
            for (auto& it : student_info[0]["terms"][0]["courses"].items()) {
                std::cout << "[" << i++ << "] " << it.value()["courseName"] << std::endl;
            }
            std::cout << std::endl;
            std::string command[4];
            userInput(Student, command);
            if (command[0] == "logout") { break; }
            auto index = Student.courses.begin();
            std::advance(index, std::stoi(command[0])); //  Get class sectionID from index
            
            Student.classMenu(Student, *index);
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

};



void split(std::string line, std::string array[]) {
    int i = 0;
    std::stringstream ssin(line);

    while (ssin.good() && i < 4) {
        ssin >> array[i];
        ++i;
    }
};

void newScreen() {
    std::system("clear");
    printf("<===== Neptune =====>\n\n");
}

void userInput(User Student, std::string command[]) {
    std::string input;
    std::cout << "[" << Student.first_name << "] (?): ";
    std::getline(std::cin, input);
    split(input, command);
}



int main() {
    User Student;
    Exceptions Error;
    std::string input;
    std::string msg = "login with `login [profile]`\nFor help on creating profiles, do `? profiles`\n\n";

    while (true) {
        std::string command[4];
        newScreen();
        printf("%s", msg.c_str());
        userInput(Student, command);
        json grades_json;

        if (command[0] == "login") {
            Student = Student.login(Error, command);
            Student.allClassesMenu(Student, Student.grades_json);
            Student.logged_in = false;
            Student.first_name = "None";
            msg = "login with `login [profile]`\nFor help on creating profiles, do `? profiles`\n\n";

        } else if (command[0] == "?") {
            if (command[1] == "profiles") {
                newScreen();
                profileAllHelp();
            }
        
        } else if (command[0] == "q") {
            std::exit(0);
        
        } else {
            msg = "Command '" + command[0] + "' not found\n\n";
        
        }
    }
};
