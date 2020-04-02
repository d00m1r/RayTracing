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
        std::vector<Object*> mesh;//указатели на треугольники Model

        Object(){};
        Object(const glm::vec3& col, const Material &material): mat(material){
            mat.color = col;
            center = glm::vec3 (0.f,0.f,0.f);};
        Object(const glm::vec3& col, const Material &material, const glm::vec3& cen): mat(material), center(cen){mat.color = col;};

        virtual ~Object(){};
        virtual void calc_hit(const Ray& ray, float* sol_arr, int& el){};
        virtual const char* say_type(){return type;};
        virtual void set_normal(){};
};


class Sphere : public Object{
    public:
        float radius;
        Sphere(){};
        //Sphere(const glm::vec3& cen, const glm::vec3& col, float rad, const Material &material): Object(col, material, center), radius(rad){};
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

        virtual void calc_hit(const Ray& ray, float* sol_arr, int& el){
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

/*
class Plane:public Object{
    public:
        std::vector<glm::vec> pl;
        Plane(const glm::vec3& col, const Material &material, const std::vector<glm::vec3>& point_list): Object(col, material), pl(point_list){
            type = "plane";
            std::vector<glm::vec3> triang_verts1{pl[0], pl[1], pl[2]};
            std::vector<glm::vec3> triang_verts2{pl[0], pl[2], pl[3]};
        
            mesh.push_back(new Triangle(col, mat, triang_verts1));
            mesh.push_back(new Triangle(col, mat, triang_verts2));
        };
        
        virtual void calc_hit(const Ray& ray, float* solution, int& el){
            float cl_sol = 99999.9f;
            for(int i = 0; i < (int)mesh.size(); ++i){
                float sol_arr[2];
                mesh[i]->calc_hit(ray, sol_arr, el);
                if (sol_arr[0] > 0.0f && sol_arr[0] < 99999.9f && sol_arr[0] < cl_sol){
                    cl_sol = sol_arr[0];
                    el = i;
                }
                if (sol_arr[1] > 0.0f && sol_arr[1] < 99999.9f && sol_arr[1] < cl_sol){
                    cl_sol = sol_arr[1];
                    el = i;
                }
            }
            solution[0] = solution[1] = cl_sol;
        }
        
        ~Plane(){for(int i = 0; i < (int)mesh.size(); ++i) delete mesh[i];};
};
*/

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

        virtual void calc_hit(const Ray& ray, float* sol_arr, int& el){// Möller-Trumbore

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

class Model: public Object{
    public:
        Model(const glm::vec3& col, const Material &material, const char *filename):Object(col, mat){
            type = "model";
            std::ifstream in;
            in.open (filename, std::ifstream::in);
            if (in.fail()) {
                std::cerr << "Failed to open " << filename << std::endl;
                return;
            }
            std::string line;
            
            std::vector<glm::vec3> vert_arr;
            while (!in.eof()) {
                std::getline(in, line);
                std::istringstream iss(line.c_str());
                char trash;
                int count = 3;
                if (!line.compare(0, 2, "t2 ")) {
                    iss >> trash;
                    glm::vec3 vert;
                    for (int i=0;i<3;i++) iss >> vert[i];
                    vert_arr.push_back(vert);
                    count--;
                    if (!count){
                        count = 3;
                        mesh.push_back(new Triangle(col, mat, vert_arr));
                        vert_arr.clear(); 
                    }
                }
            } 
        };

        ~Model(){for(int i = 0; i < (int)mesh.size(); ++i) delete mesh[i];};

        virtual void calc_hit(const Ray& ray, float* solution, int& el){
            float cl_sol = 99999.9f;
            for(int i = 0; i < (int)mesh.size(); ++i){
                float sol_arr[2];
                mesh[i]->calc_hit(ray, sol_arr, el);
                if (sol_arr[0] > 0.0f && sol_arr[0] < 99999.9f && sol_arr[0] < cl_sol){
                    cl_sol = sol_arr[0];
                    el = i;
                }
                if (sol_arr[1] > 0.0f && sol_arr[1] < 99999.9f && sol_arr[1] < cl_sol){
                    cl_sol = sol_arr[1];
                    el = i;
                }
            }
            solution[0] = solution[1] = cl_sol;
        }
};


Object* determine_closest_object(const Ray& ray, const std::vector<Object*>& obj_list, float min_t, float max_t, float& cl_sol){
    Object* cl_obj;
    char* cl_type = "none";
    int el_num = -1;
    for(long unsigned int i = 0; i < obj_list.size(); i++){
        float sol_arr[2];
        obj_list[i]->calc_hit(ray, sol_arr, el_num);
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
    if (el_num != -1 && (cl_type == "model" || cl_type == "plane")) return (Object*)cl_obj->mesh[el_num];
    return cl_obj;
}

#endif