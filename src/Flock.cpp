#include <iostream>
#include "Flock.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;


Flock::Flock()
{
	for (int i = 0; i < boidNum; i++)
	{
		auto aBoid = make_shared<Boid>(i);
		boids.push_back(aBoid);
        aBoid->load();
	}
}

Flock::~Flock()
{
    //Nothing here
}

int Flock::getFlockSize()
{
    return boidNum;
}

void Flock::runFlock()
{
    for (auto boid: boids)
    {
        boid->run(boids);
    }
}

void Flock::checkFlockId()
{
    for (auto boid:boids)
    {
        int id = boid->getId();
        cout<<id;
    }
}