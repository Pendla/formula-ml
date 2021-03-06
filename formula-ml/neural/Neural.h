#ifndef NEURAL_NEURAL
#define NEURAL_NEURAL
#include <glm/glm.hpp>

namespace neural {

    struct NetworkIO {
        float * values;
        int value_count;
    };

    class Network {
    public:
        virtual void fire(const NetworkIO &input, NetworkIO &output) = 0;
		

        virtual int inputSize() = 0;
        virtual int outputSize() = 0;

	
    };

    inline float sigmoid(float x) {
        return 2.0f / (1.0f + glm::exp(-x)) - 1;
    }

} // namespace neural

#endif 
