#include <iostream>
#include <algorithm>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Flock.h"
#include "Texture.h"
#include "WindowManager.h"
#include "particleSys.h"
#include "stb_image.h"
#include <GLFW/glfw3.h>

# define M_PI           3.14159265358979323846  /* pi */
#define TINYOBJLOADER_IMPLEMENTATION
#include <D:\A_Fall_2020\CSC_471\FinalProject\src\tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, texProg, cubeProg, partProg;

	// Shape to be used (from  file) - modify to support multiple
	shared_ptr<Shape> mesh;
	vector<std::shared_ptr<Shape>> meshList;
	vector<vec3> meshListMid;
	vector<vec3> meshListMin;
	vector<vec3> meshListMix;

	// Texture file 
	vector<std::shared_ptr<Texture>> textureList;
	shared_ptr<Texture> texture;

	//skybox texture
	unsigned int cubeMapTexture;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//example data that might be useful when trying to compute bounds on multi-shape
	vec3 gMin, gMax;

	//animation data
	float time, bodyRotate, flowerScale, viewTrans;
	float lTrans = 0;

	// x-key trigger
	int animate = -1;

	// m-key material switch
	int materialToggle = 0;

	//mouse tracker
	int firstMouse = 1;
	int mousePressed = 0;
	float lastX, lastY, xoffset, yoffset, phi, theta, radius, x, y, z;

	//camera walk around
	vec3 gaze, eye = vec3(-20, 0, 0), lookAtPoint = vec3(0,0,0), upVector, cameraRight, cameraUp;
	float cameraSpeed = 0.08;
	int goForward = 0, goBackward = 0, goLeft = 0, goRight = 0, goUp = 0, goDown = 0;

	//girl movement
	float moveLeft, moveRight, moveForw, moveBack, moveUp, moveDown;
	float girlX, girlY, girlZ, girlSpeed = 0.1, girlDir;

	//particle system
	particleSys *thePartSystem;
	vec3 particlePos;

	Flock *flocking = new Flock();

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}

		//move around the camera
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
        	goForward = 1;
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
        	goForward = 0;
    	if (key == GLFW_KEY_S && action == GLFW_PRESS)
			goBackward = 1;
    	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
			goBackward = 0;
    	if (key == GLFW_KEY_A && action == GLFW_PRESS)
			goLeft = 1;
    	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
			goLeft = 0;
    	if (key == GLFW_KEY_D && action == GLFW_PRESS)
			goRight = 1;
    	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
			goRight = 0;
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
			goUp = 1;
    	if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
			goUp = 0;
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
			goDown = 1;
    	if (key == GLFW_KEY_E && action == GLFW_RELEASE)
			goDown = 0;

		//move around the girl
		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
			moveForw = 1;
		if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
			moveForw = 0;
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
			moveBack = 1;
		if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
			moveBack = 0;
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
			moveRight = 1;
		if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
			moveRight = 0;
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
			moveLeft = 1;
		if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
			moveLeft = 0;
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
			moveUp = 1;
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
			moveUp = 0;

		//reset the particle system
		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			thePartSystem->reSet();
		}

	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 mousePressed = 1;
			 cout << "Pos X " << posX <<  " Pos Y " << posY << endl;
		}
		if (action == GLFW_RELEASE)
		{
			mousePressed = 0;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}
	
	//code to set up the two shaders - a diffuse shader and texture mapping
	void init(const std::string& resourceDirectory)
	{
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		// Set background color.
		CHECKED_GL_CALL(glClearColor(.12f, .34f, .56f, 1.0f));

		// Enable z-buffer test.
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(50.0f));

		// Initialize the GLSL program.
		partProg = make_shared<Program>();
		partProg->setVerbose(true);
		partProg->setShaderNames(
			resourceDirectory + "/lab10_vert.glsl",
			resourceDirectory + "/lab10_frag.glsl");
		if (! partProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		partProg->addUniform("P");
		partProg->addUniform("M");
		partProg->addUniform("V");
		partProg->addAttribute("pColor");
		partProg->addUniform("alphaTexture");
		partProg->addAttribute("vertPos");

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("shine");
		prog->addUniform("dL1position");
		prog->addUniform("pL1position");
		prog->addUniform("dL1color");
		prog->addUniform("pL1color");
		prog->addUniform("pL1constant");
		prog->addUniform("pL1linear");
		prog->addUniform("pL1quadratic");

		// Add textProg which process with the texture shader
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");
		texProg->addUniform("shine");
		texProg->addUniform("dL1position");
		texProg->addUniform("pL1position");
		texProg->addUniform("dL1color");
		texProg->addUniform("pL1color");
		texProg->addUniform("pL1constant");
		texProg->addUniform("pL1linear");
		texProg->addUniform("pL1quadratic");
		texProg->addUniform("Texture0");
		texProg->addUniform("flip");

		// Add cubeProg which process the skybox
		cubeProg = make_shared<Program>();
		cubeProg->setVerbose(true);
		cubeProg->setShaderNames(resourceDirectory + "/cube_vert.glsl", resourceDirectory + "/cube_frag.glsl");
		cubeProg->init();
		cubeProg->addUniform("P");
		cubeProg->addUniform("V");
		cubeProg->addUniform("M");
		cubeProg->addAttribute("vertPos");
		cubeProg->addAttribute("vertNor");
		cubeProg->addAttribute("vertTex");
		cubeProg->addUniform("skybox");

		thePartSystem = new particleSys(vec3(girlX, girlY-0.8, girlZ));
		thePartSystem->gpuSetup();
	}

	void initGeom(const std::string& resourceDirectory){

		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr, fName;
		vector<string> fNameList = {"/flower.obj", "/471Asset1.obj", 
		"/471Asset2.obj", "/cloud.obj", "/cube.obj", "/sphere.obj"};

		for (int i = 0; i < fNameList.size(); i++)
		{
			fName = fNameList[i];
			loadObjects(TOshapes, objMaterials, errStr, resourceDirectory, fName);
		}

	}

	void initTexture(const std::string& resourceDirectory)
	{
		string fName;
		vector<string> fNameList = {"/flower.jpg", "/moon.jpg", 
		"/flowerAlpha.jpg", "/nepture.jpg", "/water.jpg", "/gold.jpg","/pink_stars.jpg"};
		for (int i = 0; i < fNameList.size(); i++)
		{
			fName = fNameList[i];
			loadTexture(resourceDirectory, fName);
		}
	}

	void loadTexture(const std::string& resourceDirectory, const std::string& name)
	{
		texture = make_shared<Texture>();
		texture->setFilename(resourceDirectory + name);
		texture->init();
		texture->setUnit(1);
		texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		textureList.push_back(texture);
	}

	void loadObjects(vector<tinyobj::shape_t> TOshapes, vector<tinyobj::material_t> objMaterials, string errStr, const std::string& resourceDirectory, const std::string& name)
	{
		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + name).c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			for(int i = 0; i < TOshapes.size(); i++)
			{
				mesh = make_shared<Shape>();
				mesh->createShape(TOshapes[i]);
				mesh->measure();
				mesh->init();
				meshList.push_back(mesh);
				minMaxMiddle(mesh);
			}
		}
	}

	void minMaxMiddle(shared_ptr<Shape> mesh)
	{
		vec3 Min, Max, Mid;
		Min.x = mesh->min.x;
		Min.y = mesh->min.y;
		Min.z = mesh->min.z;
		meshListMin.push_back(Min);
		Max.x = mesh->max.x;
		Max.y = mesh->max.y;
		Max.z = mesh->max.z;
		meshListMin.push_back(Max);
		Mid.x = (Min.x + Max.x )/2;
		Mid.y = (Min.y + Max.y )/2;
		Mid.z = (Min.z + Max.z )/2;
		meshListMid.push_back(Mid);

		if(Min.x<gMin.x)
		gMin.x = Min.x;
		if(Min.y<gMin.y)
		gMin.y = Min.y;
		if(Min.z<gMin.z)
		gMin.z = Min.z;
		if(Max.x>gMax.x)
		gMax.x = Max.x;
		if(Max.y>gMax.y)
		gMax.y = Max.y;
		if(Max.z>gMax.z)
		gMax.z = Max.z;
	}

	void SetMaterial(int i) {
		switch (i) {
		case 0: //Gold
		glUniform3f(prog->getUniform("MatAmb"), 0.24725f, 0.1995f, 0.0745f);
		glUniform3f(prog->getUniform("MatDif"), 0.75164f, 0.60648f, 0.22648f);
		glUniform3f(prog->getUniform("MatSpec"), 0.628281f, 0.555802f, 0.366065f);
		glUniform1f(prog->getUniform("shine"), 52.1);
		break;
		case 1: //Perl
		glUniform3f(prog->getUniform("MatAmb"), 0.25f, 0.20725f, 0.20725f);
		glUniform3f(prog->getUniform("MatDif"), 1.0f, 0.829f, 0.829f);
		glUniform3f(prog->getUniform("MatSpec"),0.296648f, 0.296648f, 0.296648f);
		glUniform1f(prog->getUniform("shine"), 11.264);
		break;
		case 2: //Emerald
		glUniform3f(prog->getUniform("MatAmb"), 0.0215f, 0.1745f, 0.0215f);
		glUniform3f(prog->getUniform("MatDif"), 0.07568f, 0.61424f, 0.07568f);
		glUniform3f(prog->getUniform("MatSpec"), 0.633f, 0.727811f, 0.633f);
		glUniform1f(prog->getUniform("shine"), 76.8);
		break;
		}
	}

	void setLight(std::shared_ptr<Program> program){
		vec3 dL1position = vec3(0,15,-18);
		vec3 dL1color = vec3(1,0.5,0.7);

		vec3 pL1position = vec3(-15,20,-5);
		vec3 pL1color = vec3(0.3,0.678,1);
		float pL1constant = 0.3;
		float pL1linear = 0.1;
		float pL1quadratic = 0.1;

		glUniform3fv(program->getUniform("dL1position"), 1, value_ptr(dL1position));	
		glUniform3fv(program->getUniform("dL1color"), 1, value_ptr(dL1color));
		
		glUniform3fv(program->getUniform("pL1position"), 1, value_ptr(pL1position));	
		glUniform3fv(program->getUniform("pL1color"), 1, value_ptr(pL1color));

		glUniform1f(program->getUniform("pL1constant"), pL1constant);
		glUniform1f(program->getUniform("pL1linear"), pL1linear);
		glUniform1f(program->getUniform("pL1quadratic"), pL1quadratic);
	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   }

	unsigned int createSky(string dir, vector<string> faces) {
		
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for(GLuint i = 0; i < faces.size(); i++) {
			unsigned char *data = stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
			if (data) { 
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			} else {
				cout << "failed to load: " << (dir+faces[i]).c_str() << endl;
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		cout << " creating cube map any errors : " << glGetError() << endl;
		return textureID;
	} 

	void getCameraLookAt(GLFWwindow* window, int width, int height)
	{
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		if (mousePressed)
		{
			if (firstMouse)
			{
				lastX = xPos;
				lastY = yPos;
				firstMouse = 0;
			}
			xoffset = xPos - lastX;
			yoffset = lastY - yPos;
			lastX = xPos;
			lastY = yPos;

			// float sensitivity = 0.1f;
			// xoffset *= sensitivity;
			// yoffset *= sensitivity;

			theta += (M_PI/2) * (xoffset / width);
			phi += (M_PI/2) * (yoffset / width);

			if (phi > M_PI/2.25)
				phi = M_PI/2.25;
			if (phi < -M_PI/2.25)
				phi = -M_PI/2.25;
		}
		else
		{
			lastX = xPos;
			lastY = yPos;
		}
		radius = 10;
		x = radius*cos(phi)*cos(theta);
		y = radius*sin(phi);
		z = radius*cos(phi)*cos((M_PI/2.0)-theta);
	}

	void render(GLFWwindow *window, unsigned int cubeMapTexture)
	{
// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
	
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();
		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, 200.0f);

		// create Matrix stacks for the skyBox
		auto ProjectionSky = make_shared<MatrixStack>();
		auto ViewSky = make_shared<MatrixStack>();
		auto ModelSky = make_shared<MatrixStack>();
		//create identity for the skybox
		ModelSky->pushMatrix();
			ModelSky->loadIdentity();
			ModelSky->scale(vec3(100,100,100));
		
		// Pit and Yaw
		getCameraLookAt(window, width, height);
		lookAtPoint = glm::normalize(vec3(x,y,z));
		upVector = vec3(0,1,0);
		//gaze = -glm::normalize(eye - lookAtPoint);
		cameraRight = glm::normalize(glm::cross(lookAtPoint, upVector));
		cameraUp = glm::normalize(glm::cross(lookAtPoint, cameraRight));

		//walkAround
		walkAround();

		glm::mat4 View = glm::lookAt(eye, eye+lookAtPoint, upVector);

		//MeshList[0] = BackFlower
		//MeshList[1] = HeadFlower
		//MeshList[2] = BodyFlower
		//MeshList[3] = Vines
		//MeshList[4] = Body
		//MeshList[5] = Asset1
		//MeshList[6] = Asset2
		//MeshList[7] = cloud
		//MeshList[8] = cube
		//MeshList[9] = sphere

		girlMove();
		// draw mesh without texture maps 
		prog->bind();
		setLight(prog);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		Model->pushMatrix();
			Model->loadIdentity();
			Model->rotate(-M_PI/2, vec3(0,1,0));
			drawBodyFlowerVein(Model);
			drawCloud(Model);
		Model->popMatrix();
		prog->unbind();	

		// draw mesh with texture maps 
		texProg->bind();
		setLight(texProg);
		glUniform1f(texProg->getUniform("flip"),1);	
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View));
		Model->pushMatrix();
			Model->loadIdentity();
			Model->rotate(-M_PI/2, vec3(0,1,0));
			glUniform1f(texProg->getUniform("shine"),10);	
			drawBody(Model);
			glUniform1f(texProg->getUniform("shine"),1);	
			drawMoon(Model);
			drawGold(Model);
			drawNepture(Model);
		Model->popMatrix();

		drawFlock(Model);
		texProg->unbind();

		//rander skybox
		//to draw the sky box bind the right shader
		skybox(ModelSky, Projection, View);

		//draw the particles
		thePartSystem->setCamera(View);
		thePartSystem->startUpdate(vec3(girlX*cos(M_PI/2) - girlZ*sin(M_PI/2), girlY-0.8, girlX*sin(M_PI/2) + girlZ*cos(M_PI/2)));
		partProg->bind();
		textureList[2]->bind(partProg->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(View)));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix())));
		thePartSystem->drawMe(partProg);
		thePartSystem->update();
		partProg->unbind();

	}

    void skybox(shared_ptr<MatrixStack> Model, shared_ptr<MatrixStack> Projection, mat4 View)
	{
		cubeProg->bind();
		//set the projection matrix - can use the same one
		glUniformMatrix4fv(cubeProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		//set the depth function to always draw the box!
		glDepthFunc(GL_LEQUAL);
		//set up view matrix to include your view transforms 
		//(your code likely will be different depending
		glUniformMatrix4fv(cubeProg->getUniform("V"), 1, GL_FALSE,value_ptr(View));
		//set and send model transforms - likely want a bigger cube
		glUniformMatrix4fv(cubeProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
		//bind the cube map texture
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		//draw the actual cube
		meshList[8]->draw(cubeProg);
		//set the depth test back to normal!
		glDepthFunc(GL_LESS);
		//unbind the shader for the skybox
		cubeProg->unbind(); 
	}

	void drawFlock(shared_ptr<MatrixStack> Model)
	{
		flocking->runFlock();
		vector<shared_ptr<Boid>> boids = flocking->boids;
		for (auto boid: boids)
		{
			vec3 position = boid->getPosition();
			vec3 velocity = boid->getVelocity();
			Model->pushMatrix();
				Model->loadIdentity();
				Model->translate(position);
				Model->rotate(-2*glfwGetTime(),vec3(0,1,0));
				Model->scale(vec3(0.6,0.6,0.6));
				setModel(texProg, Model);
				// SetMaterial(3);
				textureList[6]->bind(texProg->getUniform("Texture0"));
				meshList[5]->draw(texProg);
			Model->popMatrix();
		}
	}

	void drawBody(shared_ptr<MatrixStack> Model)
	{
		//Draw Body
		Model->pushMatrix();

			Model->translate(vec3(girlX, girlY, girlZ));
	
			Model->translate(vec3(0,-2.5,0));
			Model->scale(vec3(0.25,0.25,0.25));
			Model->rotate(girlDir, vec3(0,1,0));
			setModel(texProg, Model);
			textureList[0]->bind(texProg->getUniform("Texture0"));
			meshList[4]->draw(texProg);
		Model->popMatrix();
		//Done with body
	}

	void drawBodyFlowerVein(shared_ptr<MatrixStack> Model)
	{
		//Draw Body
		Model->pushMatrix();

			Model->translate(vec3(girlX, girlY, girlZ));
			Model->translate(vec3(0,-2.5,0));
			Model->scale(vec3(0.25,0.25,0.25));
			Model->rotate(girlDir, vec3(0,1,0));
			//Draw Body Flowers
			Model->pushMatrix();
				setModel(prog, Model);
				SetMaterial(0);
				meshList[2]->draw(prog);
				meshList[1]->draw(prog);
				meshList[0]->draw(prog);
			Model->popMatrix();
			//Done with Body Flowers

			//Draw the vines
			Model->pushMatrix();
				setModel(prog, Model);
				SetMaterial(2);
				meshList[3]->draw(prog);
			Model->popMatrix();
			//Done with vines

		Model->popMatrix();
		//Done with body
	}

	void drawCloud(shared_ptr<MatrixStack> Model)
	{
		//Draw a cloud
		Model->pushMatrix();
			Model->translate(vec3(0,0,-15));
			Model->scale(vec3(0.0015,0.0015,0.0015));
			Model->translate(vec3(-meshListMid[7].x, -meshListMid[7].y, -meshListMid[7].z));
			setModel(prog, Model);
			SetMaterial(1);
			meshList[7]->draw(prog);
		Model->popMatrix();
		//Done with cloud
	}

	void drawMoon(shared_ptr<MatrixStack> Model)
	{
		Model->pushMatrix();
			Model->translate(vec3(7,5,-25));
			Model->rotate(glfwGetTime()/20, vec3(0,1,0));
			Model->scale(vec3(10,10,10));
			setModel(texProg, Model);
			textureList[1]->bind(texProg->getUniform("Texture0"));
			meshList[9]->draw(texProg);
		Model->popMatrix();
	}

	void drawNepture(shared_ptr<MatrixStack> Model)
	{
		Model->pushMatrix();
			Model->rotate(2.8 * M_PI/4, vec3(1,0,0));
			Model->rotate(-M_PI/4, vec3(0,0,1));
			Model->rotate(glfwGetTime()/2, vec3(0,1,0));
			Model->translate(vec3(-20,-10,18));
			Model->rotate(glfwGetTime()/2, vec3(0,1,0));
			Model->scale(vec3(2.5,2.5,2.5));
			setModel(texProg, Model);
			textureList[3]->bind(texProg->getUniform("Texture0"));
			meshList[9]->draw(texProg);
			drawWater(Model);
		Model->popMatrix();
	}

	void drawWater(shared_ptr<MatrixStack> Model)
	{
		Model->pushMatrix();

			Model->translate(vec3(-5,-2,3));
			Model->rotate(glfwGetTime()/2, vec3(0,1,0));
			Model->scale(vec3(1,1,1));
			setModel(texProg, Model);
			textureList[4]->bind(texProg->getUniform("Texture0"));
			meshList[9]->draw(texProg);
		Model->popMatrix();
	}
	
	void drawGold(shared_ptr<MatrixStack> Model)
	{
		Model->pushMatrix();
			Model->rotate(M_PI/6, vec3(1,0,1));
			Model->rotate(glfwGetTime()/1.5, vec3(0,1,0));
			Model->translate(vec3(-15,-7,20));
			Model->rotate(glfwGetTime()/2, vec3(0,1,0));
			Model->scale(vec3(1.8,1.8,1.8));
			setModel(texProg, Model);
			textureList[5]->bind(texProg->getUniform("Texture0"));
			meshList[9]->draw(texProg);
		Model->popMatrix();
	}

	void girlMove()
	{
		if(moveRight)
			{girlX += girlSpeed; 
			girlDir = M_PI/2;}
		if(moveLeft)
			{girlX -= girlSpeed; 
			girlDir = -M_PI/2;}
		if(moveForw)
			{girlZ -= girlSpeed; 
			girlDir = M_PI;}
		if(moveBack)
			{girlZ += girlSpeed;
			girlDir = 0;}
		if(moveUp)
			girlY += girlSpeed;
		if(!moveUp && girlY >= 0)
			girlY -= girlSpeed;

		if(moveRight && moveForw)
		{ 
			girlDir = 3*(M_PI/4);
		}
		if(moveRight && moveBack)
		{ 
			girlDir = M_PI/4;
		}
		if(moveLeft && moveForw)
		{ 
			girlDir = 5*M_PI/4;
		}
		if(moveLeft && moveBack)
		{ 
			girlDir = - M_PI/4;
		}


		if (girlX > 20)
			girlX = 20;
		if (girlX < -20)
			girlX = -20;
		if (girlY > 20)
			girlY = 20;
		if (girlY < -20)
			girlY = -20;
		if (girlZ > 20)
			girlZ = 20;
		if (girlZ < -20)
			girlZ = -20;
	}

	void walkAround()
	{
		if (goForward)
			eye += cameraSpeed * lookAtPoint;
		if (goBackward)
			eye -= cameraSpeed * lookAtPoint;
		if (goLeft)
			eye -= cameraRight * cameraSpeed;
		if (goRight)
			eye += cameraRight * cameraSpeed;

		if (eye.x > 20)
			eye.x = 20;
		if (eye.x < -20)
			eye.x = -20;
		if (eye.y > 20)
			eye.y = 20;
		if (eye.y < -20)
			eye.y = -20;
		if (eye.z > 20)
			eye.z = 20;
		if (eye.z < -20)
			eye.z = -20;
	}
};

int main(int argc, char **argv)
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	//names of the different side of the cube map
	vector<std::string> spaces 
	{
		"/right.png",
		"/left.png",
		"/top.png",
		"/bot.png",
		"/front.png",
		"/back.png"
	};

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(2560, 1920);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initTexture(resourceDir);
	unsigned int cubeMapTexture = application->createSky(resourceDir, spaces);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render(windowManager->getHandle(), cubeMapTexture);

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
