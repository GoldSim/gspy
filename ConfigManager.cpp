#include "ConfigManager.h"
#include <Windows.h>
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

static std::string get_base_path_without_extension() {
    char path[MAX_PATH] = { 0 };
    HMODULE hm = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(&GetConfigFilename), &hm) == 0)
    {
        return "";
    }
    if (GetModuleFileNameA(hm, path, sizeof(path)) == 0)
    {
        return "";
    }
    std::string base_path(path);
    size_t dot_pos = base_path.find_last_of(".");
    if (dot_pos != std::string::npos) {
        return base_path.substr(0, dot_pos);
    }
    return base_path;
}

std::string GetConfigFilename() {
    return get_base_path_without_extension() + ".json";
}

std::string GetLogFilename() {
    std::string config_path = GetConfigFilename();
    std::string default_name;

    size_t dot_pos = config_path.find_last_of(".");
    if (dot_pos != std::string::npos) {
        default_name = config_path.substr(0, dot_pos) + "_log.txt";
    }
    else {
        default_name = config_path + "_log.txt";
    }

    std::ifstream f(config_path);
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            if (data.contains("script_name")) {
                std::string script_name = data["script_name"];
                return script_name + "_log.txt";
            }
        }
        catch (json::parse_error&) { /* Fall through to default */ }
    }
    return default_name;
}