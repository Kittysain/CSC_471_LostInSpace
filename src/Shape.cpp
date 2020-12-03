#include "Shape.h"
#include <iostream>
#include <assert.h>

#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;
void calcNor(float v[3][3], vec3 *out);


Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0), 
   vaoID(0)
{
	min = glm::vec3(0);
	max = glm::vec3(0);
}

Shape::~Shape()
{
}

/* copy the data from the shape to this object */
void Shape::createShape(tinyobj::shape_t & shape)
{
		posBuf = shape.mesh.positions;
		norBuf = shape.mesh.normals;
		texBuf = shape.mesh.texcoords;
		eleBuf = shape.mesh.indices;
}

void Shape::measure() {
  float minX, minY, minZ;
   float maxX, maxY, maxZ;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
		if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
   max.x = maxX;
   max.y = maxY;
   max.z = maxZ;
}

void Shape::normal()
{
	//allocate all zero for the norBuf;
	for (int i = 0; i<posBuf.size(); i++)
	{
		norBuf.push_back(0);
	}
	//read 3 vertex from eleBuf everytime, these 3 vertex makes a face
	for (int tri = 0; tri < eleBuf.size()/3; tri++)
	{
		vec3 nor = vec3(0,0,0);
		float triangle[3][3];
		//parse through the vertex of each face
		for (int vert = 0; vert < 3; vert++)
		{
			//parse through the xyz coordinate of each vertex
			for (int xyz = 0; xyz <3 ;xyz++)
			{
				triangle[vert][xyz] = posBuf[3*eleBuf[tri*3+vert]+xyz];
			}
		}

		calcNor(triangle,&nor);
		float norm[3];
		norm[0] = nor.x;
		norm[1] = nor.y;
		norm[2] = nor.z;

		//add the normal to every vertices 
		for (int vert = 0; vert<3; vert++)
		{
			for (int xyz = 0; xyz < 3; xyz++)
			{
				norBuf[3*eleBuf[tri*3+vert]+xyz] += norm[xyz];
			}
		}
	}

	//normalize every normal vector 
	for (int i = 0; i<norBuf.size()/3; i++)
	{
		float length = sqrt(pow(norBuf[3*i], 2) + pow(norBuf[3*i+1], 2) + pow(norBuf[3*i+2], 2));
		norBuf[3*i] = norBuf[3*i]/length;
		norBuf[3*i+1] = norBuf[3*i+1]/length;
		norBuf[3*i+2] = norBuf[3*i+2]/length;
	}

}


void Shape::init()
{
   // Initialize the vertex array object
   glGenVertexArrays(1, &vaoID);
   glBindVertexArray(vaoID);

	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(norBuf.empty()) {
		normal();
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	} else {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(texBuf.empty()) {
		texBufID = 0;
	} else {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

   glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if(h_nor != -1 && norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0) {	
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if(h_tex != -1 && texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}
	
	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	
	// Draw
	glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void calcNor(float v[3][3], vec3 *out)
{

	float v1[3] , v2[3];
	static const int x = 0;
	static const int y = 1;
	static const int z = 2;

	//calculate 2 vectors of the 3 points
	v1[x] = v[0][x] - v[1][x];
	v1[y] = v[0][y] - v[1][y];
	v1[z] = v[0][z] - v[1][z];

	v2[x] = v[1][x] - v[2][x];
	v2[y] = v[1][y] - v[2][y];
	v2[z] = v[1][z] - v[2][z]; 

	//calculat the cross productor of the 2 vectors
	out->x = v1[y]*v2[z] - v1[z]*v2[y];
	out->y = v1[z]*v2[x] - v1[x]*v2[z];
	out->z = v1[x]*v2[y] - v1[y]*v2[x]; 

	//normalize the result normal vector
	glm::normalize(*out);
}


