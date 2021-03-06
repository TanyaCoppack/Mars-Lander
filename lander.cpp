// Mars lander simulator
// Version 1.11
// Mechanical simulation functions
// Gabor Csanyi and Andrew Gee, August 2019

// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation, to make use of it
// for non-commercial purposes, provided that (a) its original authorship
// is acknowledged and (b) no modified versions of the source code are
// published. Restriction (b) is designed to protect the integrity of the
// exercise for future generations of students. The authors would be happy
// to receive any suggested modifications by private correspondence to
// ahg@eng.cam.ac.uk and gc121@eng.cam.ac.uk.

#include "lander.h"

void autopilot (void)
  // Autopilot to adjust the engine throttle, parachute and attitude control
{
    static bool first_loop = true;
    ofstream fout;

    double h, error, Kh, Kp, Pout, delta;
    Kh = 0.03;
    Kp = 0.75;
    delta = 0.5;

    h = position.abs() - MARS_RADIUS;
    error = -(0.5 + Kh * h + velocity * position.norm());
    Pout = Kp * error;

    //throttle
    if(Pout <= -delta) {
        throttle = 0;
    }
    else if (-delta < Pout < 1) {
        throttle = delta + Pout;
    }
    else if (Pout >= 1 - delta) {
        throttle = 1;
    }

    if (velocity * position.norm() < -770) {
        throttle = 1;
    }

    //draw graph
    if (first_loop == true) { //if it is the first loop

        fout.open("results.txt");  //create a file with altitudes
        fout << h << " " << velocity * position.norm() << endl;
    }
    else { //if it is not the first loop

        if (fout) {

            cout << "nice one";
            fout.open("results.txt", std::ios_base::app);  //append to text file rather than overwrite
            fout << h << " " << velocity * position.norm() << endl;
        }
    }

    first_loop = false;

}

void numerical_dynamics (void)
  // This is the function that performs the numerical integration to update the
  // lander's position, velocity and orientation. The time step is delta_t (global variable).
{
    double lander_mass, atm_density; //changes in each  step
    vector3d acceleration, position_next, velocity_next, gravity, lander_drag, chute_drag, drag, thrust;
    static vector3d position_last;

    atm_density = atmospheric_density(position);
    lander_mass = UNLOADED_LANDER_MASS + FUEL_CAPACITY * FUEL_DENSITY * fuel;
    gravity = - (GRAVITY * MARS_MASS * lander_mass / position.abs2() )* position.norm();
    lander_drag = -0.5 * atm_density * velocity.abs2() * velocity.norm() * DRAG_COEF_LANDER * M_PI * LANDER_SIZE * LANDER_SIZE;
    thrust = thrust_wrt_world();

    if (parachute_status == DEPLOYED) {

        chute_drag = -0.5 * atm_density * velocity.abs2() * velocity.norm() * DRAG_COEF_CHUTE * 20 * LANDER_SIZE * LANDER_SIZE;
        drag = lander_drag + chute_drag;

    } else {
        drag = lander_drag;
    }
    acceleration = (gravity + drag + thrust) / lander_mass;

    //EULER
    //velocity = velocity + acceleration * delta_t;
    //position = position + velocity * delta_t;

    //VERLET
    position_last = position - velocity * delta_t;
    position_next = 2 * position - position_last + acceleration * delta_t * delta_t;
    velocity_next = (position_next - position) / delta_t;
    velocity = velocity_next;
    position_last = position;
    position = position_next;
    

  // Here we can apply an autopilot to adjust the thrust, parachute and attitude
  if (autopilot_enabled) autopilot();

  // Here we can apply 3-axis stabilization to ensure the base is always pointing downwards
  if (stabilized_attitude) attitude_stabilization();
}

void initialize_simulation (void)
  // Lander pose initialization - selects one of 10 possible scenarios
{
  // The parameters to set are:
  // position - in Cartesian planetary coordinate system (m)
  // velocity - in Cartesian planetary coordinate system (m/s)
  // orientation - in lander coordinate system (xyz Euler angles, degrees)
  // delta_t - the simulation time step
  // boolean state variables - parachute_status, stabilized_attitude, autopilot_enabled
  // scenario_description - a descriptive string for the help screen

  scenario_description[0] = "circular orbit";
  scenario_description[1] = "descent from 10km";
  scenario_description[2] = "elliptical orbit, thrust changes orbital plane";
  scenario_description[3] = "polar launch at escape velocity (but drag prevents escape)";
  scenario_description[4] = "elliptical orbit that clips the atmosphere and decays";
  scenario_description[5] = "descent from 200km";
  scenario_description[6] = "";
  scenario_description[7] = "";
  scenario_description[8] = "";
  scenario_description[9] = "";

  switch (scenario) {

  case 0:
    // a circular equatorial orbit
    position = vector3d(1.2*MARS_RADIUS, 0.0, 0.0);
    velocity = vector3d(0.0, -3247.087385863725, 0.0);
    orientation = vector3d(0.0, 90.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 1:
    // a descent from rest at 10km altitude
    position = vector3d(0.0, -(MARS_RADIUS + 10000.0), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = false;
    break;

  case 2:
    // an elliptical polar orbit
    position = vector3d(0.0, 0.0, 1.2*MARS_RADIUS);
    velocity = vector3d(3500.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 3:
    // polar surface launch at escape velocity (but drag prevents escape)
    position = vector3d(0.0, 0.0, MARS_RADIUS + LANDER_SIZE/2.0);
    velocity = vector3d(0.0, 0.0, 5027.0);
    orientation = vector3d(0.0, 0.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 4:
    // an elliptical orbit that clips the atmosphere each time round, losing energy
    position = vector3d(0.0, 0.0, MARS_RADIUS + 100000.0);
    velocity = vector3d(4000.0, 0.0, 0.0);
    orientation = vector3d(0.0, 90.0, 0.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = false;
    autopilot_enabled = false;
    break;

  case 5:
    // a descent from rest at the edge of the exosphere
    position = vector3d(0.0, -(MARS_RADIUS + EXOSPHERE), 0.0);
    velocity = vector3d(0.0, 0.0, 0.0);
    orientation = vector3d(0.0, 0.0, 90.0);
    delta_t = 0.1;
    parachute_status = NOT_DEPLOYED;
    stabilized_attitude = true;
    autopilot_enabled = false;
    break;

  case 6:
    break;

  case 7:
    break;

  case 8:
    break;

  case 9:
    break;

  }
}
