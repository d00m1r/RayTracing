#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/random.hpp>

class Ray {
    public:
        Ray() {}
        Ray(const glm::vec3& a, const glm::vec3& b) { A = a; B = b; }
        glm::vec3 origin() const       { return A; }
        glm::vec3 direction() const    { return B; }
        glm::vec3 point_at_parameter(float t) const { return A + t*B; }

        glm::vec3 A;
        glm::vec3 B;
};

class Camera {
    public:
        Camera(){
            lower_left_corner = glm::vec3(-2.0f, -1.0f, -1.0f);
            horizontal        = glm::vec3(4.0f, 0.0f, 0.0f);
            vertical          = glm::vec3(0.0f, 2.0f, 0.0f);
            origin            = glm::vec3(0.0f, 0.0f, 0.0f);
        }

        Ray get_ray(float screenshot_x, float screenshot_y){
            return Ray(origin, lower_left_corner + screenshot_x*horizontal + screenshot_y*vertical - origin);
        }

        glm::vec3 origin;
        glm::vec3 lower_left_corner;
        glm::vec3 horizontal;
        glm::vec3 vertical;
};

struct hit_record {
    float t;
    glm::vec3 p;
    glm::vec3 normal;
};

class hittable  {
    public:
        virtual bool hit(
            const Ray& ray, float t_min, float t_max, hit_record& rec) const = 0;
};

bool hit_sphere(const glm::vec3& center, float radius, const Ray& ray) {
    glm::vec3 oc = ray.origin() - center;
    
    float a = dot(ray.direction(), ray.direction());
    float b = 2.0f * dot(oc, ray.direction());
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    if (discriminant < 0) {
        return -1.0f;
    }
    else {
        return (-b - sqrt(discriminant) ) / (a*2.0f);
    }
}

class hittable_list: public hittable {
    public:
        hittable_list() {}
        hittable_list(hittable **l, int n) {list = l; list_size = n; }
        virtual bool hit(
            const Ray& ray, float tmin, float tmax, hit_record& rec) const;
        hittable **list;
        int list_size;
};

bool hittable_list::hit(const Ray& ray, float t_min, float t_max,
                        hit_record& rec) const {

    hit_record temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;
    for (int i = 0; i < list_size; i++) {
        if (list[i]->hit(ray, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    return hit_anything;
}

class sphere: public hittable  {
    public:
        sphere() {}
        sphere(glm::vec3 cen, float ray) : center(cen), radius(ray)  {};
        virtual bool hit(const Ray& ray, float tmin, float tmax, hit_record& rec) const;
        glm::vec3 center;
        float radius;
};

bool sphere::hit(const Ray& ray, float t_min, float t_max, hit_record& rec) const {
    glm::vec3 oc = ray.origin() - center;
    float a = dot(ray.direction(), ray.direction());
    float b = dot(oc, ray.direction());
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - a*c;
    if (discriminant > 0) {
        float temp = (-b - sqrt(discriminant))/a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = ray.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            return true;
        }
        temp = (-b + sqrt(discriminant)) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = ray.point_at_parameter(rec.t);
            rec.normal = (rec.p - center) / radius;
            return true;
        }
    }
    return false;
}

glm::vec3 color(const Ray& r) {
    if (hit_sphere(glm::vec3(0.0f,0.0f,-1.0f), 0.5f, r))
        return glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 unit_direction = glm::normalize(r.direction());
    float t = (unit_direction.y + 1.0)*0.5f;
    return glm::vec3(1.0f, 1.0f, 1.0f)*(1.0-t) + glm::vec3(0.5f, 0.7f, 1.0f)*t;
}

int main() {
    int W = 1024;//view
    int H = 500;
    int ns = 10;


    glm::vec3 lower_left_corner(-2.0f, -1.0f, -1.0f);
    glm::vec3 horizontal(4.0f, 0.0f, 0.0f);//x
    glm::vec3 vertical(0.0f, 2.0f, 0.0f);//y
    glm::vec3 origin(0.0f, 0.0f, 0.0f);

    hittable *list[2];
    list[0] = new sphere(glm::vec3(0.0f,0.0f,-1.0f), 0.5);
    list[1] = new sphere(glm::vec3(0.0f,-100.5f,-1.0f), 100);
    hittable *world = new hittable_list(list,2);
    Camera cam;

    std::ofstream ofs;
    ofs.open("./scene.ppm");
    ofs << "P3\n" << W << " " << H << "\n255\n";

    for (int j = H-1; j >= 0; j--) {
        for (int i = 0; i < W; i++) {
            float u = float(i) / float(W);
            float v = float(j) / float(H);
            Ray ray(origin, lower_left_corner + u*horizontal + v*vertical);
            glm::vec3 col = color(ray);
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);

            std::cout << ir << " " << ig << " " << ib << "\n";
        }
    }
    ofs.close();
}