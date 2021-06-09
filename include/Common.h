#ifndef COMMON_H
#define COMMON_H

#include <random>
#include <map>
#include <cstdint>
#include <vector>
#include <ostream>
#include <chrono>
#include <type_traits>

#include "json.h"
#include "Log.h"

using json = nlohmann::json;
#define JSON_CHECK(j, str) \
  MSG_ASSERT(j.contains(str), "JSON does not contain " + std::string(str))

inline bool
JsonContains(const json &j, std::string &&str)
{
  json::json_pointer p(str);
  return j.contains(p);
}

template <class T = std::string>
inline T
JsonCheckAndGet(const json &j, std::string &&str)
{
  json::json_pointer p(str);
  MSG_ASSERT(j.contains(p), "JSON does not contain " + str);
  return T(j.at(p));
}

/*************************/
// static std::default_random_engine g_randGen(0);
static std::random_device rd;
static std::default_random_engine g_randGen(rd());
using ExpDist_t = std::exponential_distribution<double>;
using UniDist_t = std::uniform_real_distribution<double>;

enum class SimulType : std::uint8_t
{
  TrafficLimited,
  TimeLimited
};
static const std::map<SimulType, std::string> s_simType2Str{
    {SimulType::TrafficLimited, "TRAFFIC_LIMITED"},
    {SimulType::TimeLimited, "TIME_LIMITED"}};
static const std::map<std::string, SimulType> s_str2SimType{
    {"TRAFFIC_LIMITED", SimulType::TrafficLimited},
    {"TIME_LIMITED", SimulType::TimeLimited}};

#define SIMUL_TYPE_2_STR(type) \
  s_simType2Str.at(type)

#define STR_2_SIMUL_TYPE(type) \
  s_str2SimType.at(type)

enum class DistType : std::uint8_t
{
  Exponential,
  Uniform,
  Constant
};
static const std::map<DistType, std::string> s_distType2Str{
    {DistType::Exponential, "EXP"},
    {DistType::Uniform, "UNI"},
    {DistType::Constant, "CONST"}};
static const std::map<std::string, DistType> s_str2DistType{
    {"EXP", DistType::Exponential},
    {"UNI", DistType::Uniform},
    {"CONST", DistType::Constant}};

#define DIST_TYPE_2_STR(type) \
  s_distType2Str.at(type)

#define STR_2_DIST_TYPE(type) \
  s_str2DistType.at(type)

/*************************/
enum class EvType_t : std::uint8_t
{
  Arrival,
  Departure,
  NodeStop,
  NodeRestart,
  PacketFinish,
  RegimeChange
};
static const std::map<EvType_t, std::string> s_evTypeStr{
    {EvType_t::Arrival, "ARRIVAL"},
    {EvType_t::Departure, "DEPARTURE"},
    {EvType_t::NodeStop, "NODE_STOP"},
    {EvType_t::NodeRestart, "NODE_RESTART"},
    {EvType_t::PacketFinish, "PKT_FINISH"},
    {EvType_t::RegimeChange, "REGIME_CHANGE"}};

#define EV_TYPE_2_STR(type) \
  s_evTypeStr.at(type)

/*************************/
using Id_t = std::int64_t;
using Time_t = double;
constexpr Id_t NULL_ID = -1;
constexpr Id_t ZN_ID = 0;

/*************************/
inline double
UnitRand(void)
{
  return ((double)rand() / (RAND_MAX));
}

/*************************/
template <class T = double>
using Matrix = std::vector<std::vector<T>>;

template <class T>
inline void InitMatrix(Matrix<T> &m, std::uint32_t rows, std::uint32_t cols, T &value)
{
  m.clear();
  m.resize(rows);
  for (auto i = 0u; i < rows; ++i)
  {
    m[i].resize(cols);
    for (auto j = 0u; j < cols; ++j)
    {
      m[i][j] = value;
    }
  }
}

template <class T>
inline void InitVector(std::vector<T> &m, std::uint32_t nelems, T &value)
{
  m.clear();
  m.resize(nelems);
  for (auto i = 0u; i < nelems; ++i)
  {
    m[i] = value;
  }
}

template <class T>
inline std::ostream &operator<<(std::ostream &os, const Matrix<T> &m)
{
  os << "[";
  for (auto i = 0u; i < m.size(); ++i)
  {
    os << "\n  [";
    for (auto j = 0u; j < m[i].size(); ++j)
    {
      os << m[i][j];
      if (j != m[i].size() - 1)
      {
        os << ", ";
      }
    }
    os << "]";
  }
  os << "\n]";
  return os;
}

template <class T>
inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &m)
{
  os << "[";
  for (auto j = 0u; j < m.size(); ++j)
  {
    os << m[j];
    if (j != m.size() - 1)
    {
      os << ", ";
    }
  }
  os << "]";
  return os;
}

template <class T>
inline T Mean(const std::vector<T> &v)
{
  auto acc = std::accumulate(v.begin(), v.end(), 0.0);
  return acc / v.size();
}

template <class T>
inline T Variance(const std::vector<T> &v)
{
  auto mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
  auto sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
  return (sq_sum / v.size() - (mean * mean));
}

using SysTime = std::chrono::high_resolution_clock;
using SysTimePoint = SysTime::time_point;
using fsec = std::chrono::duration<double>;
#define SYSTEM_NOW SysTime::now();

inline double
DurationSec(const SysTimePoint &end, const SysTimePoint &start)
{
  return fsec(end - start).count();
};

#endif /* COMMON_H */
