/******************************************************************************/
/* Name: interface.cpp                                                        */
/* Purpose: Creates an interface and publishes a position based on user input.*/
/* Author: Matthew Lynch                                                      */
/******************************************************************************/

// Include Libraries
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sstream>

// Struct for the Object's movement and location
struct ObjectData
{
	int xPosition;
	int yPosition;
	int xVelocity;
	int yVelocity;
	int xDelta;
	int yDelta;
	int xTarget;
	int yTarget;
};

// Function Prototypes
bool initializeSDL();
void closeSDL();
bool eventLoop(ros::Publisher t, ObjectData &obj);
bool updateMovement(ObjectData &obj);
bool renderObjects(ObjectData obj);

// Global Variables
static int WINDOW_WIDTH = 640;
static int WINDOW_HEIGHT = 480;
static int OBJECT_WIDTH = 64;
static int OBJECT_HEIGHT = 64;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
SDL_Texture* object;
SDL_Texture* background;

// Main
int main (int argc, char** argv)
{
	// ROS things
	ros::init(argc, argv, "interface");
	ros::NodeHandle node;
	ros::Publisher topic;
	topic = node.advertise<std_msgs::String>("position", 1000);
	ros::Rate loop_rate(10); //10 Hz

	ObjectData robot;
	robot.xPosition = WINDOW_WIDTH / 2;
	robot.yPosition = WINDOW_HEIGHT / 2;

	if (!initializeSDL())
	{
		return 1;
	}

	// Main Loop
	while (ros::ok())
	{
		if (!eventLoop(topic, robot))
		{
			break;
		}
		else if (!updateMovement(robot))
		{
			break;
		}
		else if (!renderObjects(robot))
		{
			break;
		}
		else
		{
			ros::spinOnce();
			loop_rate.sleep();
		}
	}
	closeSDL();
	return 0;
}

bool initializeSDL()
{
	if (SDL_INIT_EVERYTHING < 0)
	{
		std::cout << "Error initializing SDL2: " << SDL_GetError() << std::endl;
		std::cout << "Press enter to continue...";
		std::cin.get();
		return false;
	}
	if (IMG_Init(IMG_INIT_PNG) < 0 )
	{
		std::cout << "Error initializing SDL_image: " << SDL_GetError() << std::endl;
		std::cout << "Press enter to continue...";
		std::cin.get();
		return false;
	}
	window = SDL_CreateWindow(
		"Window",					// Name of the window
		SDL_WINDOWPOS_CENTERED,		// X Position of the window
		SDL_WINDOWPOS_CENTERED,		// Y Position of the window
		WINDOW_WIDTH,				// Width of the window
		WINDOW_HEIGHT,				// Height of the window
		0							// Flags for the window (e.g. SDL_WINDOW_RESIZABLE)
	);
	if (!window)
	{
		std::cout << "Error creating window: " << SDL_GetError() << std::endl;
		std::cout << "Press enter to continue...";
		std::cin.get();
		return false;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
	{
		std::cout << "Error creating renderer: " << SDL_GetError() << std::endl;
		std::cout << "Press enter to continue...";
		std::cin.get();
		return false;
	}
	object = SDL_CreateTextureFromSurface(renderer, IMG_Load("../../../src/position_publisher/images/object.png"));
	if (!object)
	{
		std::cout << "Error loading \"object.png\": " << SDL_GetError() << std::endl;
		std::cout << "Press enter to continue...";
		std::cin.get();
		return false;
	}
	background = SDL_CreateTextureFromSurface(renderer, IMG_Load("../../../src/position_publisher/images/background.png"));
	if (!background)
	{
		std::cout << "Error loading \"background.png\": " << SDL_GetError() << std::endl;
		std::cout << "Press enter to continue...";
		std::cin.get();
		return false;
	}
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	return true;
}

void closeSDL()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
	SDL_DestroyTexture(object);
	SDL_DestroyTexture(background);

	window = NULL;
	renderer = NULL;
	texture = NULL;
	object = NULL;
	background = NULL;

	SDL_Quit();
	IMG_Quit();
}

