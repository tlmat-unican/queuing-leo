#ifndef RANDGENERATOR_H
#define RANDGENERATOR_H
#include <functional>
#include "Common.h"

class RandGen {
public:
    RandGen();
    RandGen(DistType dt, double param);
    RandGen(DistType dt, double min, double max);
    double operator()();

    std::function<double() > m_func;
    ExpDist_t m_expD;
    UniDist_t m_uniD;
    double m_const;
private:
    double ConstCall();
};

double GetRandExpSample(double rate);
double GetRandUniSample(double max);
double GetRandSample(DistType dt, double val);

#endif /* RANDGENERATOR_H */