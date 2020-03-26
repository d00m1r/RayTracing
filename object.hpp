#ifndef RAY_TRACING_OBJECT
#define RAY_TRACING_OBJECT

#include <iostream>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "ray.hpp"
#include "material.hpp"

class Sphere{
    public:
        Sphere(){};
        Sphere(const glm::vec3& cen, const glm::vec3& col, float rad, const Material &material): center(cen), radius(rad), mat(material){mat.color = col;};

        glm::vec3 center;
        float radius;
        Material mat;

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
/*
class Model{
    private:
        std::vector<glm::vec3> verts;
        std::vector<glm::vec3> faces;

    public:
        Model(const char *filename) : verts(), faces() {
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
                if (!line.compare(0, 2, "v ")) {
                    iss >> trash;
                    Vec3f v;
                    for (int i=0;i<3;i++) iss >> v[i];
                    verts.push_back(v);
                } else if (!line.compare(0, 2, "f ")) {
                    Vec3i f;
                    int idx, cnt=0;
                    iss >> trash;
                    while (iss >> idx) {
                        idx--; // in wavefront obj all indices start at 1, not zero
                        f[cnt++] = idx;
                    }
                    if (3==cnt) faces.push_back(f);
                }
            }
            std::cerr << "# v# " << verts.size() << " f# "  << faces.size() << std::endl;

            Vec3f min, max;
            get_bbox(min, max);
        }

        int Model::nverts() const {
            return (int)verts.size();
        }                          // number of vertices

        int Model::nfaces() const {
            return (int)faces.size();
        }                          // number of triangles

        // Moller and Trumbore
        bool Model::ray_triangle_intersect(const int &fi, const Vec3f &orig, const Vec3f &dir, float &tnear) {
            Vec3f edge1 = point(vert(fi,1)) - point(vert(fi,0));
            Vec3f edge2 = point(vert(fi,2)) - point(vert(fi,0));
            Vec3f pvec = cross(dir, edge2);
            float det = edge1*pvec;
            if (det<1e-5) return false;

            Vec3f tvec = orig - point(vert(fi,0));
            float u = tvec*pvec;
            if (u < 0 || u > det) return false;

            Vec3f qvec = cross(tvec, edge1);
            float v = dir*qvec;
            if (v < 0 || u + v > det) return false;

            tnear = edge2*qvec * (1./det);
            return tnear>1e-5;
        }

        const Vec3f &Model::point(int i) const{// coordinates of the vertex i
            assert(i>=0 && i<nverts());
            return verts[i];
        }

        glm::vec3 &point(int i);                   // coordinates of the vertex i

        int Model::vert(int fi, int li) const {// index of the vertex for the triangle fi and local index li
            assert(fi>=0 && fi<nfaces() && li>=0 && li<3);
            return faces[fi][li];
        }

        void Model::get_bbox(Vec3f &min, Vec3f &max) {// bounding box for all the vertices, including isolated ones
            min = max = verts[0];
            for (int i=1; i<(int)verts.size(); ++i) {
                for (int j=0; j<3; j++) {
                    min[j] = std::min(min[j], verts[i][j]);
                    max[j] = std::max(max[j], verts[i][j]);
                }
            }
            std::cerr << "bbox: [" << min << " : " << max << "]" << std::endl;
        }
};

std::ostream& operator<<(std::ostream& out, Model &m) {
    for (int i=0; i<m.nverts(); i++) {
        out << "v " << m.point(i) << std::endl;
    }
    for (int i=0; i<m.nfaces(); i++) {
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