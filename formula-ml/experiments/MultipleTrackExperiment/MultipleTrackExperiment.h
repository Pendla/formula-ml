#ifndef MULTIPLE_TRACK_EXPERIMENT
#define MULTIPLE_TRACK_EXPERIMENT

#include <experiments/Experiment.h>
#include <experiments/ExperimentWindow.h>
#include <experiments/SimulationEvaluator.h>

#include <neural/neat/Trainer.h>
#include <neural/neat/Network.h>

class MultipleTrackExperiment : public Experiment {
public:
    MultipleTrackExperiment();
    MultipleTrackExperiment(string path);
    ~MultipleTrackExperiment();
    virtual void run();

private:
    string load_network_path;

    std::shared_ptr<neat::Trainer> trainer;
    std::shared_ptr<ExperimentWindow> window;

    class MultipleTrackEvaluator : public neat::Evaluator{
    public:
        MultipleTrackEvaluator(AiSettings ai_settings);
        ~MultipleTrackEvaluator();

        EvaluationResult evaluate_network(neat::Network* network);
        static void print(const EvaluationResult& result);
        void reset();

        std::vector<SimulationEvaluator*> evaluators;
    };
};

#endif