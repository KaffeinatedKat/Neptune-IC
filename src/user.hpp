#pragma once
#include <cpr/redirect.h>
#include <cpr/response.h>
#include <cpr/session.h>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include "exceptions.hpp"
#include "profile.hpp"

using json = nlohmann::json;

struct User {
    UserProfiles Profiles;
    json grades_json, student_json, notifs_json;
    bool logged_in = false;
    bool fetch = false;
    cpr::Parameters parameters;
    std::string first_name = "None";
    std::string profile_name;
    std::string error;
    std::string url;
    std::string ms_url;
    std::string login_path;
    std::string login_method;
    std::string username;
    std::string password;
    std::string login_url;
    std::string term;
    int unreadNotifs = 0;
    std::list<int> courses;
    std::list<std::string> term_list;

    std::string getDate() {
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
 
        char buffer[128];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", now);
        return buffer;
    }


    User login(User &Student, Exceptions Error) {
        Profiles.load(Profiles);
        json student_json, grades_json, notifs_json;
                        

        try {
            Student.login_method = Profiles.profile_json["user"][Student.profile_name]["login_method"].get<std::string>();

            if (Student.login_method == "json") {
                //  Read json files with profile name
                std::ifstream g("../userJson/" + Student.profile_name + "_grades.json");
                std::ifstream s("../userJson/" + Student.profile_name + "_students.json");
                
                //  Set variables 
                Student.grades_json = json::parse(g);
                Student.student_json = json::parse(s);
                
                g.close();
                s.close();
                Student.logged_in = true;
            } else if (Student.login_method == "username") {
                cpr::Session session;
                std::string iD;
                std::string appName;
                Student.url = Profiles.profile_json["user"][Student.profile_name]["campus_url"].get<std::string>();
                Student.username = Profiles.profile_json["user"][Student.profile_name]["username"].get<std::string>();
                Student.password = Profiles.profile_json["user"][Student.profile_name]["password"].get<std::string>();

                session.SetUrl(cpr::Url{Student.url}); //  Request url to get appName
                cpr::Response s = session.Get();
                appName = std::string(s.url).erase(0, std::string(s.url).find("campus/") + 7); //  Isolate the appName
                appName = appName.erase(appName.find("."));
    
                session.SetUrl(cpr::Url{Student.url + "/campus/portal/students/" + appName + ".jsp"}); //  Request student page to get portalUrl
                cpr::Response r = session.Get();

                iD = r.text.erase(0, r.text.find("portal/students/" + appName + ".jsp")); //  Isolate the line of html with the portalUrl for login
                iD.erase(iD.find("\"")); //  Truncate everyting after the " to isolate just the portalUrl string

                Student.login_url = Student.url + "/campus/verify.jsp";
                Student.parameters = cpr::Parameters{{"username", Student.username}, {"password", Student.password}, {"portalUrl", iD}, {"appName", appName}};
                Student.fetch = true;

            } else if (Student.login_method == "microsoft") {
                std::string saml = Profiles.profile_json["user"][Student.profile_name]["saml"].get<std::string>(); //  SAMLResponse
                Student.login_path = Profiles.profile_json["user"][Student.profile_name]["login_path"].get<std::string>(); //  SAMLResponse POST path
                Student.url = Profiles.profile_json["user"][Student.profile_name]["campus_url"].get<std::string>(); //  Infinite Campus URL
                Student.parameters = cpr::Parameters{{"SAMLResponse", saml}};
                Student.login_url = Student.url + Student.login_path;
                Student.fetch = true;
            }
            
            if (Student.fetch) {
                cpr::Session session;
                Student.fetch = false;

                //  Login to infinite campus and GET reuest all URL's to get nessisary data
                session.SetUrl(cpr::Url{Student.login_url});
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

                //  Set all variables for json's and such after GET requesting all the URL's
                Student.grades_json = json::parse(g.text); 
                Student.student_json = json::parse(s.text);
                Student.notifs_json = json::parse(n.text);
                Student.unreadNotifs = std::stoi(std::string(json::parse(u.text)["data"]["RecentNotifications"]["count"]));
                Student.logged_in = true;

            }

            if (Student.logged_in) {
                std::string current_date = getDate(); //  Get current date
                Student.term_list.clear(); //  Clear terms and courses to prevent overlapping when logging out and back in
                Student.courses.clear();
                
                for (auto& it : Student.grades_json[0]["terms"].items()) { //  Loop through each term and compare dates to current
                    if (current_date > it.value()["startDate"] && current_date < it.value()["endDate"]) { //  If current date is between a terms start and end dates, that is the current term
                        Student.term = it.value()["termName"];
                    }
                    Student.term_list.push_back(std::string(it.value()["termName"]));
                }

                for (auto& it : Student.grades_json[0]["terms"].items()) { //  Add each course's ID into a list to access each class via an index 
                    if (it.value()["termName"] == Student.term) { //  Only add classes for current term
                        for (auto& it : it.value()["courses"].items()) {
                            Student.courses.push_back(it.value()["sectionID"]);
                        }
                    }
                }

                Student.first_name = Student.student_json[0]["firstName"];
            }
        } catch (nlohmann::detail::type_error) { //  Lazy catch, general catchall for errors, present "no profile" error
            Student.error = Error.userNotFound(Student.profile_name);
            Student.logged_in = false;
        } catch (nlohmann::detail::parse_error) { //  Lazy catch, this should only happen if the json files are not present, present error assuming they are absent
            Student.error = Error.noJson(Student.profile_name);
            Student.logged_in = false;
        }
        return Student;
    };
};
