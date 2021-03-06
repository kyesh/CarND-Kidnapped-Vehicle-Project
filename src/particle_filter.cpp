/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"

using namespace std;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// TODO: Set the number of particles. Initialize all particles to first position (based on estimates of 
	//   x, y, theta and their uncertainties from GPS) and all weights to 1. 
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).

	num_particles = 50;

	default_random_engine gen;
	normal_distribution<double> dist_x(x, std[0]);
	normal_distribution<double> dist_y(y, std[1]);
	normal_distribution<double> dist_theta(theta, std[2]);

	for(int i = 0; i < num_particles; i++){
		Particle p;
		p.id = i;
		p.x = dist_x(gen);
		p.y = dist_y(gen);
		p.theta = dist_theta(gen);
		p.weight = 1;

		particles.push_back(p);
	}

	is_initialized = true;

}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
	// TODO: Add measurements to each particle and add random Gaussian noise.
	// NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
	//  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	//  http://www.cplusplus.com/reference/random/default_random_engine/

        default_random_engine gen;
        normal_distribution<double> dist_x(0, std_pos[0]*4);
        normal_distribution<double> dist_y(0, std_pos[1]*4);
        normal_distribution<double> dist_theta(0, std_pos[2]*4);

	for(int i =0; i < num_particles; i++){
		//I used simplier movement model approximation to save on computation costs
		particles[i].theta = particles[i].theta + yaw_rate*delta_t;
		particles[i].x = particles[i].x + cos(particles[i].theta)*delta_t;
		particles[i].y = particles[i].y + sin(particles[i].theta)*delta_t;

		//Add noise
		particles[i].theta += dist_theta(gen);
		particles[i].x 	+= dist_x(gen);
		particles[i].y 	+= dist_y(gen);
	}

}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
	//   observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//   implement this method and use it as a helper during the updateWeights phase.



}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		const std::vector<LandmarkObs> &observations, const Map &map_landmarks) {
	// TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	//   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	//   The following is a good resource for the theory:
	//   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33
	//   http://planning.cs.uiuc.edu/node99.html
	weights.clear();
	int b_assoc;
	double b_sense_x, b_sense_y, t_sense_x, t_sense_y, minDist, d, x_diff, y_diff, expon;
	double gauss_norm = (1/(2 * M_PI * std_landmark[0] * std_landmark[1]));
//	cout << "Variables Declard" << endl;
	for(int i = 0; i < num_particles; i++){
		particles[i].weight = 1;
	//	cout << "\t Particle: " << i << endl;
		for(int j = 0; j < observations.size(); j++){
			minDist = 999999999999;
			b_sense_x = 0;
			b_sense_y = 0;
			b_assoc = 0;
	//		cout << "\t\t Initialized Observation: " << j  << " of: " << observations.size()  << endl;
			for(int k =0; k<map_landmarks.landmark_list.size(); k++){
	//			cout << "\t\t\t LandMark: " << k << endl;
				t_sense_x = particles[i].x + (cos(particles[i].theta) * observations[j].x) - (sin(particles[i].theta) * observations[j].y);
				t_sense_y = particles[i].y + (sin(particles[i].theta) * observations[j].x) + (cos(particles[i].theta) * observations[j].y);
				d = dist(map_landmarks.landmark_list[k].x_f, map_landmarks.landmark_list[k].y_f, t_sense_x, t_sense_y);
				if(d<minDist){
					minDist = d;
					b_sense_x = t_sense_x;
					b_sense_y = t_sense_y;
					b_assoc = k;
	//				cout << "\t\t\t\t Updated LandMark" << endl;
				}
				
			}
	//		cout << "\t\t Before Saving closest point" << endl;
			//particles[i].sense_x.push_back(b_sense_x);
			//particles[i].sense_y.push_back(b_sense_y);
			//particles[i].associations.push_back(b_assoc);
			x_diff = b_sense_x - map_landmarks.landmark_list[b_assoc].x_f;
			y_diff = b_sense_y - map_landmarks.landmark_list[b_assoc].y_f;
			expon = ((x_diff*x_diff)/(2*std_landmark[0]*std_landmark[0])) + ((y_diff*y_diff)/(2*std_landmark[1]*std_landmark[1]));
			particles[i].weight *=  (gauss_norm*exp(-expon));
	//		cout << "\t\t gauss_norm:" << gauss_norm << endl;
	//		cout << "\t\t expon: " << expon << endl;
	//		cout << "\t\t Weight After current obs:" << particles[i].weight << endl;
		}
		
		weights.push_back(particles[i].weight);
	//	cout << "\t weights reassigned" << endl;
	}
}

void ParticleFilter::resample() {
	// TODO: Resample particles with replacement with probability proportional to their weight. 
	// NOTE: You may find std::discrete_distribution helpful here.
	//   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
	std::vector<Particle> newParticles;
	default_random_engine gen;
	std::discrete_distribution<> d(weights.begin(),weights.end());
	for(int i = 0; i < num_particles; i++){
		newParticles.push_back(particles[d(gen)]);
	}
	particles = newParticles;
}

Particle ParticleFilter::SetAssociations(Particle& particle, const std::vector<int>& associations, 
                                     const std::vector<double>& sense_x, const std::vector<double>& sense_y)
{
    //particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
    // associations: The landmark id that goes along with each listed association
    // sense_x: the associations x mapping already converted to world coordinates
    // sense_y: the associations y mapping already converted to world coordinates

    particle.associations= associations;
    particle.sense_x = sense_x;
    particle.sense_y = sense_y;
}

string ParticleFilter::getAssociations(Particle best)
{
	vector<int> v = best.associations;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
	vector<double> v = best.sense_x;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
	vector<double> v = best.sense_y;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
