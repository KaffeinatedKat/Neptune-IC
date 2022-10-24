#include <cpr/redirect.h>
#include <cpr/response.h>
#include <cpr/session.h>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "exceptions.hpp"
#include "profile.hpp"

using json = nlohmann::json;

struct User {
    UserProfiles Profiles;
    json grades_json, student_json, notifs_json;
    bool logged_in = false;
    cpr::Parameters parameters;
    std::string first_name = "None";
    std::string profile_name;
    std::string error;
    std::string url;
    std::string ms_url;
    std::string login_path;
    std::string login_method;
    std::string email;
    std::string password;
    int unreadNotifs = 0;
    std::list<int> courses;

    


    User login(User Student, Exceptions Error) {
        Profiles.load(Profiles);
        json student_json, grades_json, notifs_json;
                        

        try {
            Student.login_method = Profiles.profile_json["user"][Student.profile_name]["login_method"].get<std::string>();

            if (Student.login_method == "json") {
                std::ifstream g("../userJson/" + Student.profile_name + "_grades.json");
                std::ifstream s("../userJson/" + Student.profile_name + "_students.json");
                Student.grades_json = json::parse(g);
                Student.student_json = json::parse(s);
                Student.logged_in = true;
            } else if (Student.login_method == "microsoft") {
                cpr::Session session;
                std::string saml = Profiles.profile_json["user"][Student.profile_name]["saml"].get<std::string>(); //  SAMLResponse
                Student.login_path = Profiles.profile_json["user"][Student.profile_name]["login_path"].get<std::string>(); //  SAMLResponse POST path
                Student.url = Profiles.profile_json["user"][Student.profile_name]["campus_url"].get<std::string>(); //  Infinite Campus URL
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
                session.SetUrl(cpr::Url{Student.url + "/campus/prism?x=notifications.NotificationUser-countUnviewed&urlFilter=portal"});
                session.SetHeader(cpr::Header{{"Accept", "application/json"}});
                cpr::Response u = session.Get();


                Student.grades_json = json::parse(g.text);
                Student.student_json = json::parse(s.text);
                Student.notifs_json = json::parse(n.text);
                Student.unreadNotifs = std::stoi(std::string(json::parse(u.text)["data"]["RecentNotifications"]["count"]));
                Student.logged_in = true;
            }

            if (Student.logged_in) {
                for (auto& it : Student.grades_json[0]["terms"][0]["courses"].items()) { //  Add each course's ID into a list to access each class via an index
                    Student.courses.push_back(it.value()["sectionID"]);
                }
                Student.first_name = Student.student_json[0]["firstName"];
            }
        } catch (nlohmann::detail::type_error) {
            Student.error = Error.userNotFound(Student.profile_name);
            Student.logged_in = false;
        } catch (nlohmann::detail::parse_error) {
            Student.error = Error.noJson(Student.profile_name);
            Student.logged_in = false;
        }
        return Student;
    };
};
