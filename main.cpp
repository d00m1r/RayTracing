#include <iostream>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/vec3.hpp>

class ray {
    public:
        ray() {}
        ray(const glm::vec3& a, const glm::vec3& b) { A = a; B = b; }
        glm::vec3 origin() const       { return A; }
        glm::vec3 direction() const    { return B; }
        glm::vec3 point_at_parameter(float t) const { return A + t*B; }

        glm::vec3 A;
        glm::vec3 B;
};

bool hit_sphere(const glm::vec3& center, float radius, const ray& r) {
    glm::vec3 oc = r.origin() - center;
    float a = dot(r.direction(), r.direction());
    float b = 2.0 * dot(oc, r.direction());
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    return (discriminant > 0);
}

glm::vec3 color(const ray& r) {
    //returns the color of the background (a simple gradient)
    if (hit_sphere(glm::vec3(0.0f,0.0f,-1.0f), 0.5f, r))
        return glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 unit_direction = glm::normalize(r.direction());
    float t = 0.5*(unit_direction.y + 1.0);
    return glm::vec3(1.0f, 1.0f, 1.0f)*float(1.0-t) + glm::vec3(0.5f, 0.7f, 1.0f)*float(t);
}

int main() {
    int nx = 200;//view
    int ny = 100;
    std::ofstream ofs;
    ofs.open("./scene.ppm");
    ofs << "P3\n" << nx << " " << ny << "\n255\n";

    glm::vec3 lower_left_corner(-2.0f, -1.0f, -1.0f);
    glm::vec3 horizontal(4.0f, 0.0f, 0.0f);//x
    glm::vec3 vertical(0.0f, 2.0f, 0.0f);//y
    glm::vec3 origin(0.0f, 0.0f, 0.0f);

    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            float u = float(i) / float(nx);
            float v = float(j) / float(ny);
            ray r(origin, lower_left_corner + u*horizontal + v*vertical);
            glm::vec3 col = color(r);
            int ir = int(255.99*col[0]);
            int ig = int(255.99*col[1]);
            int ib = int(255.99*col[2]);
            ofs << ir << " " << ig << " " << ib << "\n";
        }
    }
    ofs.close();
}   