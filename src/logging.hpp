#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <iostream>
#include <ostream>
#include <stdexcept>

#define EXCEPTION(x) (Logging::instance()->exception(x))
#define LOGGING (Logging::instance())

enum LoggingLevel
{
    VERBOSE = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

class Logging
{
public:
    Logging(Logging &) = delete;
    void operator=(const Logging &) = delete;

    static Logging *instance()
    {
        if (instance_ == nullptr)
        {
            instance_ = new Logging(VERBOSE);
        }
        return instance_;
    }

    std::ostream &verbose()
    {
        return getStream(VERBOSE) << "[VERBOSE] ";
    }
    
    std::ostream &info()
    {
        return getStream(INFO) << "[INFO] ";
    }
    std::ostream &warn()
    {
        return getStream(WARN) << "[WARN] ";
    }
    std::ostream &error()
    {
        return getStream(ERROR) << "[ERROR] ";
    }

    std::ostream &print(LoggingLevel level)
    {
        return getStream(level);
    }
    std::runtime_error exception(std::string what)
    {
        return std::runtime_error("[EXCEPTION] " + what);
    }


private:
    class VoidBuffer: public std::streambuf 
    {
        public:
        int overflow(int c) { return c;}
    } void_buffer_;

    Logging(LoggingLevel level) : level_(level), void_stream_(&void_buffer_)
    {
    }

    std::ostream& getStream(LoggingLevel level)
    {
        if(level>=level_)
        {
            return std::cerr;
        }else
            return void_stream_;
    }
    
    LoggingLevel level_;
    std::ostream void_stream_;
    
    
    
    static Logging *instance_;

};

#endif