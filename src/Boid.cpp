#include <iostream>
#include "Boid.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include <GLFW/glfw3.h>
#include <cmath>

using namespace std;
using namespace glm;

Boid::Boid(int i)
{
	id = i;
}

Boid::~Boid()
{
    //nothing here
}

void Boid::load()
{
    position = randVec3(-15,15);
	velocity = randVec3(-0.005,0.005);
	acceleration = vec3(0,0,0);
    maxForce = 0.08;
    maxSpeed = 0.05;
}

void Boid::run(vector<shared_ptr<Boid>> boids)
{
    flock(boids);
    update();
    edge();
}

void Boid::update()
{

    velocity += acceleration;

    velocity = limit(velocity, maxSpeed);

    position += velocity;
    acceleration = vec3(0,0,0);
}

void Boid::applyForce(vec3 force)
{
    acceleration += force;
}

void Boid::flock(vector<shared_ptr<Boid>> boids)
{
    vec3 ali = align(boids);
    vec3 coh = cohesion(boids);
    vec3 sep = seperation(boids);

    ali = multByFloat(ali, 0.1);
    coh = multByFloat(coh, 0.04);
    sep = multByFloat(sep, 0.75);

    applyForce(ali);   
    applyForce(coh);   
    applyForce(sep);   
}

float Boid::boidDistance(vec3 self, vec3 other)
{
    float distance = 0;
    for (int i = 0;i<3; i++)
    {
        distance += pow(self[i] - other[i], 2);
    }
    distance = sqrt(distance);
    return distance;
}

void Boid::edge()
{  
    float Xbound = 15;
    float Ybound = 2;
    float Zbound = 15;
    if (position.x >= Xbound)
    {
        position.x = -Xbound;
    }
    if (position.x <= -Xbound)
    {
        position.x = Xbound;
    }
    if (position.y >= -2)
    {
        position.y = -20;
    }
    if (position.y <= -20)
    {
        position.y = -2;
    }    
    if (position.z >= Zbound)
    {
        position.z = -Zbound;
    }
    if (position.z <= -Zbound)
    {
        position.z = Zbound;
    }
}

void Boid::bounce()
{
    // velocity.x = -1*(velocity.x + randFloat(-0.25, 0.25));
    // velocity.y = -1*(velocity.y + randFloat(-0.25, 0.25));
    // velocity.z = -1*(velocity.z + randFloat(-0.25, 0.25));
    velocity.x = velocity.x * -1;
    velocity.y = velocity.y * -1;
    velocity.z = velocity.z * -1;
}

vec3 Boid::align(vector<shared_ptr<Boid>> boids)
{
    float perception = 1.6;
    vec3 steering = vec3(0,0,0);
    int total = 0;

    for(auto boid: boids)
    {
        float dist = boidDistance(position, boid->getPosition());
        if (id != boid->getId() && dist < perception)
        {
            steering += boid->getVelocity();
            total++;
        }
    }

    if (total > 0)
    {
        steering = divideByFloat(steering, total);
        // steering = normalize(steering);
        steering = steering - velocity;
        steering = limit(steering, maxForce);
    }
    return steering;
}

vec3 Boid::cohesion(vector<shared_ptr<Boid>> boids)
{
    float perception = 2;
    vec3 steering = vec3(0,0,0);
    int total = 0;

    for(auto boid: boids)
    {
        float dist = boidDistance(position, boid->getPosition());
        if (id != boid->getId() && dist < perception)
        {
            steering += boid->getPosition();
            total++;
        }
    }

    if (total > 0)
    {
        steering = divideByFloat(steering, total);
        // steering = normalize(steering);
        steering = steering - position;
        steering = limit(steering, maxForce);
    }
    return steering;
}

vec3 Boid::seperation(vector<shared_ptr<Boid>> boids)
{
    float perception = 1.25;
    vec3 steering = vec3(0,0,0);
    int total = 0;

    for(auto boid: boids)
    {
        float dist = boidDistance(position, boid->getPosition());
        if (id != boid->getId() && dist < perception)
        {
            vec3 diff = position - boid->getPosition();
            diff = divideByFloat(diff, dist);
            steering += diff;
            total++;
        }
    }

    if (total > 0)
    {
        steering = divideByFloat(steering, total);
        // steering = normalize(steering);
        steering = steering - velocity;
        steering = limit(steering, maxForce);
    }
    return steering;
}

int Boid::getId()
{
    return id;
}

vec3 Boid::getPosition()
{
    return position;
}

vec3 Boid::getVelocity()
{
    return velocity;
}

vec3 Boid::randVec3(float min, float max)
{
    return vec3(randFloat(min, max), randFloat(min, max), randFloat(min, max));
}

float Boid::randFloat(float l, float h)
{
    float r = l + (rand() / ((float)RAND_MAX / h-l));
    return r;
}

vec3 Boid::divideByFloat(vec3 vector, float f)
{
    return (vec3(vector.x/f, vector.y/f, vector.z/f));
}

vec3 Boid::multByFloat(vec3 vector, float f)
{
    return (vec3(vector.x*f, vector.y*f, vector.z*f));
}

vec3 Boid::limit(vec3 vector, float f)
{
    if (vector.x > f)
        vector.x = f;
    if (vector.y > f)
        vector.y = f;
    if (vector.z > f)
        vector.z = f;
    if (vector.x < -f)
        vector.x = -f;
    if (vector.y < -f)
        vector.y = -f;
    if (vector.z < -f)
        vector.z = -f;
    return vector;
}