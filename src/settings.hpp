#pragma once
#include <string.h>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Options {
    json settings_json;
    bool colors = false;
    std::string missing_color = "";
    std::string color_reset = "";
    int default_notification_count = 5;

    Options load(Options Settings) {
        std::ifstream f("../settings.json"); //  Open file, load json into settings_json, then close the file
        Settings.settings_json = json::parse(f);
        f.close();

        Settings.default_notification_count = Settings.settings_json[0]["notification-count"]; //  Set default notification count

        if (Settings.settings_json[0]["colored-text"]["enabled"] == true) { //  Enable terminal colors
            Settings.colors = true;
            Settings.missing_color = "\033[1;31m";  
            Settings.color_reset = "\033[0m";
        }

        return Settings;
    }

    
    std::string gradeColor(Options Settings, std::string grade) {  //  Return color code from grade in color settings, if no match return nothing
            json colors = Settings.settings_json[0]["colored-text"];
            auto it = colors["grade-colors"].find(grade);
            
            if (it == colors["grade-colors"].end() || Settings.colors == false) { return ""; }

            return "\033[" + std::string(it.value()) + "m";
    }
};
