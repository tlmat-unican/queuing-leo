#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <map>
#include <cassert>

//the following are UBUNTU/LINUX ONLY terminal color codes.
//Special
//0  Reset all attributes
//1  Bright/Bold
//2  Dim
//4  Underscore
//5  Blink
//7  Reverse
//8  Hidden
//
//Foreground colors
//30  Black
//31  Red
//32  Green
//33  Yellow
//34  Blue
//35  Magenta
//36  Cyan
//37  White
//
//Background colors
//40  Black
//41  Red
//42  Green
//43  Yellow
//44  Blue
//45  Magenta
//46  Cyan
//47  White

#define UNDERLINE "\33[4m"
#define BOLD "\033[1m"
#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

#define HEADERS_COL "\033[1m\033[32m"
#define WARNING_COL "\033[1m\033[4m\033[33m"
#define ERROR_COL "\033[1m\033[4m\033[31m"
#define ASSERT_COL "\033[1m\033[4m\033[31;43m"

extern bool LogRedirected;
/******************************************** LOGGING INTERFACE *************************************/
//#ifndef LOG_ENABLED
//#define LOG_ENABLED
//#endif
// #define LOG_ONLY_MODULES
//#define LOG_ONLY_HEADERS
// #define LOG_FILE_ENABLED
// #define LOG_TIME_ENABLED

/**
 * \ingroup logging
 * \brief Environment varaible from which the current logging is taken
 */
#define LOG_ENV_VAR_NAME "OGASE_LOG"

/**
 * \ingroup logging
 * \brief Logging levels. The upper levels include the lower ones
 */
enum class LogLevel : std::uint8_t
{
  NONE,    // No logging info
  ERROR,   // Only ERROR info
  WARNING, // WARNING and ERROR info
  INFO,    // DEBUG and INFO logging info
  DEBUG,   // DEBUG, WARNING and ERROR logging info
  ALL      // DEBUG, INFO, BEG and END logging info
};

/**
 * \ingroup logging
 * \brief Register a new logging module. Note that this is to be done in the .cc (or similar) files out
 * of the functions
 * \code
 * #include "OneClass.h"
 * LOG_REGISTER_MODULE ("OneClass")
 * 
 * OneClass::OneClass () 
 * {
 *      MBEG 
 *      MEND
 * }
 * ...
 * \endcode
 */
#define LOG_REGISTER_MODULE(name) \
  static LogModule s_LogModule(name);

/**
 * \ingroup logging
 * \brief Set logging level for a particular logging module. It can me called anywhere but is is 
 * preferable  to be called from the main file within the code. This overrides the configuration set in
 * the environment variable
 * \code
 * int
 * main ( int argc, char** argv )
 * {
 *      LOG_SET_LEVEL("OneClass", LogLevel::ALL)
 *      ...
 * }
 * \encode
 */
#define LOG_SET_LEVEL(name, level) \
  LogManager::Instance().SetLogLevel(name, level);

/**
 * \ingroup logging
 * \brief Set logging level to all modules. It overrides the information set in the environment 
 * variable
 */
#define LOG_SET_ALL_LEVEL(level) \
  LogManager::Instance().SetLogLevel(level);

/**
 * \ingroup logging
 * \brief Redirect std::clog. It is preferable to be used in the main file within the code. 
 * The std::ofstream closes the file at destruction 
 * \code
 * ...
 * LOG_SET_OUT ("MyLogFile.log")
 * ...
 * \endcode
 */
#define LOG_SET_OUT(name)              \
  LogRedirected = true;                \
  static std::ofstream s_logOut(name); \
  std::clog.rdbuf(s_logOut.rdbuf());

#ifdef LOG_ENABLED

#ifndef LOG_ONLY_HEADERS

/**
 * \ingroup logging
 * \brief Function begin. Only enabled with logging level ALL
 */
