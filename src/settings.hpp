#pragma once

#include <fstream>
#include <optional>
#include <boost/json.hpp>

#include "logging.hpp"


#define SETTINGS (Settings::instance())
class Settings
{
    
public:
    Settings(Settings &) = delete;
    void operator=(const Settings &) = delete;
    void save();
    static Settings *instance()
    {
        if (instance_ == nullptr)
        {
            instance_ = new Settings("./settings.json");
        }
        return instance_;
    }


    template <class T>
    T get(const char* path, std::optional<T> defaultValue);
    
    
private:
    std::fstream file_;
    static Settings* instance_;
    boost::json::object baseObject_;
    Settings(const char *path);
};

template <>
int64_t Settings::get<int64_t>(const char* path, std::optional<int64_t> defaultValue);
template <>
double Settings::get<double>(const char* path, std::optional<double> defaultValue);