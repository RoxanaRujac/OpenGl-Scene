//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "SkyBox.hpp"

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;


glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

//directional light
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

//point light
glm::vec3 lightPct = glm::vec3(10.30f, 2.71, -19.19f);;
bool pctLightOn = false;

gps::Camera myCamera(
				glm::vec3(5.0f, 24.0f, -99.5f), 
				glm::vec3(-4.0f, 9.0f, 1.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.3f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

//----------------------------- Objects --------------------------------------

gps::Model3D scena;
gps::Model3D apa;
gps::Model3D aur;
gps::Model3D drop;
gps::Model3D sticla1;
gps::Model3D sticla2;
gps::Model3D treasure;

gps::Model3D nanosuit;
gps::Model3D lightCube;
gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLuint shadowMapFBO;

GLuint depthMapTexture;

bool showDepthMap;

bool fogEnabled = false;
bool transparencyEnabled = false;
bool lightOn = true;

//tour
bool tour = false;
float tourTime = 0.0f;
float tourSpeed = 0.05f;
int currentSegment = 0;



//--------------------------------- OpenGL stuff ------------------------------------------------------------------

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);	
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}



//--------------------- Keyboard & mouse ----------------------------------------------------------------

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	// Handle escape key to close the window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Toggle depth map visibility on M key press
	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}


float lastX, lastY;
bool mouse = true;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	// Initialize lastX and lastY on the first mouse movement
	if (mouse) {
		lastX = xpos;
		lastY = ypos;
		mouse = false;
	}

	//if in tour mode the camera rotation movement is locked
	if (tour) {
		return;
	}

	// Calculate offsets for mouse movement
	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;

	// Update lastX and lastY for the next movement
	lastX = xpos;
	lastY = ypos;

	// Sensitivity factor for mouse movement
	const float sensitivity = 0.5f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	// Apply rotation to the camera based on the offsets
	myCamera.rotate(yOffset, xOffset);
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Poject", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}



//-------------------------- Movement & commands -----------------------------------------------------------


void processMovement()
{
	//rotate 
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;		
	}


	//move light
	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}


	//move camera
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}


	//fog
	if (pressedKeys[GLFW_KEY_F]) {
		fogEnabled = !fogEnabled;
	}

	//transparent
	if (pressedKeys[GLFW_KEY_T]) {
		transparencyEnabled = !transparencyEnabled;
	}

	//wireframe
	if (pressedKeys[GLFW_KEY_X]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}


	//polygon
	if (pressedKeys[GLFW_KEY_Y]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}


	//solid
	if (pressedKeys[GLFW_KEY_Z]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	//smooth
	if (pressedKeys[GLFW_KEY_C]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_SMOOTH);
	}


	//directional light on
	if (pressedKeys[GLFW_KEY_O]) {
		lightOn = !lightOn;
	}


	//punctiform light on
	if (pressedKeys[GLFW_KEY_P]) {
		pctLightOn = !pctLightOn;
	}


	//show coordinates in terminal
	if (pressedKeys[GLFW_KEY_B]) {
		printf("Camera position: x = %.2f, y = %.2f, z = %.2f\n", myCamera.getCameraPos().x, myCamera.getCameraPos().y, myCamera.getCameraPos().z);
	}


	//tour view
	if (pressedKeys[GLFW_KEY_U]) {
		tour != tour;
		if (tour = true){
			tourTime = 0.0f;
			currentSegment = 0;
			std::cout << "Automated tour started!" << std::endl;
		}
	}

}


//----------------------------------- Object & shaders initializations --------------------------------------

void initObjects() {

	scena.LoadModel("objects/scena/insula.obj");
	apa.LoadModel("objects/scena/apa2.obj");
	aur.LoadModel("objects/scena/aur.obj");
	drop.LoadModel("objects/scena/pic.obj");
	sticla1.LoadModel("objects/scena/sticla1.obj");
	sticla2.LoadModel("objects/scena/sticla2.obj");
	treasure.LoadModel("objects/scena/treasure.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
}


void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}



//------------------------------------ Uniform variables initialization ----------------------------------

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


	//-------- Compute directional light
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);			//location 
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	// White light
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));


	//-------- Compute punctiform light
	GLint pctLightOnLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pctLightOn");	//toggle on off
	glUniform1i(pctLightOnLoc, pctLightOn);

	GLint pctLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPct");		//location
	glUniform3fv(pctLightLoc, 1, glm::value_ptr(lightPct));

	// Yellow color for golden ingot
	glm::vec3 yellowColor = glm::vec3(1.0f, 1.0f, 0.0f); 
	GLint pctLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPctColor");
	glUniform3fv(pctLightColorLoc, 1, glm::value_ptr(yellowColor));


	// Light shader
	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


}



//-------------------------------------------------- Skybox ---------------------------------------------

void initSkyBox() {
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	faces.push_back("skybox/top.tga");
	faces.push_back("skybox/bottom.tga");
	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");
	mySkyBox.Load(faces);
}


