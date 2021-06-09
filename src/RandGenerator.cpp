#include "RandGenerator.h"
#include "Log.h"

LOG_REGISTER_MODULE("RandGenerator");

RandGen::RandGen()
{
  BEG;
  m_expD = ExpDist_t(1.0);
  m_func = std::bind(m_expD, g_randGen);
  END;
}

RandGen::RandGen(DistType dt, double param)
{
  BEG;
  switch (dt)
  {
  case DistType::Exponential:
    m_expD = ExpDist_t(param);
    m_func = std::bind(m_expD, g_randGen);
    break;
  case DistType::Uniform:
    m_uniD = UniDist_t(0, param);
    m_func = std::bind(m_uniD, g_randGen);
    break;
  case DistType::Constant:
    m_const = param;
    //            m_func = std::bind(&RandGen::ConstCall, this, g_randGen);
    break;
  default:
    MSG_ASSERT(false, "Unknown distribution time");
  }
  END;
}

RandGen::RandGen(DistType dt, double min, double max)
{
  BEG;
  switch (dt)
  {
  case DistType::Uniform:
    m_uniD = UniDist_t(min, max);
    m_func = std::bind(m_uniD, g_randGen);
    break;
  default:
    MSG_ASSERT(false, "Unknown distribution time");
  }
  END;
}

double
RandGen::operator()()
{
  BEGEND;
  return m_func();
}

double
RandGen::ConstCall()
{
  BEGEND;
  return m_const;
}

double
GetRandExpSample(double rate)
{
  ExpDist_t d(rate);
  return d(g_randGen);
}

double
GetRandUniSample(double max)
{
  UniDist_t d(0, max);
  return d(g_randGen);
}

double
GetRandSample(DistType dt, double val)
{
  //    return GetRandExpSample(val);
  if (dt == DistType::Exponential)
  {
    return GetRandExpSample(val);
  }
  else if (dt == DistType::Uniform)
  {
    return GetRandUniSample(val);
  }
  else // if (dt == DistType::Constant) {
  {
    return 1.0 / val;
  }
  //    MSG_ASSERT(false, "Wrong distribution type");
};
