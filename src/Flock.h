#pragma once

#ifndef FLOCK_H_
#define FLOCK_H_

#include <vector>
#include <memory>
#include "Boid.h"

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class MatrixStack;
class Program;
class Texture;

class Flock
{
public:
    Flock();
    ~Flock();

    vector<shared_ptr<Boid>> boids;
    int boidNum = 100;
    int getFlockSize();
    void runFlock();
    void checkFlockId();
};

#endif // FLOCK