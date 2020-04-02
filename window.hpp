#ifndef RAY_TRACING_WINDOW
#define RAY_TRACING_WINDOW

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include "BMP/EasyBMP.h"

struct Window{
    const int width = 1280;
    const int height = 720;
    const float fov = glm::pi<float>()/3;
	float min_t = 0.001;
    float max_t = 99999999.0f;
    int depth = 6;
	float inf = 25.f;
	float pi = 3.1415926f;

};

void save_image(Window win, std::vector<glm::vec3> buffer, char* name){
	BMP image;
	image.SetSize(win.width, win.height);
	int displacement = 0;
	for (int j = 0; j < win.height; j++){
		for (int i = 0; i < win.width; i++){
			glm::vec3 color = buffer[displacement + i];
			image(i,j)->Red  =glm::clamp(color.x, 0.0f, 1.0f) * 255.0f;
			image(i,j)->Green=glm::clamp(color.y, 0.0f, 1.0f) * 255.0f;
			image(i,j)->Blue =glm::clamp(color.z, 0.0f, 1.0f) * 255.0f;
		}
		displacement += win.width;
	}
	image.WriteToFile(name);
}

glm::vec3 look_image(BMP& img, float x, float y) {
	int Wi = img.TellWidth();
	int He = img.TellHeight();
	x *= Wi*7;
	y *= He*7;
	if(x > Wi) x=(int)x%Wi;
	if(y > He) y=(int)y%He;
	unsigned char r = img(x, y)->Red;
	unsigned char g = img(x, y)->Green;
	unsigned char b = img(x, y)->Blue;
	float r1 = (float)r / 255;
	float g1 = (float)g / 255;
	float b1 = (float)b / 255;
	return glm::vec3(r1, g1, b1);
}

#endif