//--------------------------------------------------- Shadows -------------------------------------------

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 3.0f, far_plane = 15.0f;
	glm::mat4 lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}



//----------------------------------------------- Rain Effect ------------------------------------------------------

// Struct that represents each raindrop with its position and speed
struct rain {
	glm::vec3 position;
	glm::vec3 speed;
};

// All raindrops
std::vector<rain> rainDrops;

void rainInit() {
	// Initialize 5000 raindrops with random positions and fixed downward speed
	for (int i = 0; i < 5000; i++) {
		rain raindrop;

		raindrop.position = glm::vec3((rand() % 100) - 50, rand() % 100, (rand() % 100) - 50);
		raindrop.speed = glm::vec3(0.0f, -1.0f, 0.0f);
		rainDrops.push_back(raindrop);
	}
}

void rainUpdate() {

	for (auto& picatura : rainDrops) {
		picatura.position += picatura.speed;

		// Check if the raindrop has fallen below the reset threshold -> reinitialize
		if (picatura.position.y < -10.0f) {
			picatura.position = glm::vec3((rand() % 100) - 50, rand() % 100, (rand() % 100) - 50);
		}
	}
}




//--------------------------------------------- Tour ------------------------------------------------------------

std::vector<glm::vec3> positions = {
	glm::vec3(-29.28f, 22.52f, -61.14f), 
	glm::vec3(-46.95f, 13.05f, -39.37f), 
	glm::vec3(-43.70f, 10.32f, -19.98f), 
	glm::vec3(-40.72f, 9.23f, -13.91f),  
	glm::vec3(-22.66f, 6.52f, 4.54f),    
	glm::vec3(-11.35f, 5.61f, 10.65f),   
	glm::vec3(-3.26f, 4.67f, 8.57f),     
	glm::vec3(5.07f, 4.49f, 3.38f),      
	glm::vec3(20.07f, 1.50f, -13.85f),   
	glm::vec3(18.65f, 2.27f, -16.95f),   
	glm::vec3(7.41f, 2.98f, -15.29f),   
	glm::vec3(-1.03f, 2.98f, -27.75f),   
	glm::vec3(4.11f, 6.21f, -37.03f),    
	glm::vec3(28.04f, 4.00f, -21.16f),   
	glm::vec3(9.14f, 9.67f, -42.93f),    
	glm::vec3(10.83f, 18.57f, -66.66f)   
};


void updateAutomatedTour() {
	if (!tour || currentSegment >= positions.size() - 1) {
		return;
	}

	glm::vec3 start = positions[currentSegment];
	glm::vec3 end = positions[currentSegment + 1];

	float t = tourTime;
	glm::vec3 interpolatedPos = glm::mix(start, end, t);

	myCamera.setCameraPos(interpolatedPos);

	// Increment time
	tourTime += tourSpeed;

	// Move to the next segment if t exceeds 1
	if (tourTime >= 1.0f) {
		tourTime = 0.0f;
		currentSegment++;
	}

	// Last segment is reached -> stop
	if (currentSegment >= positions.size() - 1) {
		tour = false;
	}
}



//-------------------------------------------- Draw & Render ----------------------------------------------------------


void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	// Draw island
	scena.Draw(shader);

	//Draw water transparent

	if (transparencyEnabled) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	apa.Draw(shader);

	if (transparencyEnabled) {
		glDisable(GL_BLEND);
	}

	// Draw gold ingot

	aur.Draw(shader);

	// Draw bottle1 transparent

	if (transparencyEnabled) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	sticla1.Draw(shader);

	if (transparencyEnabled) {
		glDisable(GL_BLEND);
	}


	// Draw bottle2 transparent
	
	if (transparencyEnabled) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	sticla2.Draw(shader);

	if (transparencyEnabled) {
		glDisable(GL_BLEND);
	}


	// Draw chest
	
	treasure.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

}



void renderScene() {

	// First render pass - render depth map
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, showDepthMap);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	if (showDepthMap) {

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Fog enable, directional and punctiform light enable + position
		myCustomShader.useShaderProgram();
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "enableFog"), fogEnabled);

		GLint lightOnLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightOn");
		glUniform1i(lightOnLoc, lightOn);

		GLint pctLightOnLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pctLightOn");
		glUniform1i(pctLightOnLoc, pctLightOn);


		glm::vec4 lightPosEye = view * glm::vec4(lightPct, 1.0f);
		GLint pctLightLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPct");
		glUniform3fv(pctLightLoc, 1, glm::value_ptr(glm::vec3(lightPosEye)));


		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		// Generate rain
		for (auto& picatura : rainDrops) {
			glm::mat4 matrix = glm::mat4(1.0f);
			matrix = glm::translate(matrix, picatura.position);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(matrix));

			drop.Draw(myCustomShader);
		}


		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// Draw skybox
		mySkyBox.Draw(skyboxShader, view, projection);

	}

}



void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}


int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkyBox();
	rainInit();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {

		if (tour) {
			updateAutomatedTour();
		}
		else {
			processMovement();
		}
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
		rainUpdate();
	}

	cleanup();

	return 0;
}
