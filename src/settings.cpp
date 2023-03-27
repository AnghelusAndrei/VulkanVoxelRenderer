#include "settings.hpp"
#include "logging.hpp"
Settings *Settings::instance_;
Settings::Settings(const char *path)
{

    // file_.open(path, std::ios::in | std::ios::out | std::ios::ate);
    // std::streampos pos = file_.tellg();
    // char *text = new char[pos];
    // file_.seekg(0);
    // file_.read(text, pos);
    // file_.close();
    baseObject_=boost::json::object();
}

void Settings::save()
{
    //file_.
}

template <>
int64_t Settings::get<int64_t>(const char* path, std::optional<int64_t> defaultValue)
{
    return (baseObject_.if_contains(path) && baseObject_[path].is_int64())?baseObject_[path].as_int64():defaultValue.value();
}
template <>
double Settings::get<double>(const char* path, std::optional<double> defaultValue)
{
    return (baseObject_.if_contains(path) && baseObject_[path].is_double())?baseObject_[path].as_double():defaultValue.value();
}
