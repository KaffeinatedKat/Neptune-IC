#include <iostream>
#include <nlohmann/detail/exceptions.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <exception>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;



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
    }


    void list_info() {
        std::cout << "First Name: " << student_json[0]["firstName"]
                << "\nLast Name: " << student_json[0]["lastName"]
                << "\nSchool: " << student_json[0]["enrollments"][0]["schoolName"] << std::endl;

    }


    void class_info(User Student, int sectionID, int index) {
        int n = -1;
        cpr::Session session;

        session.SetUrl(cpr::Url{Student.url + Student.login_path});
        session.SetParameters(Student.parameters);
        cpr::Response r = session.Post();
        session.SetUrl(cpr::Url{Student.url + "/campus/resources/portal/grades/detail/" + std::to_string(sectionID)});
        cpr::Response c = session.Get();
        json course_json = json::parse(c.text);

        //  "ClassName" ["Grade"] (Percentage)
        std::cout << "\n" << course_json["details"][0]["task"]["courseName"] << " [" << course_json["details"][0]["task"]["progressScore"] << "] (" << course_json["details"][0]["task"]["progressPercent"] << ")\n";

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
    }



    void expandCategory(User Student, json course_json) {
        for (auto& it : course_json["assignments"].items()) {
            try { 
                printf("\t\t%s [%.2f/%.2f]\n", std::string(it.value()["assignmentName"]).c_str(), std::stof(it.value()["scorePoints"].get<std::string>()), it.value()["totalPoints"].get<double>());
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



void list_classes(json student_info) {
    int i = 0;
    for (auto& it : student_info[0]["terms"][0]["courses"].items()) {
        std::cout << "[" << i++ << "] " << it.value()["courseName"] << std::endl;
    }
    std::cout << std::endl;
};



int main() {
    User Student;
    Exceptions Error;
    std::string input;

    while (true) {
        std::string command[4];
        std::cout << "[" << Student.first_name << "] (?): ";
        std::getline(std::cin, input);
        split(input, command);
        json grades_json;

        if (command[0] == "login") {
            Student = Student.login(Error, command);

        } else if (command[0] == "list") {
            if (Student.logged_in == false) {
                Error.notLoggedIn(command[0] + " " + command[1]);    
                continue;
            } else if (command[1] == "classes") {
                std::cout << std::endl;
                list_classes(Student.grades_json);
            } else if (command[1] == "info") {
                Student.list_info();
            }

        } else if (command[0] == "logout") {
            if (Student.logged_in == false) {
                Error.notLoggedIn(command[0]);   
                continue;
            } else {
                Student.logged_in = false;
                std::cout << "Neptune: Successfully logged out as '" << Student.first_name << "'\n";
                Student.first_name = "None";
            }

        } else if (command[0] == "class") {
            if (Student.logged_in == false) {
                Error.notLoggedIn(command[0]);
                continue;
            } else {
                try {
                    auto index = Student.courses.begin();
                    std::advance(index, std::stoi(command[1])); //  Get class sectionID from index
                    if (command[2] == "") { command[2] = "-1"; };
                    Student.class_info(Student, *index, std::stoi(command[2]));
                } catch (std::invalid_argument) {
                    Error.notANumber(command[0]);
                    continue;
                }
            }

        } else if (command[0] == "q") {
            std::exit(0);
        } else {
            std::cout << "Command '" << command[0] << "' not found\n";
        }
    }
};
