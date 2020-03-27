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
#include "material.hpp"

void make_render(){

    Window win;
    std::vector<glm::vec3> buffer(win.width * win.height);
    int displacement = win.width;
    
    std::vector<Sphere> obj_list{
        Sphere(glm::vec3(0.0f, 4.f,-1.f),  glm::vec3(1.0f, 0.0f, 0.0f), 3.f, matte),//R
        Sphere(glm::vec3(0.0f, 0.f,-0.5f),   glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, glass),
        Sphere(glm::vec3(0.4f, -1.f,0.9f), glm::vec3(0.0f, 0.0f, 0.0f), 0.5f, mirror),//G
        Sphere(glm::vec3(0.8f, 0.2f,1.5f),  glm::vec3(0.2f, 0.2f, 0.2f), 0.3f, metal),
        //Plane(glm::vec3(0.2f, 0.2f, 0.2f), metal),
    };                                                                   //rad shine reflective

    std::vector<Light> light_list{
        Light(glm::vec3(14.7f,-12.0f,14.0f),   glm::vec3(1.0f,1.0f,1.0f), 0.5f),
        Light(glm::vec3(17.1f,-20.0f,64.0f), glm::vec3(1.0f,1.0f,1.0f),   0.5f)
    };

    glm::vec3 O1(0.0f, 0.0f, -10.0f);//прямо по Z
    BMP img;
    img.ReadFromFile("space1.bmp");

    for (int j = 0; j < win.height; j++){
        for (int i = 0; i < win.width; i++){
            Ray ray(O1,O1);
            ray = camera_set(O1, win, i, j, 0.f, 59.7f, 0.f); 
            /*                              30    0    0   сверху
                                            0     60   0   сзади    
                                            0     90   0   слева 
                                            0     0    240 перевернуть 
                                                                */
            buffer[j * displacement + i] = beam_shot(ray, obj_list, light_list, win, O1, win.depth, img);//ret col
        }
    }
    char name[] = "scene.bmp";
    save_image(win, buffer, name);    
}

int main(){
    make_render();
    return 0;
}