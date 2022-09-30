#include <string>
#include <fstream>
#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "exceptions.hpp"

using json = nlohmann::json;

struct User {
    json grades_json, student_json, notifs_json;
    bool logged_in = false;
    cpr::Parameters parameters;
    std::string first_name = "None";
    std::string profile_name;
    std::string error;
    std::string url;
    std::string login_path;
    std::list<int> courses;
    int unreadNotifs = 0;

    User login(User Student, Exceptions Error) {
        std::ifstream f("../profiles.json");
        json profiles = json::parse(f);
        json student_json, grades_json, notifs_json;
                        

        try {
            std::string login_method = profiles["user"][Student.profile_name]["login_method"].get<std::string>();

            if (login_method == "json_file") {
                std::ifstream g("../" + profiles["user"][Student.profile_name]["grades_json"].get<std::string>());
                std::ifstream s("../" + profiles["user"][Student.profile_name]["student_json"].get<std::string>());
                Student.grades_json = json::parse(g);
                Student.student_json = json::parse(s);
                Student.logged_in = true;
            } else if (login_method == "microsoft") {
                cpr::Session session;
                std::string saml = profiles["user"][Student.profile_name]["saml"].get<std::string>(); //  SAMLResponse
                Student.login_path = profiles["user"][Student.profile_name]["login_path"].get<std::string>(); //  SAMLResponse POST path
                Student.url = profiles["user"][Student.profile_name]["campus_url"].get<std::string>(); //  Infinite Campus URL
                Student.parameters = cpr::Parameters{{"SAMLResponse", saml}};
                
                session.SetUrl(cpr::Url{Student.url + Student.login_path});
                session.SetParameters(Student.parameters);
                cpr::Response r = session.Post();       
                session.SetUrl(cpr::Url{Student.url + "/campus/resources/portal/grades"});
                cpr::Response g = session.Get();
                session.SetUrl(cpr::Url{Student.url + "/campus/api/portal/students"});
                cpr::Response s = session.Get();
                session.SetUrl(cpr::Url{Student.url + "/campus/prism?x=notifications.Notification-retrieve&limitCount=200&urlFilter=portal"});
                session.SetHeader(cpr::Header{{"Accept", "application/json"}});
                cpr::Response n = session.Get();

                Student.grades_json = json::parse(g.text);
                Student.student_json = json::parse(s.text);
                Student.notifs_json = json::parse(n.text);
                Student.logged_in = true;
            }

            if (Student.logged_in) {
                for (auto& it : Student.grades_json[0]["terms"][0]["courses"].items()) { //  Add each course's ID into a list to access each class via an index
                    Student.courses.push_back(it.value()["sectionID"]);
                }
                for (auto& it : Student.notifs_json["data"]["NotificationList"]["Notification"].items()) {
                    
                    if ( it.value()["read"] == "false" ) {
                        Student.unreadNotifs++;
                    }
                }
            }
        } catch (nlohmann::detail::type_error) {
            Student.error = Error.userNotFound(Student.profile_name);
        }
        return Student;
    };
};
