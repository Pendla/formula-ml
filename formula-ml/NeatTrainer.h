#ifndef NEAT_TRAINER
#define NEAT_TRAINER

#include <neural/neat/Pool.h>
#include <neural/neat/Network.h>

class NeatTrainer
{
public:
    NeatTrainer();
    ~NeatTrainer();

    void run();

    neural::Network* train();

private:

    void evaluate(neat::Genome& genome);
    void showBest();

    void evaluateFitness(neat::Genome& genome);
    neat::Pool * pool;
    neat::Network best;
};

#endif