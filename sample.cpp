#include <iostream>
#include <fstream>
#include "glm.hpp"
#include <vector>

#include "EasyBMP.h"
using namespace std;
using namespace glm;

struct sRay {
	glm::vec3 m_start;// начало
	glm::vec3 m_dir;// конец
};

struct sCamera {
	
	std::vector<glm::vec3> m_pixels;// вектор из точек в пространстве( холст)
	
	
	void cset(glm::uvec2 newRs, glm::vec3 newEye, glm::vec3 newV, float newA);
	
	glm::vec3 pos(glm::uvec2 pix);
	
	glm::vec3 dir(glm::uvec2 pix);
	
	glm::uvec2 rs;// разрешение
	glm::vec3 eye, v;// вектор нормали к холсту
	float a;// угол
	glm::vec3 u, n;//ortogonal vectors
	float N, H, W;
	
	
private:
	void calc();
};

class cScene {
public:
	cScene():iter(250), step(0.15), ra(1.0), Ra(9.0) {}
	
	void scset(int newIter, float newStep, float newra, float newRa);//заполняем
	int iter;
	float step, ra, Ra;// ra мылый акреционный диск Ra внешний
};

class cTracer {
public:
	BMP pImageD, pImageS;// небо и диск
	sCamera m_camera;
	cScene m_scene;
	cTracer() { Load();}//обЪединяем сцену и камеру
	cTracer(const sCamera &c, const cScene &s) : m_camera(c), m_scene(s) {
		Load();
	}
	
	void tset(const sCamera &c, const cScene &s) {
		m_camera = c;//передаем структуру камеры и сцены для трейсера и загружаем две картинки
		m_scene = s;
		Load();
	}
	
	void Load();
	
	void SaveImageToFile(char* fileName);
	
	sRay MakeRay(glm::uvec2 pixelPos);
	
	glm::vec3 TraceRay(sRay ray);
	
	void RenderImage();
	
	glm::vec4 lookupDt(float x, float y);
	glm::vec3 lookupSt(float x, float y);
};

int main(int argc, char **argv) {
	sCamera camera;
	cScene scene;
	cTracer tracer;
	glm::uvec2 rs; //w and h
	glm::vec3 eye, v;//position of camera
	float a; //angle of view
	int iter;//limit of iterations
	float step, ra, Ra;
	
	
		FILE *file = fopen(argv[1], "r");
		fscanf(file, "%d%d", &rs.x, &rs.y);
		fscanf(file, "%f%f%f", &eye.x, &eye.y, &eye.z);
		fscanf(file, "%f%f%f", &v.x, &v.y, &v.z);
		fscanf(file, "%f%d%f%f%f", &a, &iter, &step, &ra, &Ra);

	std::cout << rs.x << ' ' << rs.y << ' ' << eye.x << ' ' << eye.y << ' ' << eye.z << ' ' << v.x << ' ' << v.y << ' ' << v.z << ' ' << a;
	std::cout << ' ' << iter << ' ' << step << ' ' << ra << ' ' << Ra << std::endl;
	camera.cset(rs, eye, v, a);
	scene.scset(iter, step, ra, Ra);
	tracer.tset(camera, scene);
	tracer.RenderImage();
	tracer.SaveImageToFile("2.bmp");

	return 0;
	
}

void sCamera::cset(glm::uvec2 newRs, glm::vec3 newEye, glm::vec3 newV, float newA) {
	rs = newRs;//забивем то что обЪявили(холст)
	eye = newEye;
	v = newV;
	a = newA;
	m_pixels.resize(rs.x * rs.y);
	N = 1.0;
	calc();
}

glm::vec3 sCamera::pos(glm::uvec2 pix) {
	glm::vec3 t;//определяет позицию пикселя 
	float uc, vz;
	uc = W * ((pix.x + 0.5) / rs.x - 0.5);
	vz = H * ((pix.y + 0.5) / rs.y - 0.5);
	t = eye - N * n + uc * u + vz * v;
	return t;
}

glm::vec3 sCamera::dir(glm::uvec2 pix) {
	glm::vec3 t = pos(pix) - eye;//куда направлне наш пиксель с холста
	return glm::normalize(t);
}

void sCamera::calc() {
	n = glm::normalize(eye);
	u = glm::normalize(glm::cross(v, n));// находим ортонормированный базис v- ориентация камеры
	v = glm::normalize(glm::cross(n, u));
	a = atan(a);//находим угол вылета пикселя
	H = 2 * N * tan(a/2);
	W = H * (rs.x / rs.y);
}

void cScene::scset(int newIter, float newStep, float newra, float newRa) {
	iter = newIter;
	step = newStep;
	ra = newra;
	Ra = newRa;
}



void cTracer::Load() {
	pImageD.ReadFromFile("disk_24.bmp");
	
	pImageS.ReadFromFile("stars.bmp");
}

