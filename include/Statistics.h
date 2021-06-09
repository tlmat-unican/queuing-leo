#ifndef STATISTICS_H
#define STATISTICS_H

class Statistics {
public:

    static Statistics& GetInstance() {
        static Statistics instance;
        return instance;
    }


private:
    Statistics();
    Statistics(Statistics const&) = delete;
    void operator=(Statistics const&) = delete;

};

#endif /* STATISTICS_H */

