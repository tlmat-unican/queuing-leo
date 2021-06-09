
//#include <cassert>
#include "Log.h"

bool LogRedirected = false;

std::size_t LogManager::m_maxModuleNameSize = 0;
void
LogManager::Register ( LogModule& logModule )
{
    auto check_ = m_modules.insert (ModulesMap_t::value_type{
                                    logModule.GetModuleName (), logModule
    });
    assert (check_.second == true && "LogModule::Register "
            "==> Error inserting module. Duplicated??");
    std::size_t currentSize_ = logModule.GetModuleName ().size ();
    
    if (currentSize_ > LogManager::m_maxModuleNameSize)
    {
        LogManager::m_maxModuleNameSize = currentSize_;
    }
}

void
LogManager::SetLogLevel ( std::string moduleName, LogLevel level )
{
    auto check_ = m_modules.find (moduleName);
    
    assert (check_ != m_modules.end () && "In LogModule::SetLogLevel "
            "==> Module name is not registered");
    check_->second.SetLevel (level);
}

void
LogManager::SetLogLevel ( LogLevel level )
{
    for (auto& item_ : m_modules)
    {
        item_.second.SetLevel (level);
    }
}

/********************************************************************************************************/

/*****************************Log Modules**************************/

LogModule::LogModule ( std::string name, LogLevel level )
: m_name ( name ), m_level ( level )
{
    CheckEnvVar ();
    LogManager::Instance ().Register (*this);
}

void
LogModule::CheckEnvVar ( void )
{
    char* clogVar_ = getenv (LOG_ENV_VAR_NAME);
    if (clogVar_ == nullptr)
    {
        return;
    }
    std::string logVar_ = clogVar_;
    std::size_t beg_ = logVar_.find (m_name);

    if (beg_ == std::string::npos)
    {
        return;
    }

    std::size_t token_ = logVar_.substr (beg_, logVar_.size ()
                                         - beg_).find (":");
    
    logVar_ = token_ == std::string::npos ?
            logVar_.substr (beg_, logVar_.size () - beg_) :
            logVar_.substr (beg_, token_ - beg_);

    beg_ = logVar_.find ("=");
    assert (beg_ != std::string::npos && "Environment Variable ill-formed");
    logVar_ = logVar_.substr (beg_ + 1, logVar_.size () - beg_ + 1);

    if ( logVar_.compare ("NONE") == 0 )
    {
        m_level = LogLevel::NONE;
    }
    else if ( logVar_.compare ("ERROR") == 0 )
    {
        m_level = LogLevel::ERROR;
    }
    else if ( logVar_.compare ("WARN") == 0 )
    {
        m_level = LogLevel::WARNING;
    }
    else if (logVar_.compare ("INFO") == 0 ) 
    {
        m_level = LogLevel::INFO;
    }
    else if ( logVar_.compare ("DEBUG") == 0 )
    {
        m_level = LogLevel::DEBUG;
    }
    else if (logVar_.compare ("ALL") == 0 ) // information & debug & function (BEG END)
    {
        m_level = LogLevel::ALL;
    }
    return;
}

std::string
LogModule::GetModuleName ( void ) const
{
    return m_name;
}

std::string
LogModule::GetPrintName ( void ) const
{
    std::string str_ (m_name);
    str_.resize (LogManager::m_maxModuleNameSize, ' ');
    return str_;
}

void
LogModule::SetLevel ( LogLevel level )
{
    m_level = level;
}

LogLevel
LogModule::GetLevel ( void ) const
{
    return m_level;
}