bool eventLoop(ros::Publisher t, ObjectData &obj)
{
	std_msgs::String message;
	std::stringstream ss;

	int mouseX, mouseY;

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
			case SDL_MOUSEBUTTONUP:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						// Sets a target for the object to move to
						obj.xTarget = event.button.x;
						obj.yTarget = event.button.y;

						// Update variables based on the target.
						obj.xDelta = obj.xTarget - obj.xPosition;
						obj.yDelta = obj.yTarget - obj.yPosition;
						obj.xVelocity = obj.xDelta / 10; // Probably find a different way to calcuate the velocity.
						obj.yVelocity = obj.yDelta / 10;
						
						// Publishes the target location.
						std::cout << "Target Location: ( " << obj.xTarget << " , " << obj.yTarget << " )" << std::endl;
						ss << "Target Location: ( " << obj.xTarget << " , " << obj.yTarget << " )";
						message.data = ss.str();
						t.publish(message);
						break;
				}
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_h:
						std::cout << "Halting movement." << std::endl;
						obj.xVelocity = 0;
						obj.yVelocity = 0;
						obj.xDelta = 0;
						obj.yDelta = 0;
						break;

					case SDLK_o:
						std::cout << "Object Location: ( " << obj.xPosition << " , " << obj.yPosition << " )" << std::endl;
						break;

					case SDLK_p:
						SDL_GetMouseState( &mouseX, &mouseY);
						std::cout << "Mouse Location: ( " << mouseX << " , " << mouseY << " )" << std::endl;
						break;

					case SDLK_ESCAPE:
						std::cout << "Exiting program." << std::endl;
						return false;
						break;
				}
				break;

			case SDL_QUIT:
				return false;
				break;
		}
	}
	return true;
}

bool updateMovement(ObjectData &obj)
{
	// Checks if the object position is within a certain range of the target
	if (obj.xPosition < obj.xTarget - 10 || obj.xPosition > obj.xTarget + 10)
	{
		obj.xPosition += obj.xVelocity;
	}
	if (obj.yPosition < obj.yTarget - 10 || obj.yPosition > obj.yTarget + 10)
	{
		obj.yPosition += obj.yVelocity;
	}

	// Restrains the object to the screen
	if (obj.xPosition > WINDOW_WIDTH)
	{
		obj.xPosition = WINDOW_WIDTH;
	}
	if (obj.xPosition < 0)
	{
		obj.xPosition = 0;
	}
	if (obj.yPosition > WINDOW_HEIGHT)
	{
		obj.yPosition = WINDOW_HEIGHT;
	}
	if (obj.yPosition < 0)
	{
		obj.yPosition = 0;
	}

	return true;
}

bool renderObjects(ObjectData obj)
{
	SDL_Rect destination;

	// Render Background
	destination.x = 0;
	destination.y = 0;
	destination.w = WINDOW_WIDTH;
	destination.h = WINDOW_HEIGHT;

	texture = background;

	if (!texture)
	{
		std::cout << "Error creating texture: " << SDL_GetError() << std::endl;
		std::cout << "Press enter to continue...";
		std::cin.get();
		return false;
	}

	SDL_RenderCopy(renderer, texture, NULL, &destination);
	texture = NULL;

	// Render Object
	destination.x = obj.xPosition - (OBJECT_WIDTH / 2);
	destination.y = obj.yPosition - (OBJECT_HEIGHT / 2);
	destination.w = OBJECT_WIDTH;
	destination.h = OBJECT_HEIGHT;

	texture = object;

	if (!texture)
	{
		std::cout << "Error creating texture: " << SDL_GetError() << std::endl;
		std::cout << "Press enter to continue...";
		std::cin.get();
		return false;
	}

	SDL_RenderCopy(renderer, texture, NULL, &destination);
	texture = NULL;

	SDL_RenderPresent(renderer);

	return true;
}
