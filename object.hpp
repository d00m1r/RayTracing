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

class Object{
    public:
        Material mat;
        glm::vec3 center;
        glm::vec3 norm;
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

        virtual void calc_hit(const Ray& ray, float* sol_arr){
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



class Triangle:public Object{
    public:
        std::vector<glm::vec3> pl;
        glm::vec3 edge1;
        glm::vec3 edge2;

        Triangle(const glm::vec3& col, const Material &material, const std::vector<glm::vec3>& point_list): Object(col, material), pl(point_list){
            //set_normal();
            type = "triangle";
        };

        void set_normal(){
            edge1 = pl[1] - pl[0];
            edge2 = pl[2] - pl[0];
            norm = glm::cross(edge1, edge2);
            norm = norm / glm::length(norm);
        }

        virtual void calc_hit(const Ray& ray, float* sol_arr){// Möller-Trumbore
            /*edge1 = pl[1] - pl[0];
            edge2 = pl[2] - pl[0];
            glm::vec3 T = p - v[0];
            glm::vec3 P = glm::cross(D, edge2);
            glm::vec3 Q = glm::cross(T, edge1);
            glm::vec3 D = ray.dir;
            
            sol_arr[0] = sol_arr[1] = (1 - u - v)
            */
            edge1 = pl[1] - pl[0];
            edge2 = pl[2] - pl[0];
            // Вычисление вектора нормали к плоскости
            glm::vec3 pvec = cross(ray.dir, edge2);
            //norm = cross(edge1, edge2);
            norm = pvec;
            norm = norm / glm::length(norm);
            float det = dot(edge1, pvec);
            bool flag = true;
            sol_arr[0] = sol_arr[1] = 0.f;
            // Луч параллелен плоскости
            if (det < 1e-8 && det > -1e-8) {
                flag = false;
            }

            float inv_det = 1 / det;
            glm::vec3 tvec = ray.start - pl[0];
            float u = dot(tvec, pvec) * inv_det;
            if (u < 0 || u > 1) {
                flag = false;
            }

            glm::vec3 qvec = cross(tvec, edge1);
            float v = dot(ray.dir, qvec) * inv_det;
            if (v < 0 || u + v > 1) {
                flag = false;
            }
            if(flag){sol_arr[0] = sol_arr[1] = dot(edge2, qvec) * inv_det;}
            
        }
};

Object* determine_closest_object(const Ray& ray, const std::vector<Object*>& obj_list, float min_t, float max_t, float& cl_sol){
    Object* cl_obj;
    for(long unsigned int i = 0; i < obj_list.size(); i++){
        float sol_arr[2];
        obj_list[i]->calc_hit(ray, sol_arr);
        if (sol_arr[0] > min_t && sol_arr[0] < max_t && sol_arr[0] < cl_sol){
            cl_sol = sol_arr[0];
            cl_obj = (Object*)obj_list[i];//!upcast
        }
        if (sol_arr[1] > min_t && sol_arr[1] < max_t && sol_arr[1] < cl_sol){
            cl_sol = sol_arr[1];
            cl_obj = (Object*)obj_list[i];//!upcast
        }
    }
    return cl_obj;
}

/*
class Triangle:public Object{

    Triangle* TriangleCreate(Vec3 a,
                             Vec3 b,
                             Vec3 c,
                             Color color,
                             Color specularColor,
                             float phongExponent,
                             Color reflectiveColor)
    {
        Triangle* triangle = (Triangle*) malloc(sizeof(Triangle));

        triangle->a = a;
        triangle->b = b;
        triangle->c = c;
        triangle->color = color;
        triangle->specularColor = specularColor;
        triangle->phongExponent = phongExponent;
        triangle->reflectiveColor = reflectiveColor;

        return triangle;
    }

    Vec3 TriangleNormal(Triangle* triangle)
    {
        glm::vec3 ab = dot(triangle->b, triangle->a);
        glm::vec3 ac = dot(triangle->c, triangle->a);
        return glm::normalize(glm::cross(ab, ac));
    }

    RayHit* TriangleIntersect(void* surface, Ray* ray)
    {
        Triangle* tri = (Triangle*) surface;

        // Solve linear system using cramer's rule to find intersection between
        // ray and triangle defined in barycentric form.
        float a = tri->a.x - tri->b.x;
        float b = tri->a.y - tri->b.y;
        float c = tri->a.z - tri->b.z;
        float d = tri->a.x - tri->c.x;
        float e = tri->a.y - tri->c.y;
        float f = tri->a.z - tri->c.z;
        float g = ray->direction.x;
        float h = ray->direction.y;
        float i = ray->direction.z;

        float j = tri->a.x - ray->origin.x;
        float k = tri->a.y - ray->origin.y;
        float l = tri->a.z - ray->origin.z;

        float eihf = e * i - h * f;
        float gfdi = g * f - d * i;
        float dheg = d * h - e * g;

        float M = a * eihf + b * gfdi + c * dheg;

        float akjb = a * k - j * b;
        float jcal = j * c - a * l;
        float blkc = b * l - k * c;

        float t = -(f * akjb + e * jcal + d * blkc) / M;
        if (t < 0)
            return NULL;

        float gamma = (i * akjb + h * jcal + g * blkc) / M;
        if (gamma < 0 || gamma > 1)
            return NULL;

        float beta = (j * eihf + k * gfdi + l * dheg) / M;
        if (beta < 0 || beta > 1 - gamma)
            return NULL;

        Vec3 point = RayEvaluatePoint(ray, t);//?
        Vec3 normal = TriangleNormal(tri);

        RayHit* hit = RayHitCreate(tri->color, tri->specularColor,
                                   tri->phongExponent, tri->reflectiveColor,
                                   point, normal, t);//?
        return hit;
    }
*/



/*
class Model: public Object{
    private:
        std::vector<glm::vec3> verts;
        std::vector<glm::ivec3> faces;

    public:
        Model(const glm::vec3& col, const Material &material, const char *filename) : Object(col, mat), verts(), faces() {
            std::ifstream in;
            in.open (filename, std::ifstream::in);
            if (in.fail()) {
                std::cerr << "Failed to open " << filename << std::endl;
                return;
            }
            std::string line;

            while (!in.eof()) {
                std::getline(in, line);
                std::istringstream iss(line.c_str());
                char trash;
                if (!line.compare(0, 2, "t2 ")) {
                    iss >> trash;
                    glm::vec3 t2;
                    for (int i=0;i<3;i++) iss >> t2[i];
                    verts.push_back(t2);
                } else if (!line.compare(0, 2, "f ")) {
                    glm::ivec3 f;
                    int idx, cnt=0;
                    iss >> trash;
                    while (iss >> idx) {
                        idx--; // in wavefront obj all indices start at 1, not zero
                        f[cnt++] = idx;
                    }
                    if (3==cnt) faces.push_back(f);
                }
            }
            std::cerr << "# t2# " << verts.size() << " f# "  << faces.size() << std::endl;

            glm::vec3 min, max;
            get_bbox(min, max);
        }

        void calc_hit(const Ray& ray, float* sol_arr) {
            for(int fi=0; fi < (int)faces.size(); fi++){
                glm::vec3 edge1 = point(vert(fi,1)) - point(vert(fi,0));
                glm::vec3 edge2 = point(vert(fi,2)) - point(vert(fi,0));
                glm::vec3 norm = glm::cross(ray.dir, edge2);
                float det = dot(edge1, norm);
                if (det > 1e-5){
                    glm::vec3 tmp = ray.start - point(vert(fi,0));

                    float t1 = dot(tmp, norm);
                    if (t1 > 0 && t1 < det){
                        glm::vec3 tmp2 = cross(tmp, edge1);
                        float t2 = dot(ray.dir, tmp2);
                        if (t2 > 0 && t1 + t2 < det){
                            sol_arr[0] = sol_arr[1] = dot(edge2, tmp2) *float(1./det);
                        }
                    }
                }
            }      
        }

        const glm::vec3& point(int i) const{// coordinates of the vertex i
            assert(i >= 0 && i < (int)verts.size());
            return verts[i];
        }

        int vert(int fi, int li) const {// index of the vertex for the triangle fi and local index li
            assert(fi>=0 && fi < (int)faces.size() && li>=0 && li<3);
            return faces[fi][li];
        }

        void get_bbox(glm::vec3 &min, glm::vec3 &max) {// ограничительная рамка для всех вершин, включая изолированные
            min = max = verts[0];
            for (int i=1; i<(int)verts.size(); ++i) {
                for (int j=0; j<3; j++) {
                    min[j] = std::min(min[j], verts[i][j]);
                    max[j] = std::max(max[j], verts[i][j]);
                }
            }
            //std::cerr << "bbox: [" << min << " : " << max << "]" << std::endl;
        }
};*/

/*
std::ostream& operator<<(std::ostream& out, Model &m) {
    for (int i=0; i < (int)m.verts.size(); i++) {
        out << "t2 " << m.point(i) << std::endl;
    }
    for (int i=0; i < (int)m.faces.size(); i++) {
        out << "f ";
        for (int k=0; k<3; k++) {
            out << (m.vert(i,k)+1) << " ";
        }
        out << std::endl;
    }
    return out;
}
*/

#endif