#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include "window.hpp"
#include "ray.hpp"
#include "effects.hpp"
#include "object.hpp"

void make_render(){
    Window win;
    std::vector<glm::vec3> buffer(win.width * win.height);
    int displacement = win.width;

    std::vector<Sphere> sphere_list{
        Sphere(glm::vec3(-1.7f, 0.f,0.8f),  glm::vec3(1.0f, 0.0f, 0.0f), 0.5f, 50,  0.3f),//R
        Sphere(glm::vec3(0.0f, -0.9f,0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.5f, 150, 0.3f),//G
        Sphere(glm::vec3(0.8f, 0.2f,1.2f),  glm::vec3(0.0f, 0.0f, 1.0f), 0.3f, 100, 0.3f),//B
        Sphere(glm::vec3(0.0f, 0.f,0.0f),   glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 20, 0.2f),
    };

    std::vector<Light> light_list{
        Light(glm::vec3(14.7f,-12.0f,14.0f),   glm::vec3(1.0f,1.0f,1.0f), 0.5f),
        Light(glm::vec3(17.1f,-20.0f,64.0f), glm::vec3(1.0f,1.0f,1.0f), 0.5f)
    };

    float min_t = 0;
    float max_t = 999999.0f;
    int depth = 2;

    glm::vec3 O1(0.0f, 0.0f, 7.0f);//прямо по Z
    
    for (int j = 0; j < win.height; j++){
        for (int i = 0; i < win.width; i++){
            Ray ray(O1,O1);
            ray = camera_set(O1, win, i, j, 0.f, 0.f, 180.f); 
            /*                              30    0    0   сверху
                                            0     90   0   слева                          
                                                                */
            buffer[j * displacement + i] = beam_shot(ray, sphere_list, light_list, min_t, max_t, O1, depth);//ret col
        }
    }
    char name[] = "scene.bmp";
    save_image(win, buffer, name);    
}

int main(){
    make_render();
    return 0;
}