#define BEG                                                                      \
  if (s_LogModule.GetLevel() == LogLevel::ALL)                                   \
    LogPrint(WHITE, __TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : [IN]      ", \
             __METHOD_NAME__, "\n");                                             \
  LogPrint(RESET);

/**
 * \ingroup logging
 * \brief Function out. Only enabled with logging level ALL
 */
#define END                                                                      \
  if (s_LogModule.GetLevel() == LogLevel::ALL)                                   \
    LogPrint(WHITE, __TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : [OUT]     ", \
             __METHOD_NAME__, "\n");                                             \
  LogPrint(RESET);

/**
 * \ingroup logging
 * \brief Function begin. Only enabled with logging level ALL
 */
#define BEGEND                                                            \
  if (s_LogModule.GetLevel() == LogLevel::ALL)                            \
    LogPrint(__TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : [IN-OUT]  ", \
             __METHOD_NAME__, "\n");                                      \
  LogPrint(RESET);

/**
 * \ingroup logging
 * \brief Debug information ouput. Enabled with logging levels DEBUG and upper
 */
#define DBG(...)                                                                         \
  if (s_LogModule.GetLevel() >= LogLevel::DEBUG)                                         \
    LogPrint(BOLDGREEN, __TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : [DEBUG]   ==> ", \
             __VA_ARGS__, "\n");                                                         \
  LogPrint(RESET);

/**
 * \ingroup logging
 * \brief Generic information output. Enabled with logging levels INFO and upper
 */
#define INFO(...)                                                                       \
  if (s_LogModule.GetLevel() >= LogLevel::INFO)                                         \
    LogPrint(BOLDCYAN, __TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : [INFO]    ==> ", \
             __VA_ARGS__, "\n");                                                        \
  LogPrint(RESET);

/**
 * \ingroup logging
 * \brief Warning information. Enabled with logging levels WARNING and upper
 */
#define WARN(...)                                                               \
  if (s_LogModule.GetLevel() >= LogLevel::WARNING)                              \
  {                                                                             \
    if (!LogRedirected)                                                         \
    {                                                                           \
      LogPrint(WARNING_COL, __TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : ",  \
               "[WARNING] ==> ", __VA_ARGS__, RESET, "\n");                     \
      LogPrint("");                                                             \
    }                                                                           \
    else                                                                        \
    {                                                                           \
      LogPrint(__TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : [WARNING] ==> ", \
               __VA_ARGS__, "\n");                                              \
    }                                                                           \
  }                                                                             \
  LogPrint(RESET);

/**
 * \ingroup logging
 * \brief Error information. Enabled with logging levels ERROR and upper
 */
#define ERROR(...)                                                              \
  if (s_LogModule.GetLevel() >= LogLevel::ERROR)                                \
  {                                                                             \
    if (!LogRedirected)                                                         \
    {                                                                           \
      LogPrint(ERROR_COL, __TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : ",    \
               "[ERROR]   ==> ", __VA_ARGS__, RESET, "\n");                     \
    }                                                                           \
    else                                                                        \
    {                                                                           \
      LogPrint(__TIME_INFO__ __FILE_INFO__ __MODULE_NAME__ " : [ERROR]   ==> ", \
               __VA_ARGS__, "\n");                                              \
    }                                                                           \
  }                                                                             \
  LogPrint(RESET);
#else // DEFINED LOG_ONLY_HEADERS
#define BEG
#define END
#define BEGEND
#define DBG(...)
#define INFO(...)
#define WARN(...)
#define ERROR(...)
#endif // #ifndef LOG_ONLY_HEADERS

/**
 * \ingroup logging
 * \brief Assertion with message. Is is disabled only if logging system is. 
 * It ends the program by calling std::quick_exit(EXIT_FAILURE), so that some 
 * code can be finished before exiting (e.g. end writing in a file)
 */
#define MSG_ASSERT(expr, ...)                                         \
  if (!(expr))                                                        \
  {                                                                   \
    LogPrint(ASSERT_COL, __TIME_INFO__ "[", __FILE__, ": ", __LINE__, \
             "] [ASSERT]  ==> ", __VA_ARGS__, RESET, "\n");           \
    std::quick_exit(EXIT_FAILURE);                                    \
    LogPrint("");                                                     \
  }

/**
 * Logging to be done within .h files, this does NOT obey logging levels . 
 * It is usefull for library files and those which use templates. 
 * The function name is not shown as ClassName::MethodName but only the method same
 * is shown. The file line is also shown.
 * 
 */

#ifndef LOG_ONLY_MODULES
/**
 * \ingroup logging
 * \brief Unbound function begin
 */
#define UBEG                                                              \
  if (!LogRedirected)                                                     \
    LogPrint(HEADERS_COL, __TIME_INFO__ "[", __FILE__, ": ", __LINE__,    \
             "] [IN]      ", __FUNCTION__, RESET, RESET, "\n");           \
  else                                                                    \
    LogPrint(__TIME_INFO__ "[", __FILE__, ": ", __LINE__, "] [IN]      ", \
             __FUNCTION__, "\n");

/**
 * \ingroup logging
 * \brief Unbound function end
 */
#define UEND                                                              \
  if (!LogRedirected)                                                     \
    LogPrint(HEADERS_COL, __TIME_INFO__ "[", __FILE__, ": ", __LINE__,    \
             "] [OUT]     ", __FUNCTION__, RESET, RESET, "\n");           \
  else                                                                    \
    LogPrint(__TIME_INFO__ "[", __FILE__, ": ", __LINE__, "] [OUT]     ", \
             __FUNCTION__, "\n");

/**
 * \ingroup logging
 * \brief Unbound debugging information
 */
#define UINFO(...)                                                           \
  if (!LogRedirected)                                                        \
    LogPrint(HEADERS_COL, __TIME_INFO__ "[", __FILE__, ": ", __LINE__,       \
             "] [INFO]   ==> ", __VA_ARGS__, RESET, RESET, "\n");            \
  else                                                                       \
    LogPrint(__TIME_INFO__ "[", __FILE__, ": ", __LINE__, "] [INFO]   ==> ", \
             __VA_ARGS__, "\n");

/**
 * \ingroup logging
 * \brief Unbound warning information
 */
#define UWARN(...)                                                            \
  if (!LogRedirected)                                                         \
    LogPrint(WARNING_COL, __TIME_INFO__ "[", __FILE__, ": ", __LINE__, "] ",  \
             "[WARNING] ==> ", __VA_ARGS__, RESET, RESET, "\n");              \
  else                                                                        \
    LogPrint(__TIME_INFO__ "[", __FILE__, ": ", __LINE__, "] [WARNING] ==> ", \
             __VA_ARGS__, "\n");

/**
 * \ingroup logging
 * \brief Unbound Error information
 */
#define UERROR(...)                                                            \
  if (!LogRedirected)                                                          \
    LogPrint(ERROR_COL, __TIME_INFO__ "[", __FILE__, ": ", __LINE__, "] ",     \
             "[ERROR]   ==> ", __VA_ARGS__, RESET, RESET, "\n");               \
  else                                                                         \
    LogPrint(__FILE_INFO__ "[", __FILE__, ": ", __LINE__, "] [ERROR]    ==> ", \
             __VA_ARGS__, "\n");
#else // DEFINED LOG_ONLY_MODULES
#define UBEG
#define UEND
#define UINFO(...)
#define UWARN(...)
#define UERROR(...)
#endif // #ifndef LOG_ONLY_MODULES

#else // NOT DEFINED LOG_ENABLED
#define BEG
#define END
#define BEGEND
#define DBG(...)
#define INFO(...)
#define WARN(...)
#define ERROR(...)
#define MSG_ASSERT(expr, ...)
#define UBEG
#define UEND
#define UINFO(...)
#define UWARN(...)
#define UERROR(...)
#endif // #ifdef LOG_ENABLED

/****************************************** LOGGING IMPLEMNTATION *********************************/
#define __MODULE_NAME__ "[", s_LogModule.GetPrintName(), "]",

#ifdef LOG_FILE_ENABLED
#define __FILE_INFO__ "[", __FILE__, ": ", __LINE__, "]\t",
#else
#define __FILE_INFO__
#endif

#ifdef LOG_TIME_ENABLED
#define __TIME_INFO__ StrTime(), " ",
#else
#define __TIME_INFO__
#endif

/**
 * \ingroup logging
 * \brief Variadic TMP print
 */
template <typename T>
void LogPrint(const T &t)
{
  std::clog << t << "";
}

template <typename HEAD, typename... TAIL>
void LogPrint(HEAD head, TAIL... tail)
{
  LogPrint(head);
  LogPrint(tail...);
}

/**
 * \ingroup logging
 * \brief Extract user readable name of the function ( class::function) from __PRETTY_FUNCTION__
 * compiler macro
 */
inline std::string FuncName(std::string name)
{
  name = name.substr(0, name.rfind("("));
  name = name.substr(name.rfind(" ") + 1, name.size());
  return name;
}
#define __METHOD_NAME__ FuncName(__PRETTY_FUNCTION__)

/**
 * \ingroup logging
 * \brief Give time in a given format
 */
inline std::string StrTime(void)
{
  time_t rawtime;
  char buffer[9];
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(buffer, 9, "%H:%M:%S", timeinfo);
  return std::string(buffer);
}

/********************************************************************************************************/
/*********************************Log classes***********************************************************/

/**
 * \ingroup logging
 * \brief Singleton class to store all log modules
 */
class LogModule;

class LogManager
{
public:
  static LogManager &Instance(void)
  {
    static LogManager dm;
    return dm;
  }

  /**
     * \brief Each log module auto-register in the log manager at construction. 
     * \param logMododule
     * \return
     */
  void Register(LogModule &logModule);

  /**
     * \brief Sets a logging level for a module
     * \param moduleName
     * \param level
     * \return
     */
  void SetLogLevel(std::string moduleName, LogLevel level);

  /**
     * \brief Set a logging level to ALL modules
     * \param level
     * \return
     */
  void SetLogLevel(LogLevel level);

  static std::size_t m_maxModuleNameSize;

private:
  using ModulesMap_t = std::map<std::string, LogModule &>;
  ModulesMap_t m_modules;

  LogManager() = default;
  ~LogManager() = default;
  LogManager(const LogManager &) = delete;
  const LogManager &operator=(const LogManager &) = delete;
};

/**
 * \ingroup logging
 * \brief Class to define a logging
 */
class LogModule
{
public:
  /**
     * \brief 
     * \param name Module name
     * \param level Logging level
     */
  LogModule(std::string name, LogLevel level = LogLevel::NONE);
  LogModule() = delete;

  /**
     * \brief Get the name of the module
     * \return 
     */
  std::string GetModuleName(void) const;

  /**
     * \brief Get the name of the module to be printed
     * \return 
     */
  std::string GetPrintName(void) const;

  /**
     * \brief Set level
     * \param level
     * \return
     */
  void SetLevel(LogLevel level);

  /**
     * \brief Get level
     * \return Logging level
     */
  LogLevel GetLevel(void) const;

private:
  /**
     * \brief Check environment variable defined by LOG_ENV_VAR_NAME
     */
  void CheckEnvVar(void);

  const std::string m_name;
  LogLevel m_level;
};

#endif /* LOG_H */