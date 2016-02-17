#include "Simulator.h"
#include <neural/FixedNetwork.h>
#include "core/Keyboard.h"

using namespace neural;

Simulator::Simulator() {
	track = new TrackModel();
//    glm::vec3 *checkpoints;
//    int* checkpoint_count;
//    track->get_checkpoints(&checkpoints, checkpoint_count);
    // TODO: Set checkpoints for car to establish distance driven. SIMON AT WORK!

    car = new CarModel();

    // Set input values
    const int TRACK_GRID_WIDTH = 16;
    const int TRACK_GRID_DEPTH = 16;
    const int OTHER_INPUTS = 1;

    network_indata = NetworkIO();
    network_indata.value_count = OTHER_INPUTS + TRACK_GRID_WIDTH * TRACK_GRID_DEPTH;
    network_indata.values = new float[network_indata.value_count];

    grid = TrackGrid();
    grid.data = &network_indata.values[OTHER_INPUTS];
    grid.size = TRACK_GRID_WIDTH * TRACK_GRID_DEPTH;
    grid.width = TRACK_GRID_WIDTH;
    grid.depth = TRACK_GRID_DEPTH;
    grid.cell_size = 5;
    grid.value_track = 1;
    grid.value_not_track = 0;

    network = new FixedNetwork(2,1,1,2);
}

Simulator::~Simulator() {
	if (car) delete car;
	if (track) delete track;
    if (network) delete network;
}


void Simulator::update(float dt) {

	// Car data
	network_indata.values[0] = car->getSpeed();

	// Track data
	track->fillTrackMatrix(grid, car->position, car->direction);

	// Fire network
	/*
    NetworkIO out = network->fire(in);
	*/

	// Use network output
	// Assume 'out' to be of correct size and ranges
	CarControl control = CarControl();
	/*
	control.acceleration = out.values[0];
	control.brake = out.values[1];
	control.steer = out.values[2];
	*/

	bool gas = isKeyDown(GLFW_KEY_UP);
	bool brake = isKeyDown(GLFW_KEY_DOWN);
	bool steerLeft = isKeyDown(GLFW_KEY_LEFT);
	bool steerRight = isKeyDown(GLFW_KEY_RIGHT);
	bool steerCareful = isKeyDown(GLFW_KEY_RIGHT_CONTROL);
	bool accelerateMax = isKeyDown(GLFW_KEY_RIGHT_SHIFT);

	if (gas) {
		control.acceleration = accelerateMax ? 1 : 0.5f;
	}
	if (brake) {
		control.brake = accelerateMax ? 1 : 0.5f;
	}

	if (steerLeft && !steerRight) {
		control.steer = steerCareful ? 0.5f : 1;
	} else if (!steerLeft && steerRight) {
		control.steer = steerCareful ? -0.5f : -1;
	}

	car->update(dt, control);
    if (track->on_track(car->position)) {
        // On model.
    }
}