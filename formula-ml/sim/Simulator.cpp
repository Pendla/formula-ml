#include "Simulator.h"
#include <neural/FixedNetwork.h>
#include <iostream>

using namespace neural;

Simulator::Simulator() {
	result = SimulationResult();
}

Simulator::~Simulator() {
}

float Simulator::distance_to_middle() {
    glm::vec3 line = glm::normalize(track->get_checkpoints()[car->checkpoint].middle - track->get_checkpoints()[car->checkpoint - 1].middle);
    glm::vec3 car_pos = car->position - track->get_checkpoints()[car->checkpoint - 1].middle;
    glm::vec3 right = glm::cross(line, glm::vec3(0, 0, 1));
    return glm::dot(car_pos, right);
}

float Simulator::angle_to_line() {
    glm::vec3 last_checkpoint = track->get_checkpoints()[glm::max(car->checkpoint - 1, 0)].middle;
    glm::vec3 next_checkpoint = track->get_checkpoints()[glm::max(car->checkpoint, 0)].middle;
    glm::vec3 car_position = car->position;

    // move frame of reference to last_checkpoint
    next_checkpoint -= last_checkpoint;
    car_position -= last_checkpoint;
    glm::vec3 next_checkpoint_normalized = glm::normalize(next_checkpoint);

    // Find angle to line
    //float angle_to_line = glm::angle(next_checkpoint, sim->car->direction);
    float angle_to_line = glm::acos(glm::dot(next_checkpoint, car->direction) / glm::length(next_checkpoint) * glm::length(car->direction));
    if (glm::cross(glm::vec3(0, 1, 0), car->direction).z > 0)
        angle_to_line = -angle_to_line;

    return angle_to_line;
}

/*
	Run a complete simulation until failure
	Each simulation step update with time dt
*/
SimulationResult Simulator::run(const float dt) {
	SimulationResult best = SimulationResult();

	while (true) {
		update(dt);

		// Check for crash
		if (!track->on_track(car->position)) {
			// Call it quits
			break;
		}

		// Check for progress
		if (result.distance_driven > best.distance_driven) {
			// The car has progressed
			best = result;
		} else if(result.time_alive > best.time_alive + progress_timeout) {
			// No progress for a while
			// Call it quits
			break;
		}
	}
	return result;
}

/*
	Update the simulation with one time step dt [seconds]
*/
void Simulator::update(float dt) {
	// Update result
	result.time_alive += dt;
    result.distance_driven = car->distance_on_track;

    Checkpoint next_checkpoint = track->get_checkpoints()[car->checkpoint];
    glm::vec3 car_p = car->position - next_checkpoint.left;
    glm::vec3 gate = glm::normalize(next_checkpoint.left - next_checkpoint.right);
    glm::vec3 point_on_gate = glm::dot(car_p, gate) * gate;

    if (glm::length(car_p - point_on_gate) < 0.5) {
        car->distance_on_track = track->get_checkpoints()[car->checkpoint].distance_on_track;
        car->checkpoint++;
    } else {
        int last_checkpoint = glm::max(car->checkpoint - 1, 0);
        float d = track->get_checkpoints()[last_checkpoint].distance_on_track + glm::distance(car->position, track->get_checkpoints()[last_checkpoint].middle);
        car->distance_on_track = d;
    }

    CarControl control = this->carUpdater();
	car->update(dt, control);
}