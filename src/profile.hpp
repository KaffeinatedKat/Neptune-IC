#pragma once
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct UserProfiles {
    json profile_json;

    void load(UserProfiles &Profiles) {
        std::ifstream f("../profiles.json");
        Profiles.profile_json = json::parse(f);
    };

    void write(UserProfiles &Profiles) {
        std::ofstream f("../profiles.json");
        f << Profiles.profile_json;
        f.close();
        Profiles.load(Profiles);
    }
};
