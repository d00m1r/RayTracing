#ifndef RAY_TRACING_MATERIAL
#define RAY_TRACING_MATERIAL

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/mat3x3.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>

#include "ray.hpp"

class Material{
    public:
        glm::vec4 features;
        glm::vec3 color;
        float reflect;
        float shine;
        float refract;
        
        Material(){};
        Material(const glm::vec4 &f, const float reflect, const float refract, const float shine):
            features(f), reflect(reflect), refract(refract), shine(shine){};
        /*
        Material& operator=(const Material& right) {
                //проверка на самоприсваивание
                if (this == &right) {
                    return *this;
                }
                color    = right.color;
                features = right.features;
                reflect  = right.reflect;
                refract  = right.refract;
                shine    = right.shine;
                return *this;
            }
            */
};
                        //color refl refr shine refl refr   shine
Material matte  (glm::vec4(1.f, 0.f, 0.f, 1.f), 0.0f, 0.0f,  15.f);
Material gloss  (glm::vec4(1.f, 1.f, 0.f, 1.f), 0.250f, 0.0f, 100.f);
Material metal  (glm::vec4(1.f, 1.f, 0.f, 1.f), 0.2f, 0.0f,  5.f);
Material glass  (glm::vec4(0.f, 1.f, 1.f, 1.f), 0.04f, 2.5f,  750.f);
Material mirror (glm::vec4(0.f, 1.f, 0.f, 1.f), 0.85f, 0.0f, 100.f);
Material lamp  (glm::vec4(1.f, 0.f, 0.f, 1.f), 0.0f, 0.0f,  -500.f);
#endif