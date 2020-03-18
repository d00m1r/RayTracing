#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>

#include "BMP/EasyBMP.h"
#include "BMP/EasyBMP.cpp"

struct Window{
    const int width = 1024;
    const int height = 768;
    const float fov = glm::pi<float>()/3;
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

class Ray{
    public:
        Ray(){}
        Ray(const glm::vec3& orig, const glm::vec3& direction): start(orig), dir(direction){};
        glm::vec3 start;
        glm::vec3 dir;
};

glm::vec3 ray_dir(int i, int j, Window win){
    glm::vec3 dir;
    dir.x =  (i + 0.5f) -  (win.width/2.0f);
    dir.y = (j + 0.5f) - (win.height/2.0f);
    dir.z = -(win.width)/tan(win.fov/2.0f);
    return glm::normalize(dir);
}

class Sphere{
    public:
        Sphere(){}
        Sphere(glm::vec3 cen, glm::vec3 col, float rad): center(cen), color(col), radius(rad){};
        glm::vec3 center;
        glm::vec3 color;
        float radius;

        bool check_hit(const Ray& ray, float min_t, float max_t){

            glm::vec3 oc = ray.start - center;
            float a = dot(ray.dir, ray.dir);
            float b = dot(oc, ray.dir)*2;
            float c = dot(oc, oc) - radius*radius;
            float D = b*b - 4*a*c;
            if (D > 0){
                float solution = (-b + sqrt(D))/(2*a);
                if (solution > min_t && solution < max_t){
                    return true;
                }
                solution = (-b - sqrt(D))/(2*a);
                if (solution > min_t && solution < max_t){
                    return true;
                }
            }
            return false;
        }
};


glm::vec3 beam_shot(const Ray& ray,  std::vector<Sphere>& sphere_list, float min_t, float max_t){
    for(long unsigned int i = 0; i < sphere_list.size(); i++){
        if (sphere_list[i].check_hit(ray, min_t, max_t)){
            return sphere_list[i].color;
        }
    }
    return glm::vec3(0.1f, 0.1f, 0.1f); // BCKG col
}

void make_render(){
    Window win;
    std::vector<glm::vec3> buffer(win.width * win.height);
    glm::vec3 O(0.0f, 0.0f, 0.0f);//камера
    int displacement = win.width;

    std::vector<Sphere> sphere_list{
        Sphere (glm::vec3(-1.0f,0.0f,-5.0f), glm::vec3(1.0f,0.0f,0.0f), 0.5f),
        Sphere (glm::vec3(0.0f,-1.0f,-7.0f), glm::vec3(0.0f,1.0f,0.0f), 0.7f),
        Sphere (glm::vec3(1.0f, 0.0f,-5.0f), glm::vec3(0.0f,0.0f,1.0f), 0.3f)
    };
    float min_t = 0.0f;
    float max_t = 9999.0f;

    for (int j = 0; j < win.height; j++){
        for (int i = 0; i < win.width; i++){
            Ray ray ( O, ray_dir(i, j, win));//уже нормализованный
            buffer[j * displacement + i] = beam_shot(ray, sphere_list, min_t, max_t);//ret col
        }
    }
    char name[] = "scene.bmp";
    save_image(win, buffer, name);    
}

int main(){
    make_render();
    return 0;
}