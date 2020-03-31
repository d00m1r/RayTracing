#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <memory>

#include "window.hpp"
#include "ray.hpp"
#include "effects.hpp"
#include "object.hpp"
#include "material.hpp"

void make_render(){

    Window win;
    std::vector<glm::vec3> buffer(win.width * win.height);
    int displacement = win.width;
    
    //auto s1 = std::make_shared<Sphere>(glm::vec3(0.0f, 4.f,-1.f),  glm::vec3(1.0f, 0.0f, 0.0f), 3.f, matte);
    //auto s2 = std::make_shared<Sphere>(glm::vec3(0.0f, 4.f,-1.f),  glm::vec3(1.0f, 0.0f, 0.0f), 3.f, matte);//R
    //auto s3 = std::make_shared<Sphere>(glm::vec3(0.0f, 0.f,-0.5f),   glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, glass);
    //auto s4 = std::make_shared<Sphere>(glm::vec3(0.4f, -1.f,0.9f), glm::vec3(0.0f, 0.0f, 0.0f), 0.5f, mirror);//G
    //auto s5 = std::make_shared<Sphere>(glm::vec3(0.8f, 0.2f,1.5f),  glm::vec3(0.4f, 0.4f, 0.4f), 0.3f, metal);
    //auto p1 = std::make_shared<Plane>(glm::vec3(0.2f, 0.2f, 0.2f), metal),
//
//
    //std::vector< std::shared_ptr<Object> > obj_list;//не удалось реализовать cast                                                               
    //obj_list.push_back(std::move(s1));
    //obj_list.push_back(std::move(s2));
    //obj_list.push_back(std::move(s3));
    //obj_list.push_back(std::move(s4));
    //obj_list.push_back(std::move(s5));
    //obj_list.push_back(std::move(p1));

    std::vector<glm::vec3> tr {glm::vec3(0.0f, 4.f ,4.3f), glm::vec3(2.0f, 4.f ,4.3f), glm::vec3(2.0f, 3.f ,1.f)};

    std::vector<Object*> obj_list{//дин массив указателей на объекты
        new Sphere(glm::vec3(1.0f, 0.0f, 0.0f), matte , glm::vec3(0.0f, 4.f ,-1.f), 3.0f),//R
        new Sphere(glm::vec3(1.0f, 1.0f, 1.0f), glass , glm::vec3(0.0f, 0.f,-0.5f), 1.0f),
        new Sphere(glm::vec3(0.0f, 0.0f, 0.0f), mirror, glm::vec3(0.4f, -1.f,0.9f), 0.5f),//G
        new Sphere(glm::vec3(0.2f, 0.2f, 0.2f), metal , glm::vec3(0.8f, 0.2f,1.5f), 0.3f),
        new Triangle(glm::vec3(1.0f, 0.0f, 0.0f), mirror , tr),
        new Sphere(glm::vec3(0.2f, 0.2f, 0.2f), metal , glm::vec3(1.1f, 3.4f,3.7f), 0.2f),
        //new Plane(glm::vec3(0.2f, 0.2f, 0.2f), metal),
        //new Model(glm::vec3(1.0f, 0.0f, 0.0f), 3.f, matte, "/home/d/projects/rt/duck.obj"),
    };     

    std::vector<Light> light_list{
        Light(glm::vec3(14.7f,-12.0f,14.0f),   glm::vec3(1.0f,1.0f,1.0f), 0.5f),
        Light(glm::vec3(17.1f,-20.0f,64.0f), glm::vec3(1.0f,1.0f,1.0f),   0.5f)
    };

    glm::vec3 O1(0.0f, -0.6f, 25.0f);//прямо по Z
    BMP img;
    img.ReadFromFile("space1.bmp");

    for (int j = 0; j < win.height; j++){
        for (int i = 0; i < win.width; i++){
            //auto u = int((i + random_double()) / win.width);
            //auto v = int((j + random_double()) / win.height);
            Ray ray(O1,O1);
            ray = camera_set(O1, win, i, j, -0.19f, 0.f, 0.f); 
            /*                              30    0    0   сверху
                                            0     60   0   сзади    
                                            0     90   0   слева 
                                            0     0    240 перевернуть 
                                                                */
            buffer[j * displacement + i] = beam_shot(ray, obj_list, light_list, win, O1, win.depth, img);//ret col
        }
    }
    
    //Освобождение памяти
    for(int i = 0; i < (int)obj_list.size(); ++i)
        delete obj_list[i];

    char name[] = "scene.bmp";
    save_image(win, buffer, name);    
}

int main(){
    make_render();
    return 0;
}