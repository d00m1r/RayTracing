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

class Sphere{
    public:
        Sphere(){}
        Sphere(const glm::vec3& cen, const glm::vec3& col, float rad, int sh): center(cen), color(col), radius(rad), shine(sh){};

        glm::vec3 center;
        glm::vec3 color;
        float radius;
        int shine;

        Sphere& operator=(const Sphere& right) {
            //проверка на самоприсваивание
            if (this == &right) {
                return *this;
            }
            radius = right.radius;
            shine  = right.shine;
            center = right.center;
            color  = right.color;
            return *this;
        }

        void calc_hit(const Ray& ray, float* sol_arr)const{

            glm::vec3 oc = ray.start - center;
            float a = dot(ray.dir, ray.dir);//ОПТИМИЗИРОВАТЬ
            float b = dot(oc, ray.dir)*2;
            float c = dot(oc, oc) - radius*radius;
            float D = b*b - 4*a*c;

            if (D > 0){
                sol_arr[0] = (-b + sqrt(D))/(2*a);
                sol_arr[1] = (-b - sqrt(D))/(2*a);
                return;
            }
            sol_arr[0] = sol_arr[1] = 0.f;
        }
};

float determine_closest_object(const Ray& ray, const std::vector<Sphere>& sphere_list, float min_t, float max_t, Sphere& cl_obj){
    float cl_sol = max_t;
    for(long unsigned int i = 0; i < sphere_list.size(); i++){
        float sol_arr[2];
        sphere_list[i].calc_hit(ray, sol_arr);
        if (sol_arr[0] > min_t && sol_arr[0] < max_t && sol_arr[0] < cl_sol){
            cl_sol = sol_arr[0];
            cl_obj = sphere_list[i];
        }
        if (sol_arr[1] > min_t && sol_arr[1] < max_t && sol_arr[1] < cl_sol){
            cl_sol = sol_arr[1];
            cl_obj = sphere_list[i];
        }
    }
    return cl_sol;

}

float light_intensity(const Ray& ray, const std::vector<Sphere>& sphere_list,
 const std::vector<Light>& light_list, const glm::vec3& p, const glm::vec3& n, int shine, float max_t){
    float inten = 0.1;
    bool shadow_flag = false;

    for(long unsigned int i = 0; i < light_list.size(); i++){

        glm::vec3 l = light_list[i].pos - p;
        Ray light(p, l);
        glm::normalize(l);

        //Тени
        Sphere cl_obj;
        for(long unsigned int i = 0; i < sphere_list.size(); i++){
            float cl_sol = determine_closest_object(light, sphere_list, 0.001, max_t, cl_obj);
            if(cl_sol != max_t){
                shadow_flag = true;
                break;
            }
        }

        if (shadow_flag == true){
            shadow_flag = false;
            //light.~Ray();
            continue;
        }

        //Диффузия
        float nl = dot(n, l);
        if (nl > 0){
            inten += light_list[i].inten * nl/(glm::length(n)*glm::length(l));
        }

        //Зеркальность
        if (shine != -1) {
            glm::vec3 r = 2.0f*n*dot(n, l) - l;
            float rv = dot(r, -ray.dir);
            if (rv > 0){
                inten += light_list[i].inten*pow(rv/(glm::length(r) * glm::length(-ray.dir)), shine);
            }
        }
        //light.~Ray();
    }
    return inten; 
}


glm::vec3 beam_shot(const Ray& ray, const std::vector<Sphere>& sphere_list, const std::vector<Light>& light_list, float min_t, float max_t, const glm::vec3& O){
    
    Sphere cl_obj;
    float cl_sol = determine_closest_object(ray, sphere_list, min_t, max_t, cl_obj);
    if (cl_sol != max_t){
        glm::vec3 p = O + cl_sol*ray.dir;//вычисление пересечения
        glm::vec3 n = p - cl_obj.center;//вычисление нормали сферы в точке пересечения
        //glm::normalize(n);
        n = n / glm::length(n);
        return cl_obj.color * light_intensity(ray, sphere_list, light_list, p, n, cl_obj.shine, max_t);
    }
    return glm::vec3(0.1f, 0.1f, 0.1f); // BCKG col
}

void make_render(){
    Window win;
    std::vector<glm::vec3> buffer(win.width * win.height);
    glm::vec3 O(0.0f, 0.0f, 7.0f);//камера
    int displacement = win.width;

    std::vector<Sphere> sphere_list{
        Sphere(glm::vec3(0.f, 0.f,0.8f),  glm::vec3(1.f,0.0f,0.0f), 0.5f, 90),//R
        Sphere(glm::vec3(1.f, 0.2f,1.2f), glm::vec3(0.0f,0.0f,1.f), 0.3f, 15),//B
        Sphere(glm::vec3(0.f, 0.f,0.0f),   glm::vec3(1.f,1.f,0.f), 1.f, 10),
        Sphere(glm::vec3(0.f, -0.9f,0.0f), glm::vec3(0.0f,1.f,0.0f), 0.5f, 50),//G
    };

    std::vector<Light> light_list{
        Light(glm::vec3(14.7f,-6.0f,7.0f),   glm::vec3(1.0f,1.0f,1.0f), 0.3f),
        Light(glm::vec3(17.1f,-10.0f,32.0f), glm::vec3(1.0f,1.0f,1.0f), 0.4f)
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