void cTracer::SaveImageToFile(char* fileName) {
	BMP image;
	int width = m_camera.rs.x;
	int height = m_camera.rs.y;
	image.SetSize(width, height);
	int i, j;
	
	int textureDisplacement = 0;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			glm::vec3 color = m_camera.m_pixels[textureDisplacement + j];
			image(i,j)->Blue=clamp(color.b, 0.0f, 1.0f) * 255.0f;
			image(i,j)->Green=clamp(color.g, 0.0f, 1.0f) * 255.0f;
			image(i,j)->Red=clamp(color.r, 0.0f, 1.0f) * 255.0f;
		}
		textureDisplacement += width;
	}
	image.WriteToFile(fileName);
}

sRay cTracer::MakeRay(glm::uvec2 pixelPos) {
	sRay r;
	r.m_start = m_camera.pos(pixelPos);//Pos узнает кооинаты пикселя 
	r.m_dir = m_camera.dir(pixelPos);// дир направление
	return r;
}

glm::vec3 cTracer::TraceRay(sRay ray) {
	glm::vec3 color(0, 0, 0);
	//int iter = m_scene.iter;
	float tau = m_scene.step, Ra = m_scene.Ra;
	glm::vec3 r = ray.m_start, v = ray.m_dir;
	glm::vec3 a, r0;// а ускорение
	float l , am;
	for (int i = 0; i < m_scene.iter; i++) {
		r0 = r;
		l = glm::length(r);
		am = 1 / (2 * l * l * l);
		a = -r * am;// ускорение
		float tau2 = tau * tau * 0.5;// тау=степ  
		r += v * tau + a * tau2;
		v += a * tau;// скорость растет пошагово
		v = glm::normalize(v);// проходим по чуть - чуть
		if (r0.z * r.z <= 0) {
			float t = -r0.z / v.z;// время
			glm::vec3 P = r0 + v * t;// путь?,р0 предидушее положение
			l = length(P);// длина пути?
			if (l >= m_scene.ra && l <= m_scene.Ra) {
				float x = P.x, y = P.y;// попало ли в диск
				//r = sqrt(x * x + y * y);
				//cos(phi) = x / 2;
				x = (x + m_scene.Ra) / (2 * m_scene.Ra); //((m_scene.Ra - m_scene.ra) * 2 + m_scene.ra) * cos(2 * 3.141592653 * phi);
				y = (y + m_scene.Ra) / (2 * m_scene.Ra); //преобразование координат в двумерные
				glm::vec4 color4 = lookupDt(x,y);
			//	if (color4[3] > 0) {// хранит альфу [3] прозрачность
					color = glm::vec3(color4);
					return color;
				//}
			}
		}
		if (length(r) < 1) {
			return glm::vec3(0,0,0); // попали в черную дыру
		}
		if (i == m_scene.iter - 1 || (i > m_scene.iter / 2 && length(a * tau) < 0.005 && length(r) > Ra)){// Lenght(a*tau) изменение скорости
			double pi = 3.141592653;// попали в небо 
			float phi = atan2f(v.x, v.y), teta = asin(v.z) + pi/2;
			phi /= (2 * pi);
			teta /= pi;
			return lookupSt(phi, teta);
		}
	}
	return glm::vec3(0,0,0);
}

void cTracer::RenderImage() {
	int xRes = m_camera.rs.x, yRes = m_camera.rs.y;
	for (int i = 0; i < yRes;i++) {//проходимся по пикселям и пускаем луч
		for (int j = 0; j < xRes; j++) {
			sRay ray = MakeRay(glm::uvec2(i, j)); 
			m_camera.m_pixels[i * xRes + j] = TraceRay(ray);// смотрим по шагам как он пойдет(возврашает цвет)
		}//попадаем в определенный пиксель
	}
}

glm::vec4 cTracer::lookupDt(float x, float y) {
	int W = pImageD.TellWidth();
	int H = pImageD.TellHeight();
	int i,j;
	x *= W;
	y *= H;
	j = (int)x;
	i = (int)y;
	if ( j + 1 - x < x - j && j + 1 < W) j++;
	if (i + 1 - y < y - i && i + 1 < H) i++;
	unsigned char b = pImageD(j, i)->Blue;
	unsigned char g = pImageD(j, i)->Green;
	unsigned char r = pImageD(j, i)->Red;
	float b1 = (float)b / 255;
	float g1 = (float)g / 255;
	float r1 = (float)r / 255;
	return vec4(r1, g1, b1, 0);

}

glm::vec3 cTracer::lookupSt(float x, float y) {
	int W = pImageS.TellWidth();
	int H = pImageS.TellHeight();
	int i,j;
	x *= W;
	y *= H;
	j = (int)x;
	i = (int)y;
	if ( j + 1 - x < x - j && j + 1 < W) j++;
	if (i + 1 - y < y - i && i + 1 < H) i++;
	unsigned char b = pImageS(j, i)->Blue;
	unsigned char g = pImageS(j, i)->Green;
	unsigned char r = pImageS(j, i)->Red;
	float b1 = (float)b / 255;
	float g1 = (float)g / 255;
	float r1 = (float)r / 255;
	return vec3(r1, g1, b1);
}