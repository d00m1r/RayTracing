#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <memory>
#include <omp.h>

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

    std::vector<glm::vec3> tr1 {glm::vec3(0.0f, 0.f ,-6.f), glm::vec3(-1.0f, -7.f ,-1.f), glm::vec3(-9.0f, 0.f ,3.f)};
    std::vector<glm::vec3> tr2 {glm::vec3(-9.0f, 0.f ,3.f), glm::vec3(-1.0f, -7.f ,-1.f), glm::vec3(-7.0f, 0.f ,-6.f)};
    std::vector<glm::vec3> tr3 {glm::vec3(-7.0f, 0.f ,-6.f), glm::vec3(-1.0f, -7.f ,-1.f), glm::vec3(0.0f, 0.f ,-6.f)};

    std::vector<Object*> obj_list{//дин массив указателей на объекты
        //far light sphere
        new Sphere(glm::vec3(1.0f, 1.0f, 1.0f), lamp , glm::vec3(0.0f, -14.f , -52.f), 0.2f),

        new Sphere(glm::vec3(0.36f, 0.59f, 0.92f), matte, glm::vec3(3.0f, -1.f ,-3.f), 1.f),
        new Sphere(glm::vec3(0.0f, 0.0f, 1.0f), glass, glm::vec3(-1.7f, -0.7f ,5.f), 0.7f),
        new Sphere(glm::vec3(0.58f, 0.36f, 2.f), matte, glm::vec3(-0.7f, -0.35f ,7.f), 0.35f),
        new Sphere(glm::vec3(2.f, 2.f, 0.f), gloss, glm::vec3(-3.5f, -0.7f ,0.1f),0.4f),
        new Sphere(glm::vec3(0.f, 0.93f, 0.93f), metal, glm::vec3(-0.5f, -0.3f ,-0.1f),1.f),
        new Sphere(glm::vec3(0.4f, 0.4f, 0.4f), metal, glm::vec3(1.0f, -0.6f ,-1.5f), 0.6f),
        new Sphere(glm::vec3(1.0f, 1.0f, 1.0f), mirror, glm::vec3(4.0f, -1.5f ,-0.5f), 1.5f),

        //Triangle pyramid
        new Triangle(glm::vec3(1.0f, 0.0f, 0.0f), mirror , tr1),
        new Triangle(glm::vec3(1.0f, 0.0f, 0.0f), mirror , tr2),
        new Triangle(glm::vec3(1.0f, 0.0f, 0.0f), mirror , tr3),
        
        //Plane
        new Triangle(glm::vec3(0.36f, 0.59f, 0.92f), gloss , std::vector<glm::vec3> {glm::vec3(-win.inf, 0.0f ,win.inf),  glm::vec3(win.inf, 0.0f ,-win.inf), glm::vec3(-win.inf, 0.0f ,-win.inf)}),
        new Triangle(glm::vec3(0.36f, 0.59f, 0.92f), gloss , std::vector<glm::vec3> {glm::vec3(-win.inf, 0.0f ,win.inf), glm::vec3(win.inf,  0.0f , win.inf), glm::vec3( win.inf ,0.0f,-win.inf)}),
    };     

    std::vector<Light> light_list{
        Light(glm::vec3(14.7f,-12.0f,14.0f), 0.3f),
        Light(glm::vec3(0.0f,-12.f, -50.0f), 0.7f),
    };

    BMP img;
    img.ReadFromFile("../imgs/stars.bmp");

    omp_set_num_threads(threads);

    if(scene_number == 1){//Вид спереди
        glm::vec3 O1(0.0f, -0.5f, 20.0f);
        #pragma omp parallel for
        for (int j = 0; j < win.height; j++){
            for (int i = 0; i < win.width; i++){
                Ray ray(O1,O1);
                ray = camera_set(O1, win, i, j, 0.1f, 0.f, 0.f); 
                buffer[j * displacement + i] = beam_shot(ray, obj_list, light_list, win, O1, win.depth, img);
            }
        }
    }

    else if (scene_number == 2){//Вид сверху
        glm::vec3 O1(0.0f, -50.0f, 0.0f);
        #pragma omp parallel for
        for (int j = 0; j < win.height; j++){
            for (int i = 0; i < win.width; i++){
                Ray ray(O1,O1);
                ray = camera_set(O1, win, i, j, -win.pi/2.f, 0.f, 0.f); 
                buffer[j * displacement + i] = beam_shot(ray, obj_list, light_list, win, O1, win.depth, img);
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
