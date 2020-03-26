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

class Light{
    public:
        Light(const glm::vec3&position, const glm::vec3& col, float intensity): color(col), pos(position), inten(intensity){};
        ~Light(){};
        glm::vec3 color;
        glm::vec3 pos;
        float inten;
};

float light_intensity(const Ray& ray, const std::vector<Sphere>& sphere_list,
 const std::vector<Light>& light_list, const glm::vec3& p, const glm::vec3& n, Sphere obj, float max_t){
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
            if(cl_sol < max_t){
                shadow_flag = true;
                break;
            }
        }

        if (shadow_flag == true){// and obj.mat.reflect < 0.8){
            shadow_flag = false;
            continue;
        }

        //Диффузия
        if(obj.mat.reflect <= 0.8){
            float nl = dot(n, l);
            if (nl > 0){
                inten += light_list[i].inten * nl/(glm::length(n)*glm::length(l));
            }
        }

        //Зеркальность
        if (obj.mat.shine > 0) {
            glm::vec3 r = 2.0f*n*dot(n, l) - l;
            float rv = dot(r, -ray.dir);
            if (rv > 0){
                inten += light_list[i].inten*pow(rv/(glm::length(r) * glm::length(-ray.dir)), obj.mat.shine);
            }
        }
    }
    return inten; 
}

glm::vec3 refract(const glm::vec3 &I, const glm::vec3& n, const float eta_t, const float eta_i = 1.f) { 
    float cosi = - std::max(-1.f, std::min(1.f, dot(I, n)));
    if (cosi<0){ 
        return refract(I, -n, eta_i, eta_t);
    }
    float eta = eta_i / eta_t;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k<0 ? glm::vec3(1.f,0.f,0.f) : I * eta + n * (eta*cosi - sqrtf(k));
}

glm::vec3 reflect(const glm::vec3&dir, glm::vec3 n){
    return dot(n, dir)*2.f*n - dir;
}//dir - ...

glm::vec3 beam_shot(const Ray& ray, const std::vector<Sphere>& sphere_list, const std::vector<Light>& light_list, Window win, const glm::vec3& O, int depth, BMP& img){
    
    Sphere cl_obj;
    float cl_sol = determine_closest_object(ray, sphere_list, win.min_t, win.max_t, cl_obj);
    if (cl_sol < win.max_t){
        glm::vec3 p = O + cl_sol*ray.dir;//вычисление пересечения
        glm::vec3 n = p - cl_obj.center;//вычисление нормали сферы в точке пересечения
        n = n / glm::length(n);

        float diff = 0.f, shine = 0.f;
        bool shadow_flag = false;

        for(long unsigned int i = 0; i < light_list.size(); i++){
            glm::vec3 l = light_list[i].pos - p;
            Ray light(p, l);
            glm::normalize(l);

            //Тени
            Sphere sh_obj;
            for(long unsigned int i = 0; i < sphere_list.size(); i++){
                float sh_sol = determine_closest_object(light, sphere_list, 0.001, win.max_t, sh_obj);
                if(sh_sol < win.max_t){
                    shadow_flag = true;
                    break;
                }
            }
            if (shadow_flag == true){// and obj.mat.reflect < 0.8){
                shadow_flag = false;
                continue;
            }

            //Диффузия
            if(cl_obj.mat.features[0] > 0){
                float nl = dot(n, l);
                if (nl > 0){
                    diff += light_list[i].inten * nl/(glm::length(n)*glm::length(l));
                }
            }

            //Сияние
            if (cl_obj.mat.features[3] > 0) {
                glm::vec3 r = 2.0f*n*dot(n, l) - l;
                float rv = dot(r, -ray.dir);
                if (rv > 0){
                    shine += light_list[i].inten*pow(rv/(glm::length(r) * glm::length(-ray.dir)), cl_obj.mat.shine);
                }
            }
        }

        //float light_in =light_intensity(ray, sphere_list, light_list, p, n, cl_obj, win.max_t);
        //glm::vec3 local_color =  cl_obj.mat.color * light_in;
        
        if(depth < 1){
            return cl_obj.mat.color;
        }

        //Отражение
        glm::vec3 reflect_color (0.f,0.f,0.f);
        if (cl_obj.mat.features[1]){
            glm::vec3 reflect_dir = glm::normalize(reflect(-ray.dir, n));
            Ray reflect_ray(p, reflect_dir);
            reflect_color = beam_shot(reflect_ray, sphere_list, light_list, win, p, depth - 1, img);
        }

        //Преломление
        glm::vec3 refract_color (0.f,0.f,0.f);
        if (cl_obj.mat.features[2] > 0){   
            glm::vec3 refract_dir = glm::normalize(refract(ray.dir, n, cl_obj.mat.refract));
            Ray refract_ray(p, refract_dir);
            refract_color = beam_shot(refract_ray, sphere_list, light_list, win, p, depth - 1, img);
        }

        return /*(1.f - cl_obj.mat.reflect) */cl_obj.mat.color * cl_obj.mat.features[0] * diff 
        + reflect_color 
        + refract_color 
        + glm::vec3(0.5f,0.5f,0.5f) * shine;
    }
    return glm::vec3(0.8f, 0.85f, 1.f); // BCKG col
    //double pi = 3.141592653;// попали в небо  
    //float phi = atan2f(ray.dir.x, ray.dir.y);
    //phi /= (2 * pi);
    //float tetta = (std::asin(ray.dir.z) + pi/2)/pi;
    //return look_image(img, tetta, phi);
}

#endif