#include <string.h>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Options {
    json settings_json;

    Options load(Options Settings) {
        std::ifstream f("../settings.json");
        Settings.settings_json = json::parse(f);
        f.close();
        return Settings;
    }
};
