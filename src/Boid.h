#pragma once

#ifndef BOID_H_
#define BOID_H_

#include <vector>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;
using namespace std;

class Boid
{
public:
    //constructor 
    Boid(int i);
    ~Boid();

    void load();
    void run(vector<shared_ptr<Boid>> boids);
    void update();
    void applyForce(vec3 force);
    void flock(vector<shared_ptr<Boid>> boids);
    float boidDistance(vec3 self, vec3 other);
    void edge();
    void bounce();

    vec3 align(vector<shared_ptr<Boid>> boids);
    vec3 cohesion(vector<shared_ptr<Boid>> boids);
    vec3 seperation(vector<shared_ptr<Boid>> boids);
    int getId();
    vec3 getPosition();
    vec3 getVelocity();



    
private:
    float randFloat(float l, float h);
    vec3 randVec3(float min, float max);
    vec3 divideByFloat(vec3 vector, float f);
    vec3 multByFloat(vec3 vector, float f);
    vec3 limit(vec3 vector, float f);

    //boid properties
    int id;
    vec3 position;
    vec3 velocity;
    vec3 acceleration;
    float speed;
    float maxSpeed;
    float maxForce;

};

#endif //BOID