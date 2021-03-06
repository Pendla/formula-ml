#include "CarModel.h"
#include <iostream>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

const float mass = 642.0f;				// [kg]
const float my = 0.8f;				// µ - friction coefficient. Guessed
const float g = 9.82f;
const float downforceConstant = mass * g / 36.1111f / 36.1111f; // c_down * v^2 = mg at 130 km/h
const float dragConstant = g / 83.3333f / 83.3333f; // c_drag * v^2 = 1g at 300 km/h
const float gasForce = 14.2f * mass;		// [N]
const float brakeForce = 39.0f * mass;	    // [N]
const float minTurningRadius = 10.0f;		// Guessed
const float maxCentipetalForce = 2500.0f;	// Guessed [N]

using namespace glm;

CarModel::CarModel(glm::vec3 position, glm::vec3 direction, float initial_speed, float min_speed, float max_speed) {
    model                   = new Model("./res/models/car.model");
    this->position          = position;
    this->initial_position  = position;
	this->measure_point		= position;

    this->direction         = direction;
    this->initial_direciton = direction;

    velocity                = direction * initial_speed;
    this->initial_speed     = initial_speed;

    this->max_speed         = max_speed;
    this->initial_max_speed = max_speed;

    this->min_speed         = min_speed;
    this->initial_min_speed = min_speed;

    checkpoint              = 1;
    distance_on_track       = 0;
	current_control         = CarControl();
}

CarModel::~CarModel() {

}

void CarModel::reset() {
    position                        = initial_position;
    direction                       = initial_direciton;
    min_speed                       = initial_min_speed;
    max_speed                       = initial_max_speed;
    checkpoint                      = 1;
    distance_on_track               = 0;
    velocity                        = direction * initial_speed;
    current_control.acceleration    = 0;
    current_control.steer           = 0;
}

float CarModel::getSpeed() {
	return glm::length(velocity);
}
void CarModel::setSpeed(float speed) {
	velocity = speed * direction;
}

Model* CarModel::get_model() {
    return model;
}

// Calculate the minimum the radius the car can handle at
// current "speed" and engine/brake "forwardForce"

float CarModel::min_turning_radius() {
	auto speed = getSpeed();
	auto result = mass*speed*speed / maxCentipetalForce;
	return glm::max(result, minTurningRadius);
}

float CarModel::maxRotation(float speed, float dt, float minimum_radius) {
	return speed * dt / minimum_radius;
}

void smoothChange(float* value, float new_value, float dt, float value_range) {
    const float max_change_time = 0.35f;
    const float max_change = dt * value_range / max_change_time;
    float change = new_value - *value;
    *value += change >= 0 ? glm::min(change, max_change) : glm::max(change, -max_change);
}
void CarModel::steer(float current_speed, float dt) {
	//Make the rotation to not be bigger than the maximum allowed rotation

	float rotation;
	float max_rotation = maxRotation(current_speed, dt, min_turning_radius());
	float desired_rotation = minTurningRadius * current_control.steer;

	if (max_rotation < desired_rotation || max_rotation < desired_rotation*-1) {
		if (desired_rotation < 0) {
			rotation = max_rotation * -1;
		}
		else {
			rotation = max_rotation;
		}
	}
	else {
		rotation = desired_rotation;
	}

	direction = glm::rotateZ(direction, rotation);
	velocity = glm::rotateZ(velocity, rotation);
}

void CarModel::update(float dt, struct CarControl control) {

    // Modify acceleration to cruise more often
    if (current_control.acceleration < 0.1 && current_control.acceleration > -0.1) {
        current_control.acceleration = 0.0;
    } else {
        if (current_control.acceleration < 0) {
            current_control.acceleration = (current_control.acceleration + 0.1) / 0.9f;
        } else {
            current_control.acceleration = (current_control.acceleration - 0.1) / 0.9f;
        }
    }

	// Update current control state with smoothing
    //smoothChange(&current_control.acceleration, control.acceleration, dt, 1.f);
    //smoothChange(&current_control.steer, control.steer, dt, 2.f);

    current_control = control;

    float currentSpeed = getSpeed();

    // Acceleration in the direction of the car
    float forwardForce = 0;

	if (current_control.acceleration > 0) {
		if (currentSpeed < max_speed) {
			forwardForce = gasForce * current_control.acceleration;
			velocity += direction * (forwardForce * dt / mass);
		}
	}
	else if (current_control.acceleration < 0 && currentSpeed > min_speed) {
		forwardForce = brakeForce * current_control.acceleration;
		velocity += direction * (forwardForce * dt / mass);
		if (length(normalize(velocity) + direction) < 1) {
			velocity *= 0;
		}
	}

    // Rotation due to steering
    if (current_control.steer != 0) {
		steer(currentSpeed, dt);
    }

    // Apply velocity
    position += velocity * dt;
}
