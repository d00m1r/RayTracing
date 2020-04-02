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
#include "BMP/EasyBMP.h"

void make_render(char* output_path, int& scene_number, int& threads){
    Window win;
    std::vector<glm::vec3> buffer(win.width * win.height);
    int displacement = win.width;

    std::vector<glm::vec3> tr {glm::vec3(0.0f, 4.f ,4.3f), glm::vec3(2.0f, 4.f ,4.3f), glm::vec3(2.0f, 3.f ,1.f)};

    std::vector<Object*> obj_list{//дин массив указателей на объекты
        new Sphere(glm::vec3(0.0f, 0.0f, 1.0f), matte, glm::vec3(3.0f, -1.f ,-3.f), 1.0f),
        new Sphere(glm::vec3(0.0f, 0.0f, 1.0f), gloss, glm::vec3(-3.0f, -1.f ,3.f), 1.0f),
        new Sphere(glm::vec3(1.0f, 0.0f, 0.0f), glass, glm::vec3(0.0f, -0.3f ,7.f), 0.3f),
        new Sphere(glm::vec3(0.2f, 0.2f, 0.2f), metal, glm::vec3(6.0f, -2.f ,-6.f), 2.0f),
        
        //far light sphere
        new Sphere(glm::vec3(1.0f, 1.0f, 1.0f), lamp , glm::vec3(0.0f, -14.f , -52.f), 0.4f),

        //central sphere
        new Sphere(glm::vec3(1.0f, 1.0f, 1.0f), mirror, glm::vec3(0.0f, -1.f ,-1.f), 1.0f),

        new Triangle(glm::vec3(1.0f, 0.0f, 0.0f), mirror , tr),
        //Plane
        new Triangle(glm::vec3(0.36f, 0.59f, 0.92f), gloss , std::vector<glm::vec3> {glm::vec3(-win.inf, 0.0f ,win.inf),  glm::vec3(win.inf, 0.0f ,-win.inf), glm::vec3(-win.inf, 0.0f ,-win.inf)}),
        new Triangle(glm::vec3(0.36f, 0.59f, 0.92f), gloss , std::vector<glm::vec3> {glm::vec3(-win.inf, 0.0f ,win.inf), glm::vec3(win.inf,  0.0f , win.inf), glm::vec3( win.inf ,0.0f,-win.inf)}),
    };     

    std::vector<Light> light_list{
        Light(glm::vec3(14.7f,-12.0f,14.0f),   glm::vec3(1.0f,1.0f,1.0f), 0.3f),
        Light(glm::vec3(0.0f,-12.f, -50.0f),   glm::vec3(1.0f,1.0f,1.0f), 0.7f),
        //Light(glm::vec3(17.1f,-20.0f,64.0f), glm::vec3(1.0f,1.0f,1.0f),   0.5f)
    };

    BMP img;
    img.ReadFromFile("../imgs/stars.bmp");

    if(scene_number == 1){
        glm::vec3 O1(0.0f, -0.5f, 20.0f);//прямо по Z
        for (int j = 0; j < win.height; j++){
            for (int i = 0; i < win.width; i++){
                //auto u = int((i + random_double()) / win.width);
                //auto v = int((j + random_double()) / win.height);
                Ray ray(O1,O1);
                ray = camera_set(O1, win, i, j, 0.1f, 0.f, 0.f); 
                /*                              30    0    0   сверху
                                                0     60   0   сзади    
                                                0     90   0   слева 
                                                0     0    240 перевернуть 
                                                                    */
                buffer[j * displacement + i] = beam_shot(ray, obj_list, light_list, win, O1, win.depth, img);//ret col
            }
        }
    }

    else if (scene_number == 2){
        glm::vec3 O1(0.0f, -50.0f, 0.0f);//прямо по Z
        for (int j = 0; j < win.height; j++){
            for (int i = 0; i < win.width; i++){
                //auto u = int((i + random_double()) / win.width);
                //auto v = int((j + random_double()) / win.height);
                Ray ray(O1,O1);
                ray = camera_set(O1, win, i, j, -win.pi/2.f, 0.f, 0.f); 
                /*                              30    0    0   сверху
                                                0     60   0   сзади    
                                                0     90   0   слева 
                                                0     0    240 перевернуть 
                                                                    */
                buffer[j * displacement + i] = beam_shot(ray, obj_list, light_list, win, O1, win.depth, img);//ret col
            }
        }
    }

    //Освобождение памяти
    for(int i = 0; i < (int)obj_list.size(); ++i)
        delete obj_list[i];

    save_image(win, buffer, output_path);    
}

int main(int argc, char *argv[]){
    int scene_number = 1;
    int threads = 1;
    char *output_path = "314_damir_miniakhmetov.bmp";
    for (int count=0; count < argc; ++count){
        if (!strcmp(argv[count], "-out"))  output_path = argv[++count];
        if (!strcmp(argv[count], "-scene")){  
            std::stringstream convert(argv[++count]);
	        if(!(convert >> scene_number)) scene_number = 1;
        }
        if (!strcmp(argv[count], "-threads")){
            std::stringstream convert(argv[++count]);
	        if(!(convert >> threads)) threads = 1;
        }
    }
    if(scene_number < 1 || scene_number > 2) exit(0);
    make_render(output_path, scene_number, threads);
    return 0;
}
