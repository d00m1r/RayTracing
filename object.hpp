#ifndef RAY_TRACING_OBJECT
#define RAY_TRACING_OBJECT

#include <iostream>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "ray.hpp"

class Sphere{
    public:
        Sphere(){};
        Sphere(const glm::vec3& cen, const glm::vec3& col, float rad, int sh, float reflective): center(cen), color(col), radius(rad), shine(sh), reflect(reflective){};

        glm::vec3 center;
        glm::vec3 color;
        float radius;
        int shine;
        float reflect;

        Sphere& operator=(const Sphere& right) {
            //проверка на самоприсваивание
            if (this == &right) {
                return *this;
            }
            radius  = right.radius;
            shine   = right.shine;
            reflect = right.reflect;
            center  = right.center;
            color   = right.color;
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

#endif