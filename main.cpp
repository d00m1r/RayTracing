#include <iostream>
#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>

#include "object.hpp"
#include "ray.hpp"
#include "window.hpp"
/*class camera {
    public:
        camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect) {
            // vfov is top to bottom in degrees
            vec3 u, v, w;
            float theta = vfov*M_PI/180;
            float half_height = tan(theta/2);
            float half_width = aspect * half_height;
            origin = lookfrom;
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);
            lower_left_corner = origin - half_width*u - half_height*v - w;
            horizontal = 2*half_width*u;
            vertical = 2*half_height*v;
        }

        ray get_ray(float s, float t) {
            return ray(origin,
                       lower_left_corner + s*horizontal + t*vertical - origin);
        }

};*/

class Light{
    public:
        Light(const glm::vec3&position, const glm::vec3& col, float intensity): color(col), pos(position), inten(intensity){};
        ~Light(){};
        glm::vec3 color;
        glm::vec3 pos;
        float inten;
};

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

glm::vec3 reflect(const glm::vec3&dir, glm::vec3 n){
    return dot(n, dir)*2.f*n - dir;
}

glm::vec3 beam_shot( const Ray& ray, const std::vector<Sphere>& sphere_list, const std::vector<Light>& light_list, float min_t, float max_t, const glm::vec3& O, int depth){
    
    Sphere cl_obj;
    float cl_sol = determine_closest_object(ray, sphere_list, min_t, max_t, cl_obj);
    if (cl_sol != max_t){
        glm::vec3 p = O + cl_sol*ray.dir;//вычисление пересечения
        glm::vec3 n = p - cl_obj.center;//вычисление нормали сферы в точке пересечения
        n = n / glm::length(n);
        float light_in =light_intensity(ray, sphere_list, light_list, p, n, cl_obj.shine, max_t);
        glm::vec3 local_color =  cl_obj.color * light_in;
        
        if(depth < 1 || cl_obj.reflect < 0){
            return local_color ;
        }
        //ray.dir = -ray.dir;
        glm::vec3 reflect_dir = reflect(-ray.dir, n);
        //glm::vec3 reflect_orig = dot(reflect_dir, n) < 0.f ? p - n * 0.001f : p + n * 0.001f;
        Ray ref_ray(p, reflect_dir);
        glm::vec3 reflected_color = beam_shot(ref_ray, sphere_list, light_list, 0.001, max_t, p, depth - 1);
        
        return local_color * (1.f - cl_obj.reflect) + reflected_color * cl_obj.reflect;
    }
    return glm::vec3(0.0f, 0.0f, 0.0f); // BCKG col
}

void make_render(){
    Window win;
    std::vector<glm::vec3> buffer(win.width * win.height);
    int displacement = win.width;

    std::vector<Sphere> sphere_list{
        Sphere(glm::vec3(-1.7f, 0.f,0.8f),  glm::vec3(1.0f, 0.0f, 0.0f), 0.5f, 50,  0.3f),//R
        Sphere(glm::vec3(0.0f, -0.9f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.5f, 150, 0.3f),//G
        Sphere(glm::vec3(0.8f, 0.2f,1.2f),  glm::vec3(0.0f, 0.0f, 1.0f), 0.3f, 100, 0.3f),//B
        Sphere(glm::vec3(0.0f, 0.f,0.0f),   glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 20, 0.2f),
    };

    std::vector<Light> light_list{
        Light(glm::vec3(14.7f,-12.0f,14.0f),   glm::vec3(1.0f,1.0f,1.0f), 0.5f),
        Light(glm::vec3(17.1f,-20.0f,64.0f), glm::vec3(1.0f,1.0f,1.0f), 0.5f)
    };

    float min_t = 0;
    float max_t = 999999.0f;
    int depth = 2;

    glm::vec3 O1(0.0f, 0.0f, 7.0f);//прямо по Z
    glm::vec3 O2(0.0f, 0.0f, -7.0f);//сзади
    glm::vec3 O3(0.0f, -7.0f, 0.0f);//сверху

    glm::mat3 rotz(-1.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 1.f);//Z 180
    glm::mat3 roty(-1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, -1.f);//Y 180
    glm::mat3 rotx(1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 1.f, 0.f);//X 90



    for (int j = 0; j < win.height; j++){
        for (int i = 0; i < win.width; i++){
            Ray ray ( O3, rotz*rotx * ray_dir(i, j, win));//уже нормализованный
            buffer[j * displacement + i] = beam_shot(ray, sphere_list, light_list, min_t, max_t, O3, depth);//ret col
        }
    }
    char name[] = "scene.bmp";
    save_image(win, buffer, name);    
}

int main(){
    make_render();
    return 0;
}