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
#include <glm/trigonometric.hpp>

#include "window.hpp"
#include "ray.hpp"
#include "object.hpp"
#include "material.hpp"
#include "BMP/EasyBMP.h"

class Light{
    public:
        Light(const glm::vec3&position, const glm::vec3& col, float intensity): color(col), pos(position), inten(intensity){};
        ~Light(){};
        glm::vec3 color;
        glm::vec3 pos;
        float inten;
};

glm::vec3 refract(const glm::vec3 &I, const glm::vec3& normal, const float eta_t, const float eta_i = 1.f) { 
    float cosi = - std::max(-1.f, std::min(1.f, dot(I, normal)));
    if (cosi<0) return refract(I, -normal, eta_i, eta_t);
    float eta = eta_i / eta_t;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k<0 ? glm::vec3(0.1f,0.1f,0.1f) : I * eta + normal * (eta*cosi - sqrtf(k));
}

glm::vec3 beam_shot(const Ray& ray, const std::vector<Object*>& obj_list, const std::vector<Light>& light_list, Window win, const glm::vec3& start_pos, int depth, BMP& img){
    
    float cl_sol = win.max_t;
    auto cl_obj = determine_closest_object(ray, obj_list, win.min_t, win.max_t, cl_sol);
    if (cl_sol < win.max_t){
        
        glm::vec3 point = start_pos + cl_sol * ray.dir;// вычисление пересечения
        glm::vec3 normal = cl_obj->norm;
        if (cl_obj->type == "sphere"){
            normal = point - cl_obj->center;// вычисление нормали сферы в точке пересечения
            normal = normal / glm::length(normal);
        }

        float diff = 0.f, shine = 0.f;
        bool shadow_flag = false;

        for(long unsigned int i = 0; i < light_list.size(); i++){
            glm::vec3 l = light_list[i].pos - point;
            Ray light(point, l);//  инициализация луча света
            //glm::normalize(l);
            l = l/ glm::length(l);
            // Тени
            float sh_sol = win.max_t;
            for(long unsigned int i = 0; i < obj_list.size(); i++){
                determine_closest_object(light, obj_list, 0.001, win.max_t, sh_sol);
                if(sh_sol < win.max_t){
                    shadow_flag = true;
                    break;
                }
            }
            if (shadow_flag == true and cl_obj->mat.reflect < 0.8){
                shadow_flag = false;
                continue;
            }

            // Диффузия
            if(cl_obj->mat.features[0] > 0){
                float nl = dot(normal, l);
                if (nl > 0){
                    diff += light_list[i].inten * nl/(glm::length(normal)*glm::length(l));
                }
            }

            // Сияние
            if (cl_obj->mat.features[3] > 0) {
                glm::vec3 r = glm::reflect(-l, normal);
                //if (dot(r, -ray.dir) > 0){
                    shine += light_list[i].inten*pow(dot(r, -ray.dir)/(glm::length(r) * glm::length(-ray.dir)), cl_obj->mat.shine);
                //}
            }
        }
        
        if(depth < 1) return cl_obj->mat.color;
            
        // Отражение
        glm::vec3 reflect_color (0.f,0.f,0.f);
        if (cl_obj->mat.features[1]){
            glm::vec3 reflect_dir = glm::normalize(glm::reflect(ray.dir, normal));
            Ray reflect_ray(point, reflect_dir);
            reflect_color = beam_shot(reflect_ray, obj_list, light_list, win, point, depth - 1, img);
        }

        // Преломление
        glm::vec3 refract_color (0.f,0.f,0.f);
        if (cl_obj->mat.features[2] > 0){   
            glm::vec3 refract_dir = glm::normalize(refract(ray.dir, normal, cl_obj->mat.refract));
            Ray refract_ray(point, refract_dir);
            refract_color = beam_shot(refract_ray, obj_list, light_list, win, point, depth - 1, img);
        }

        return /*(1.f - cl_obj->mat.reflect) */cl_obj->mat.color * cl_obj->mat.features[0] * diff 
        + reflect_color * cl_obj->mat.reflect
        + refract_color 
        + glm::vec3(1.f,1.f,1.f) * shine;
    }
    //return glm::vec3 (0.f,0.f,0.f);
    //return glm::vec3(0.8f, 0.85f, 1.f); // BCKG col

    float x = 0.5f + atan2f(ray.dir.z, ray.dir.x) / (2*win.pi);
    float y = 0.5f - glm::asin(ray.dir.y) / win.pi;
    return look_image(img, x, y);
}

#endif