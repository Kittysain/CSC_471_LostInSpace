//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"
#include <GLFW/glfw3.h>


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle(vec3 start) :
	id(0),
	charge(1.0f),
	m(1.0f),
	d(0.0f),
	x(start),
	v(0.0f, 0.0f, 0.0f),
	lifespan(1.0f),
	tEnd(0.0f),
	scale(1.0f),
	color(randFloat(0.50f, 0.94f), randFloat(0.60f, 1.0f), randFloat(0.85f, 1.0f), 1.0f)
{
}

Particle::~Particle()
{
}

void Particle::load(vec3 start, int i)
{
	// Random initialization
	rebirth(0.0f, start);
	id = i;
}

/* all particles born at the origin */
void Particle::rebirth(float t, vec3 start)
{
	charge = randFloat(0.0f, 1.0f) < 0.5 ? -1.0f : 1.0f;	
	m = 1.0f;
  	d = randFloat(0.0f, 0.02f);
	x = start;
	v.x = randFloat(-2.95f, 2.95f);
	v.y = randFloat(0.0f, 1.0f);
	v.z = randFloat(-2.95f, 2.95f);
	lifespan = randFloat(0.2f, 1.5f); 
	//lifespan = 100.0f;
	tEnd = t + lifespan;
	scale = randFloat(0.2, 1.0f);
   	color.r = randFloat(0.50f, 0.94f);
   	color.g = randFloat(0.60f, 1.0f);
   	color.b = randFloat(0.85f, 1.0f);
   	colorScale.r = randFloat(0.7f, 0.9f);
   	colorScale.g = randFloat(0.2f, 0.7f);
   	colorScale.b = randFloat(0.2f, 0.7f);
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3 &g, const vec3 start)
{
	if(t > tEnd) {
		rebirth(t, start);
	}
	else
	{
	float time = glfwGetTime();
	vec3 acc = vec3(randFloat(-0.35f, 0.35f),randFloat(-0.35f, 0.35f),randFloat(-0.35f, 0.35f));
	vec3 g = vec3(0,-9.8f,0);
	//very simple update
	// x += h*v;
	// v += acc*h;
	// acc -= acc/time*(float)5.0;

	x += h*v;
	v += g*h;


   	color.r = colorScale.r*(cos(time)/4+0.75)*1.2;
   	color.g = colorScale.g*(cos(time+20)/4+0.75)*1.2;
   	color.b = colorScale.b*(cos(time+40)/4+0.75)*1.2;

	//To do - how do you want to update the forces?
	color.a = (tEnd-t)/lifespan;
	}	
}

int Particle::getId()
{
	return id;
}

