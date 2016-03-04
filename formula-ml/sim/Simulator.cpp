#include "Simulator.h"
#include <neural/FixedNetwork.h>
#include <iostream>

using namespace neural;

Simulator::Simulator() {
	result = SimulationResult();
}

Simulator::~Simulator() {
}

/*
    Angle between two normalized vectors
*/
inline float angle(glm::vec3 v1, glm::vec3 v2) {
    float angle;

    // Set the angle
    float dot = glm::dot(v1, v2);
    if (dot > 0.9999999f) {
        angle = 0;
    }
    else if (dot < -0.9999999f) {
        // TODO: use definition of pi
        // TODO: use a number slightly less than pi, for stability reasons?
        angle = 3.14159265359f; 
    }
    else {
        angle = glm::acos(glm::dot(v1, v2));
    }

    // Set the correct sign
    if (glm::cross(v1, v2).z < 0) {
        angle = -angle;
    }
    return angle;
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
    glm::vec3 line = glm::normalize(next_checkpoint - last_checkpoint);
    return angle(line, car->direction);
}

/*
Write to 'target' with 'offset'
Updates offset to include what write_track_curve() writes
*/
void Simulator::write_checkpoints(float* target, int& offset, int nbr_of_checkpoints) {

    int current_checkpoint = glm::max(car->checkpoint, 0);

    glm::vec3 next_checkpoint = track->get_checkpoints()[current_checkpoint].middle;
    glm::vec3 last_line = next_checkpoint - car->position;

    // Angle + distance to the next checkpoint, from the perspective of the car
    if (nbr_of_checkpoints >= 1) {
        target[offset++] = angle(car->direction, glm::normalize(last_line));
        target[offset++] = glm::length(last_line);
    }

    // Angle + distance to the next checkpoint, from the perspective of the last checkpoint
    glm::vec3 last_checkpoint = next_checkpoint;
    for (int i = 2; i <= nbr_of_checkpoints; i++) {
        current_checkpoint++;
        next_checkpoint = track->get_checkpoints()[current_checkpoint].middle;

        glm::vec3 next_line = next_checkpoint - last_checkpoint;

        target[offset++] = angle(glm::normalize(next_line), glm::normalize(last_line));
        target[offset++] = glm::length(next_line);

        last_line = next_line;
        last_checkpoint = next_checkpoint;
    }
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

        // Check termination limits
        if (result.distance_driven >= termination_distance) {
            // Call it quits
            result.distance_driven = termination_distance;
            break;
        }

		// Check for progress
		if (result.distance_driven > best.distance_driven) {
			// The car has progressed
			best = result;
		} else if(result.time_alive > best.time_alive + progress_timeout) {
			// No progress for a while
			// Call it quits
			//break;
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

    if (!track->on_track(car->position)) {
        car->setSpeed(0.0f);
    }

}
