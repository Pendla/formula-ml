#ifndef FORMULA_ML_EXPERIMENT_H
#define FORMULA_ML_EXPERIMENT_H

#include <sim/Simulator.h>

class Experiment {
public:
    ~Experiment() {
        delete simulator;
    }

    virtual void run() = 0;

protected:
    Simulator* simulator;
};

#endif