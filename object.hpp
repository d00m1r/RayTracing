#ifndef RAY_TRACING_OBJECT
#define RAY_TRACING_OBJECT

#include <iostream>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <cassert>
#include <fstream>
#include <sstream>

#include "ray.hpp"
#include "material.hpp"
#include "window.hpp"

class Object{
    public:
        Material mat;
        glm::vec3 center;//Sphere
        glm::vec3 norm;//Triangle
        char* type;
        
        Object(){};
        Object(const glm::vec3& col, const Material &material): mat(material){
            mat.color = col;
            center = glm::vec3 (0.f,0.f,0.f);};
        Object(const glm::vec3& col, const Material &material, const glm::vec3& cen): mat(material), center(cen){mat.color = col;};

        virtual ~Object(){};
        virtual void calc_hit(const Ray& ray, float* sol_arr){};
        virtual const char* say_type(){return type;};
        virtual void set_normal(){};
};


class Sphere : public Object{
    public:
        float radius;
        Sphere(){};
        Sphere(const glm::vec3& col, const Material &material, const glm::vec3& cen, float rad): Object(col, material, cen), radius(rad){
            type = "sphere";
        };

        Sphere& operator=(const Sphere& right) {
            //проверка на самоприсваивание
            if (this == &right) {
                return *this;
            }
            radius  = right.radius;
            mat = right.mat;
            center  = right.center;
            mat.color   = right.mat.color;
            return *this;
        }

        virtual void calc_hit(const Ray& ray, float* sol_arr){
            glm::vec3 oc = ray.start - center;
            float a = dot(ray.dir, ray.dir);
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

class Triangle:public Object{
    public:
        std::vector<glm::vec3> pl;
        glm::vec3 edge1;
        glm::vec3 edge2;

        Triangle(){};
        Triangle(const glm::vec3& col, const Material &material, const std::vector<glm::vec3>& point_list): Object(col, material), pl(point_list){
            set_normal();
            type = "triangle";
        };

        void set_normal(){
            edge2 = pl[1] - pl[0];
            edge1 = pl[2] - pl[0];
            norm = cross(edge1, edge2);
            norm = norm / glm::length(norm);
        }

        virtual void calc_hit(const Ray& ray, float* sol_arr){// Möller-Trumbore

            glm::vec3 T = ray.start - pl[0];
            glm::vec3 P = cross(ray.dir, edge2);
            glm::vec3 Q = cross(T, edge1);
            float k = dot(P, edge1);
            float ik = 1 / k;

            bool flag = true;
            sol_arr[0] = sol_arr[1] = 0.f;
            // Луч параллелен плоскости
            if (k < 1e-8 && k > -1e-8) {flag = false;}

            float u = ik * dot(P, T);
            if (u < 0 || u > 1) {flag = false;}

            float v = ik * dot(ray.dir, Q);
            if (v < 0 || u + v > 1) {flag = false;}

            if(flag){sol_arr[0] = sol_arr[1] = ik * dot(Q, edge2);}
            
        }
};


Object* determine_closest_object(const Ray& ray, const std::vector<Object*>& obj_list, float min_t, float max_t, float& cl_sol){
    Object* cl_obj;
    char* cl_type = "none";
    for(long unsigned int i = 0; i < obj_list.size(); i++){
        float sol_arr[2];
        obj_list[i]->calc_hit(ray, sol_arr);
        if (sol_arr[0] > min_t && sol_arr[0] < max_t && sol_arr[0] < cl_sol){
            cl_sol = sol_arr[0];
            cl_type = obj_list[i]->type;
            cl_obj = (Object*)obj_list[i];//!upcast
        }
        if (sol_arr[1] > min_t && sol_arr[1] < max_t && sol_arr[1] < cl_sol){
            cl_sol = sol_arr[1];
            cl_type = obj_list[i]->type;
            cl_obj = (Object*)obj_list[i];//!upcast
        }
    }
    return cl_obj;
}

#endif