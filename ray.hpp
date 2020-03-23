#ifndef RAY_TRACING_RAY
#define RAY_TRACING_RAY

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include "window.hpp"

class Ray{
    public:
        Ray(){};
        Ray(const glm::vec3& orig, const glm::vec3& direction): start(orig), dir(direction){};
        ~Ray(){};
        glm::vec3 start;
        glm::vec3 dir;

    Ray& operator=(const Ray& right) {
        //проверка на самоприсваивание
        if (this == &right) {
            return *this;
        }
        start = right.start;
        dir = right.dir;
        return *this;
    }
};

glm::vec3 ray_dir(int i, int j, Window win){
    glm::vec3 dir;
    dir.x =  (i + 0.5f) -  (win.width/2.0f);
    dir.y = (j + 0.5f) - (win.height/2.0f);
    dir.z = -(win.width)/tan(win.fov/2.0f);
    return glm::normalize(dir);
}

glm::mat3 calc_rotx_matrix(float angle){
    return glm::mat3 (1.f, 0.f, 0.f,
                      0.f, glm::cos(angle), -glm::sin(angle),
                      0.f, glm::sin(angle), glm::cos(angle));
}
glm::mat3 calc_roty_matrix(float angle){
    return glm::mat3 (glm::cos(angle), 0.f, glm::sin(angle),
                      0.f, 1.f, 0.f,
                      -glm::sin(angle), 0.f, glm::cos(angle));
}
glm::mat3 calc_rotz_matrix(float angle){
    return glm::mat3 (glm::cos(angle), -glm::sin(angle), 0.f,
                      glm::sin(angle), glm::cos(angle), 0.f,
                      0.f, 0.f, 1.f);
}

Ray camera_set(glm::vec3 cam_pos, Window win, int i, int j, float angle_x = 0.f, float angle_y = 0.f, float angle_z = 0.f){
    glm::mat3 rot(1.f,0.f,0.f,0.f,1.f,0.f,0.f,0.f,1.f);
    if(angle_x){ rot *= calc_rotx_matrix(angle_x);}
    if(angle_y){ rot *= calc_roty_matrix(angle_y);}
    if(angle_z){ rot *= calc_rotz_matrix(angle_z);}
    return Ray(cam_pos, rot * ray_dir(i, j, win));//уже нормализованный
}

#endif