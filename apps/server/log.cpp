#include "log.hpp"
#include <cassert>
#include <iostream>
#include <exception>

namespace digital_curling::server {

Log::Log(std::ofstream && file)
    : file_(std::move(file))
{
    assert(instance_ == nullptr);
    instance_ = this;
}

Log::~Log()
{
    instance_ = nullptr;
}

void Log::Trace(std::string_view message)
{
    Log::Print("[trace] ", message);
}

void Log::Debug(std::string_view message)
{
    Log::Print("[debug] ", message);
}

void Log::Info(std::string_view message)
{
    Log::Print("[info] ", message);
}

void Log::Warn(std::string_view message)
{
    Log::Print("[warn] ", message);
}

void Log::Error(std::string_view message)
{
    Log::Print("[error] ", message);
}

void Log::Print(char const * prefix, std::string_view message)
{
    assert(instance_);
    std::lock_guard guard(instance_->mutex_);
    instance_->file_ << prefix << message << '\n';
    instance_->file_.flush();
    std::cout << prefix << message << std::endl;
}

}
