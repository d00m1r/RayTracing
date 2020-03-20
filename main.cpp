#include <iostream>
#include <cmath>
#include <vector>

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
        Ray(const glm::vec3& orig, const glm::vec3& direction): start(orig), dir(direction){};
        ~Ray(){};
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

class Light
{
    public:
        Light(const glm::vec3&position, const glm::vec3& col, float intensity): color(col), pos(position), inten(intensity){};
        ~Light(){};
        glm::vec3 color;
        glm::vec3 pos;
        float inten;
};

float light_intensity(const Ray& ray, const std::vector<Light>& light_list, const glm::vec3& p, const glm::vec3& n, int shine){
    float inten = 0;
    for(long unsigned int i = 0; i < light_list.size(); i++){
        glm::vec3 l = light_list[i].pos - p;
        glm::normalize(l);
        //Диффузия
        float nl = dot(n, l);
        if (nl > 0){
            inten += light_list[i].inten * nl/(glm::length(n)*glm::length(l));//std::max(0.0f,nl);
        }

        //Зеркальность
        if (shine != -1) {
            glm::vec3 r = l-dot(n, l)*2.0f*n;//СМЕНА ЗНАКА
            float rv = dot(r, -ray.dir);
            if (rv > 0){
                inten += light_list[i].inten*pow(rv/(glm::length(r) * glm::length(-ray.dir)), shine);
                //inten += light_list[i].inten * pow(glm::normalize(rv), shine);
            }
            //+= light_list[i].inten * pow(std::max(0.f, (ray.dir + n * 2.0f * ray.dir * n)*ray.dir), shine)
        }
    }
    return inten; 
}
class Sphere{
    public:
        Sphere(){}
        Sphere(const glm::vec3& cen, const glm::vec3& col, float rad, int sh): center(cen), color(col), radius(rad), shine(sh){};
        glm::vec3 center;
        glm::vec3 color;
        float radius;
        int shine;

        bool check_hit(const Ray& ray, float min_t, float max_t, float& sol)const{

            glm::vec3 oc = ray.start - center;
            float a = dot(ray.dir, ray.dir);//ОПТИМИЗИРОВАТЬ
            float b = dot(oc, ray.dir)*2;
            float c = dot(oc, oc) - radius*radius;
            float D = b*b - 4*a*c;
            if (D > 0){
                float solution = (-b + sqrt(D))/(2*a);
                if (solution > min_t && solution < max_t){
                    sol = solution;
                    return true;
                }
                solution = (-b - sqrt(D))/(2*a);
                if (solution > min_t && solution < max_t){
                    sol = solution;
                    return true;
                }
            }
            return false;
        }
};


glm::vec3 beam_shot(const Ray& ray, const std::vector<Sphere>& sphere_list, const std::vector<Light>& light_list, float min_t, float max_t, const glm::vec3& O){
    for(long unsigned int i = 0; i < sphere_list.size(); i++){
        float sol = 0;
        if (sphere_list[i].check_hit(ray, min_t, max_t, sol)){
            glm::vec3 p = O + sol*ray.dir;//вычисление пересечения
            glm::vec3 n = p - sphere_list[i].center;//вычисление нормали сферы в точке пересечения
            //glm::normalize(n);
            n = n / glm::length(n);
            return sphere_list[i].color * light_intensity(ray, light_list, p, n, sphere_list[i].shine);
        }
    }
    return glm::vec3(0.1f, 0.1f, 0.1f); // BCKG col
}

void make_render(){
    Window win;
    std::vector<glm::vec3> buffer(win.width * win.height);
    glm::vec3 O(0.0f, 0.0f, 3.0f);//камера
    int displacement = win.width;

    std::vector<Sphere> sphere_list{
        Sphere (glm::vec3(0.2f, 0.3f,-10.0f), glm::vec3(1.f,1.f,0.f), 1.2f, 1000),
        Sphere (glm::vec3(-0.7f, 0.3f,-5.0f), glm::vec3(1.f,0.0f,0.0f), 0.5f, 700),
        Sphere (glm::vec3(0.2f, -0.5f,-7.0f), glm::vec3(0.0f,1.f,0.0f), 0.7f, 500),
        Sphere (glm::vec3(0.9f, 0.4f, -5.0f), glm::vec3(0.0f,0.0f,1.f), 0.3f, 300),
    };

    std::vector<Light> light_list{
        Light(glm::vec3(1.7f,-12.0f,-14.0f), glm::vec3(1.0f,1.0f,1.0f), 0.5f),
        Light(glm::vec3(-9.1f,-13.0f,-18.0f), glm::vec3(1.0f,1.0f,1.0f), 0.5f)
    };

    float min_t = 0;
    float max_t = 999999.0f;

    for (int j = 0; j < win.height; j++){
        for (int i = 0; i < win.width; i++){
            Ray ray ( O, ray_dir(i, j, win));//уже нормализованный
            buffer[j * displacement + i] = beam_shot(ray, sphere_list, light_list, min_t, max_t, O);//ret col
        }
    }
    char name[] = "scene.bmp";
    save_image(win, buffer, name);    
}

int main(){
    make_render();
    return 0;
}