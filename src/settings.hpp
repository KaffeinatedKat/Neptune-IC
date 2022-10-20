#include <string.h>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Options {
    json json;

    Options load(Options Settings) {
        std::ifstream f("../settings.json");
        Settings.json = json::parse(f);
        return Settings;
    }
};
