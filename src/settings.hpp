#include <string.h>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Options {
    json settings_json;

    std::string missing_color;
    std::string color_reset;

    Options load(Options Settings) {
        std::ifstream f("../settings.json");
        Settings.settings_json = json::parse(f);
        f.close();

        if (Settings.settings_json[0]["colored-text"]["enabled"] == false) {
            Settings.missing_color = "";
            Settings.color_reset = "";
        } else {
            Settings.missing_color = "\033[1;31m";
            Settings.color_reset = "\033[0m";
        }

        return Settings;
    }
};
