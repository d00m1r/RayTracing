#ifndef RAY_TRACING_EFFECTS
#define RAY_TRACING_EFFECTS

#include <vector>
#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/transform.hpp>

#include "window.hpp"
#include "ray.hpp"
#include "object.hpp"

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

glm::vec3 beam_shot(const Ray& ray, const std::vector<Sphere>& sphere_list, const std::vector<Light>& light_list, float min_t, float max_t, const glm::vec3& O, int depth){
    
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

#endif