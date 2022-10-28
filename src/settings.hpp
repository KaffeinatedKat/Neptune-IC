#include <string.h>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Options {
    json settings_json;

    std::string missing_color;
    std::string color_reset;
    int default_notification_count;

    Options load(Options Settings) {
        std::ifstream f("../settings.json");
        Settings.settings_json = json::parse(f);
        f.close();

        try {
            Settings.default_notification_count = Settings.settings_json[0]["notification-count"];
        } catch (nlohmann::detail::type_error) {
            Settings.default_notification_count = 5;
        }

        if (Settings.settings_json[0]["colored-text"]["enabled"] == false) {
            Settings.missing_color = "";
            Settings.color_reset = "";
        } else {
            Settings.missing_color = "\033[1;31m";
            Settings.color_reset = "\033[0m";
        }

        return Settings;
    }

    
    std::string gradeColor(std::string grade) {  // TODO: this can only be used to set colors in one place because of how it returns, fix that shit
            Options Settings;
            Settings = Settings.load(Settings);

            json colors = Settings.settings_json[0]["colored-text"];

            if (colors["enabled"] != true) { return "(" + grade; }

            for (auto& it : colors["grade-colors"].items()) {
                if (grade == it.key()) { return "\033[" + std::string(it.value()) + "m(" + grade; }
            }

            return "(" + grade;
    }
};
