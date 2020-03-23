#ifndef RAY_TRACING_RAY
#define RAY_TRACING_RAY

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

#include "window.hpp"

class Ray{
    public:
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

#endif