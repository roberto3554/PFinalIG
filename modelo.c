/*	Prácticas de Informática Gráfica

	Grupos C y D				Curso 2023-24
 	
	Codigo base para la realización de las practicas de IG
	
	Estudiante: Roberto Retamero Jiménez

=======================================================
	G. Arroyo, J.C. Torres 
	Dpto. Lenguajes y Sistemas Informticos
	(Univ. de Granada)

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details 
 http://www.gnu.org/copyleft/gpl.html

=======================================================/
modulo modelo.c
    Representación del modelo
    Funciones de dibujo
    Función Idle

*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>		// Libreria de utilidades de OpenGL
#include <vector>
#include "practicasIG.h"
#include "file_ply_stl.h"
#include "lector-jpg.h"


/**	void initModel()

Inicializa el modelo y de las variables globales


**/

typedef struct{
  float x, y, z;
} point;

typedef struct{
  int v0, v1, v2;
} index;

typedef struct{
  float x, y, z;
} vector;

typedef struct{
  float u, v;
} textPoint;

constexpr unsigned int N_CARA   = 0b0000;
constexpr unsigned int N_VERT   = 0b0001;
constexpr unsigned int TEXT_ON  = 0b0010;
constexpr unsigned int TEXT_OFF = 0b0000;
constexpr unsigned int MAT_ON   = 0b0100;
constexpr unsigned int MAT_OFF  = 0b0000;

void
initModel ()
{

}

vector calcularVector(point a, point b){
  vector v;
  v.x = b.x-a.x;
  v.y = b.y-a.y;
  v.z = b.z-a.z;
  return v;
}

vector productoVectorial(vector v1, vector v2){
  vector resultado;
  resultado.x = (v1.y*v2.z)-(v1.z*v2.y);
  resultado.y = (v1.z*v2.x)-(v1.x*v2.z);
  resultado.z = (v1.x*v2.y)-(v1.y*v2.x);
  return resultado;
}

void normalizarVector(vector &v){
  float modulo = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
  v.x = v.x/modulo;
  v.y = v.y/modulo;
  v.z = v.z/modulo;
}

void calcularNormalesT(const std::vector<point> &vert, const std::vector<index> &tri, std::vector<vector> &normalT, int nT){
  // printf("TAM: %d\n", nT);
  for(int i=0; i<nT; i++){
    point a, b, c;
    a.x = vert[tri[i].v0].x;
    a.y = vert[tri[i].v0].y;
    a.z = vert[tri[i].v0].z;

    b.x = vert[tri[i].v1].x;
    b.y = vert[tri[i].v1].y;
    b.z = vert[tri[i].v1].z;

    c.x = vert[tri[i].v2].x;
    c.y = vert[tri[i].v2].y;
    c.z = vert[tri[i].v2].z;

    vector v1 = calcularVector(a, b);
    vector v2 = calcularVector(a, c);

    vector v3 = productoVectorial(v1, v2);

    normalizarVector(v3);

    // printf("Normal %i:\nx: %f\ny: %f\nz: %f\n", i, v3.x, v3.y, v3.z);

    normalT[i].x = v3.x;
    normalT[i].y = v3.y;
    normalT[i].z = v3.z;
  }
}

vector suma(vector v1, vector v2){
  vector s;
  s.x = v1.x + v2.x;
  s.y = v1.y + v2.y;
  s.z = v1.z + v2.z;
  return s;
}

void calcularNormalesV(const std::vector<point> &vert, const std::vector<index> &tri, std::vector<vector> &normalT, std::vector<vector> &normalV, int nT, int nV){
  for(int i=0; i<nV; i++){
    normalV[i].x = 0.0;
    normalV[i].y = 0.0;
    normalV[i].z = 0.0;
  }
  for(int i=0; i<nT; i++){
    normalV[tri[i].v0].x += normalT[i].x;
    normalV[tri[i].v0].y += normalT[i].y;
    normalV[tri[i].v0].z += normalT[i].z;

    normalV[tri[i].v1].x += normalT[i].x;
    normalV[tri[i].v1].y += normalT[i].y;
    normalV[tri[i].v1].z += normalT[i].z;

    normalV[tri[i].v2].x += normalT[i].x;
    normalV[tri[i].v2].y += normalT[i].y;
    normalV[tri[i].v2].z += normalT[i].z;
  }
  for(int i=0; i<nV; i++){
    normalizarVector(normalV[i]);
  }
}

class Malla:Objeto3D
{
  protected:
    std::vector<point> vert;
    std::vector<index> tri;
    std::vector<vector> normalV;
    std::vector<vector> normalT;
    std::vector<textPoint> texturePoints;

  int nV, nT;
  GLfloat ra[4] = {0.0, 0.0, 0.0, 0.0};
  GLfloat rd[4] = {0.0, 0.0, 0.0, 0.0};
  GLfloat re[4] = {0.0, 0.0, 0.0, 0.0};
  GLfloat s = 0.0;
  GLuint idText;
  bool vertices = false;
  bool textura  = false;
  bool material = false;

  public:
    Malla(const char *nombre_archivo, unsigned int flags = 0b0000){
      std::vector<float> vertices_ply;
      std::vector<int> caras_ply;
      ply::read(nombre_archivo, vertices_ply, caras_ply);
      nV = round(vertices_ply.size()/3);
      nT = round(caras_ply.size()/3);
      this->mode(flags);

      // reservar memoria
      vert.reserve(nV);
      tri.reserve(nT);
      normalT.reserve(nT);
      normalV.reserve(nV);

      // rellenar vertices
      for(int i=0; i<nV; i++){
        vert[i].x=vertices_ply[i*3];
        vert[i].y=vertices_ply[(i*3)+1];
        vert[i].z=vertices_ply[(i*3)+2];
      }

      // rellenar caras
      for(int i=0; i<nT; i++){
        tri[i].v0=caras_ply[i*3];
        tri[i].v1=caras_ply[(i*3)+1];
        tri[i].v2=caras_ply[(i*3)+2];
      }

      // calcular normales de caras
      calcularNormalesT(vert, tri, normalT, nT);

      // calcular normales de vertices
      calcularNormalesV(vert, tri, normalT, normalV, nT, nV);
    }

    Malla(const char *nombre_archivo, const float rd[4], const float ra[4], unsigned int flags = 0b0000) : Malla(nombre_archivo, flags){
      this->asignarAmbiente(ra);
      this->asignarDifusa(rd);
      material = true;
    }

    Malla(const char *nombre_archivo, const float rd[4], unsigned int flags = 0b0000) : Malla(nombre_archivo, rd, rd, flags){
    }

    void asignarAmbiente(const float r[4]){
      ra[0] = r[0];
      ra[1] = r[1];
      ra[2] = r[2];
      ra[3] = r[3];
    }

    void asignarDifusa(const float r[4]){
      rd[0] = r[0];
      rd[1] = r[1];
      rd[2] = r[2];
      rd[3] = r[3];
    }

    void asignarEspecular(const float r[4]){
      re[0] = r[0];
      re[1] = r[1];
      re[2] = r[2];
      re[3] = r[3];
    }

    void asignarBrillo(const float b){
      s = b;
    }

    void asignarMaterial(const float rd[4], const float ra[4], const float re[4], float b){
      asignarAmbiente(ra);
      asignarDifusa(rd);
      asignarEspecular(re);
      asignarBrillo(b);
      material = true;
    }

    void asignarTextura(const char* nombre_arch){
      glGenTextures(1, &idText);
      glBindTexture(GL_TEXTURE_2D, idText);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      unsigned ancho;
      unsigned alto;
      unsigned char* img = LeerArchivoJPEG(nombre_arch, ancho, alto);

      glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        ancho,
        alto,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        img
      );

      textura = true;
      texturePoints.reserve(nV);

      calcularCoordenadasTextura();
    }

    void calcularCajaEnvolvente(point& min, point& max){
      min.x = vert[0].x;
      min.y = vert[0].y;
      min.z = vert[0].z;

      max.x = vert[0].x;
      max.y = vert[0].y;
      max.z = vert[0].z;

      for(int i=0; i<nV; i++){
        point pos;
        pos.x = vert[i].x;
        pos.y = vert[i].y;
        pos.z = vert[i].z;

        min.x = std::min(min.x, pos.x);
        min.y = std::min(min.y, pos.y);
        min.z = std::min(min.z, pos.z);
      
        max.x = std::max(max.x, pos.x);
        max.y = std::max(max.y, pos.y);
        max.z = std::max(max.z, pos.z);
      }
    }

    void calcularCoordenadasTextura() {
      point min, max;
      calcularCajaEnvolvente(min, max);

      // Rango en cada eje
      float rangeX = max.x - min.x;
      float rangeY = max.y - min.y;
      float rangeZ = max.z - min.z;

      // Determina el plano de proyección en función del mayor rango
      float maxRange = std::max(rangeX, rangeY);
      maxRange = std::max(maxRange, rangeZ);
      enum ProjectionPlane { XY, XZ, YZ };
      ProjectionPlane plane = XY;

      if(maxRange == rangeX){
        plane = (rangeY >= rangeZ) ? XY : XZ;
      }else if(maxRange == rangeY){
        plane = (rangeX >= rangeZ) ? XY : YZ;
      }else{
        plane = (rangeX >= rangeY) ? XZ : YZ;
      }

      for(int i=0; i<nV; i++){
        float u = 0.0f, v = 0.0f;

        // Asignación de coordenadas de textura basada en el plano seleccionado
        switch(plane){
          case XY:
            u = (vert[i].x - min.x) / rangeX;
            v = (vert[i].y - min.y) / rangeY;
            break;
          case XZ:
            u = (vert[i].x - min.x) / rangeX;
            v = (vert[i].z - min.z) / rangeZ;
            break;
          case YZ:
            u = (vert[i].y - min.y) / rangeY;
            v = (vert[i].z - min.z) / rangeZ;
            break;
        }

        // Asignar las coordenadas de textura calculadas
        texturePoints[i].u = u;
        texturePoints[i].v = v;
      }
    }

    // Los flags se pasan separados por ORs lógicos
    void mode(unsigned int flags = 0b0000){
      vertices = ((flags & 0b0001) == 0b0001);
      textura  = ((flags & 0b0010) == 0b0010);
      material = ((flags & 0b0100) == 0b0100);
    }

    void draw(){
      if(material){
        glMaterialfv(GL_FRONT, GL_AMBIENT, ra);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, rd);
        glMaterialfv(GL_FRONT, GL_SPECULAR, re);
        glMaterialfv(GL_FRONT, GL_SHININESS, &s);
      }
      if(vertices && textura)
        drawNVText();
      else if(textura)
        drawNCText();
      else if(vertices)
        drawNV();
      else
        drawNC();
    }

    void drawNC(){
      glShadeModel(GL_FLAT);
      for(int i=0; i<nT; i++){
        glBegin (GL_TRIANGLES);
        glNormal3f(normalT[i].x, normalT[i].y, normalT[i].z);
        glVertex3f(vert[tri[i].v0].x, vert[tri[i].v0].y, vert[tri[i].v0].z);
        glVertex3f(vert[tri[i].v1].x, vert[tri[i].v1].y, vert[tri[i].v1].z);
        glVertex3f(vert[tri[i].v2].x, vert[tri[i].v2].y, vert[tri[i].v2].z);
        glEnd();
      }
    }

    void drawNV(){
      glShadeModel(GL_SMOOTH);
      for(int i=0; i<nT; i++){
        glBegin (GL_TRIANGLES);
        glNormal3f(normalV[tri[i].v0].x, normalV[tri[i].v0].y, normalV[tri[i].v0].z);
        glVertex3f(vert[tri[i].v0].x, vert[tri[i].v0].y, vert[tri[i].v0].z);
        glNormal3f(normalV[tri[i].v1].x, normalV[tri[i].v1].y, normalV[tri[i].v1].z);
        glVertex3f(vert[tri[i].v1].x, vert[tri[i].v1].y, vert[tri[i].v1].z);
        glNormal3f(normalV[tri[i].v2].x, normalV[tri[i].v2].y, normalV[tri[i].v2].z);
        glVertex3f(vert[tri[i].v2].x, vert[tri[i].v2].y, vert[tri[i].v2].z);
        glEnd();
      }
    }

    void drawNCText(){
      glShadeModel(GL_FLAT);
      for(int i=0; i<nT; i++){
        glBegin (GL_TRIANGLES);
        glNormal3f(normalT[i].x, normalT[i].y, normalT[i].z);
        glTexCoord2f(texturePoints[tri[i].v0].u, texturePoints[tri[i].v0].v);
        glVertex3f(vert[tri[i].v0].x, vert[tri[i].v0].y, vert[tri[i].v0].z);
        glTexCoord2f(texturePoints[tri[i].v1].u, texturePoints[tri[i].v1].v);
        glVertex3f(vert[tri[i].v1].x, vert[tri[i].v1].y, vert[tri[i].v1].z);
        glTexCoord2f(texturePoints[tri[i].v2].u, texturePoints[tri[i].v2].v);
        glVertex3f(vert[tri[i].v2].x, vert[tri[i].v2].y, vert[tri[i].v2].z);
        glEnd();
      }
    }

    void drawNVText(){
      glShadeModel(GL_SMOOTH);
      glBindTexture(GL_TEXTURE_2D, idText);
      for(int i=0; i<nT; i++){
        glBegin (GL_TRIANGLES);
        glNormal3f(normalV[tri[i].v0].x, normalV[tri[i].v0].y, normalV[tri[i].v0].z);
        glTexCoord2f(texturePoints[tri[i].v0].u, texturePoints[tri[i].v0].v);
        glVertex3f(vert[tri[i].v0].x, vert[tri[i].v0].y, vert[tri[i].v0].z);
        glNormal3f(normalV[tri[i].v1].x, normalV[tri[i].v1].y, normalV[tri[i].v1].z);
        glTexCoord2f(texturePoints[tri[i].v1].u, texturePoints[tri[i].v1].v);
        glVertex3f(vert[tri[i].v1].x, vert[tri[i].v1].y, vert[tri[i].v1].z);
        glNormal3f(normalV[tri[i].v2].x, normalV[tri[i].v2].y, normalV[tri[i].v2].z);
        glTexCoord2f(texturePoints[tri[i].v2].u, texturePoints[tri[i].v2].v);
        glVertex3f(vert[tri[i].v2].x, vert[tri[i].v2].y, vert[tri[i].v2].z);
        glEnd();
      }
    }
};

Malla cubo("modelos/cubo.ply", N_VERT);
Malla esfera("modelos/esfera.ply", N_VERT);

bool c = false;

class Cubo:Objeto3D
{
public:
  float lado;
  GLuint idText;
  float x, y, z;
Cubo(float lado){
  this->lado=lado;
}

Cubo(float lado, float x, float y, float z) : Cubo(lado){
  this->x = x;
  this->y = y;
  this->z = z;
}

void move(float x, float y, float z){
  this->x += x;
  this->y += y;
  this->z += z;
}

bool collision(float x, float y, float z){
  if(x > this->x && x < this->x+lado){
    if(y > this->y && y < this->y+lado){
      if(z > this->z && z < this->z+lado)
        return true;
    }
  }
  return false;
}

void asignarTextura(const char* nombre_arch){
  GLuint a;
  glGenTextures(1, &a);
  idText = a;
  glBindTexture(GL_TEXTURE_2D, idText);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  unsigned ancho;
  unsigned alto;
  unsigned char* img = LeerArchivoJPEG(nombre_arch, ancho, alto);

  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGB,
    ancho,
    alto,
    0,
    GL_RGB,
    GL_UNSIGNED_BYTE,
    img
  );
}

// Dibuja el objeto
void draw( ){
  //glDisable (GL_LIGHTING);

  glTranslatef(x, y, z);

  glBegin (GL_QUADS);
  {
    // Primer lado (derecho)
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(lado, 0, 0);
    glVertex3f(lado, lado, 0);
    glVertex3f(lado, lado, lado);
    glVertex3f(lado, 0, lado);
    // Segundo lado (frente)
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(lado, 0, lado);
    glVertex3f(lado, lado, lado);
    glVertex3f(0, lado, lado);
    glVertex3f(0, 0, lado);
    // Tercer lado (arriba)
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(lado, lado, 0);
    glVertex3f(0, lado, 0);
    glVertex3f(0, lado, lado);
    glVertex3f(lado, lado, lado);
    // Cuarto lado (izquierdo)
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(0, 0, lado);
    glVertex3f(0, lado, lado);
    glVertex3f(0, lado, 0);
    glVertex3f(0, 0, 0);
    // Quinto lado (fondo)
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, lado, 0);
    glVertex3f(lado, lado, 0);
    glVertex3f(lado, 0, 0);
    // Sexto lado (base)
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(lado, 0, lado);
    glVertex3f(0, 0, lado);
    glVertex3f(0, 0, 0);
    glVertex3f(lado, 0, 0);
  }
  glEnd();
  //glEnable (GL_LIGHTING);
}

void drawText( ){
  glTranslatef(x, y, z);
  //glDisable (GL_LIGHTING);
  //glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, idText);
  glBegin (GL_QUADS);
  {
    // Primer lado (derecho)
    glNormal3f(1.0, 0.0, 0.0);

    glTexCoord2f(1, 0.75);
    glVertex3f(lado, 0, 0);

    glTexCoord2f(1, 0.5);
    glVertex3f(lado, lado, 0);

    glTexCoord2f(0.75, 0.5);
    glVertex3f(lado, lado, lado);

    glTexCoord2f(0.75, 0.75);
    glVertex3f(lado, 0, lado);

    // Segundo lado (frente)
    glNormal3f(0.0, 0.0, 1.0);

    glTexCoord2f(0.75, 0.75);
    glVertex3f(lado, 0, lado);

    glTexCoord2f(0.75, 0.5);
    glVertex3f(lado, lado, lado);

    glTexCoord2f(0.5, 0.5);
    glVertex3f(0, lado, lado);

    glTexCoord2f(0.5, 0.75);
    glVertex3f(0, 0, lado);

    // Tercer lado (arriba)
    glNormal3f(0.0, 1.0, 0.0);

    glTexCoord2f(0.75, 0.25);
    glVertex3f(lado, lado, 0);

    glTexCoord2f(0.5, 0.25);
    glVertex3f(0, lado, 0);

    glTexCoord2f(0.5, 0.5);
    glVertex3f(0, lado, lado);

    glTexCoord2f(0.75, 0.5);
    glVertex3f(lado, lado, lado);

    // Cuarto lado (izquierdo)
    glNormal3f(-1.0, 0.0, 0.0);

    glTexCoord2f(0.5, 0.75);
    glVertex3f(0, 0, lado);

    glTexCoord2f(0.5, 0.5);
    glVertex3f(0, lado, lado);

    glTexCoord2f(0.25, 0.5);
    glVertex3f(0, lado, 0);

    glTexCoord2f(0.25, 0.75);
    glVertex3f(0, 0, 0);

    // Quinto lado (fondo)
    glNormal3f(0.0, 0.0, -1.0);

    glTexCoord2f(0.25, 0.75);
    glVertex3f(0, 0, 0);

    glTexCoord2f(0.25, 0.5);
    glVertex3f(0, lado, 0);

    glTexCoord2f(0, 0.5);
    glVertex3f(lado, lado, 0);

    glTexCoord2f(0, 0.75);
    glVertex3f(lado, 0, 0);

    // Sexto lado (base)
    glNormal3f(0.0, -1.0, 0.0);

    glTexCoord2f(0.75, 0.75);
    glVertex3f(lado, 0, lado);

    glTexCoord2f(0.5, 0.75);
    glVertex3f(0, 0, lado);

    glTexCoord2f(0.5, 1);
    glVertex3f(0, 0, 0);

    glTexCoord2f(0.75, 1);
    glVertex3f(lado, 0, 0);
  }
  glEnd();
  //glDisable(GL_TEXTURE_2D);
  //glEnable (GL_LIGHTING);
}

} ;

Cubo cubo1(5, 15, 0, 15);

class Piramide:Objeto3D
{
public:
  float lado;
  float alto;
Piramide(float lado, float alto){
  this->lado=lado;
  this->alto=alto;
}
// Dibuja el objeto
void draw( ){
  //glDisable (GL_LIGHTING);
  glBegin (GL_TRIANGLES);
  {
    // Primer lado (fondo)
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(-lado*0.5, 0.0, -lado*0.5);
    glVertex3f(0.0, alto, 0.0);
    glVertex3f(lado*0.5, 0.0, -lado*0.5);
    // Segundo lado (derecho)
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(lado*0.5, 0.0, -lado*0.5);
    glVertex3f(0.0, alto, 0.0);
    glVertex3f(lado*0.5, 0.0, lado*0.5);
    // Tercer lado (frente)
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(lado*0.5, 0.0, lado*0.5);
    glVertex3f(0.0, alto, 0.0);
    glVertex3f(-lado*0.5, 0.0, lado*0.5);
    // Cuarto lado (izquierdo)
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-lado*0.5, 0.0, -lado*0.5);
    glVertex3f(-lado*0.5, 0.0, lado*0.5);
    glVertex3f(0.0, alto, 0.0);
  }
  glEnd();
  glBegin(GL_QUADS);
  {
    // Quinto lado (base)
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(lado*0.5, 0.0, lado*0.5);
    glVertex3f(-lado*0.5, 0.0, lado*0.5);
    glVertex3f(-lado*0.5, 0.0, -lado*0.5);
    glVertex3f(lado*0.5, 0.0, -lado*0.5);
  }
  glEnd();
  //glEnable (GL_LIGHTING);
}
} ;

Piramide piramide1(4, 6);

class Ejes:Objeto3D 
{ 
public: 
    float longitud = 30;
// Dibuja el objeto
void draw( )
{
  glDisable (GL_LIGHTING);
  glBegin (GL_LINES);
  {
    glColor3f (0, 1, 0);
    glVertex3f (0, 0, 0);
    glVertex3f (0, longitud, 0);

    glColor3f (1, 0, 0);
    glVertex3f (0, 0, 0);
    glVertex3f (longitud, 0, 0);

    glColor3f (0, 0, 1);
    glVertex3f (0, 0, 0);
    glVertex3f (0, 0, longitud);
  }
  glEnd ();
  if(luz)
    glEnable (GL_LIGHTING);

}
} ; 

Ejes ejesCoordenadas;

void ColorSeleccion(int i){
  unsigned char r = (i & 0xFF);
  glColor3ub(r, 0, 0);
}

void segmento(){
  glPushMatrix();
    // esfera inferior del segmento
    glScalef(2, 2, 2);
    esfera.draw();
    glScalef(0.5, 0.5, 0.5);
    // como voy a escalar para hacer el cubo guardo la matriz
    glPushMatrix();
      // escalado para eu el cubo sea alargado
      glScalef(2, 10, 2);
      // subo para dibujar el cubo desde la parte superior en lugar de en el centro de la esfera
      glTranslatef(0, 0.5, 0);
      // dibujo el cubo
      cubo.draw();
      // recupero el estado antes de escalar
    glPopMatrix();
    // subo hasta la parte superiro del cubo de antes
    glTranslatef(0, 9.5, 0);
    // dibujo la otro esfera
    glScalef(2, 2, 2);
    esfera.draw();
    glScalef(0.5, 0.5, 0.5);
  glPopMatrix();
}

// solo dibuja la mitad del gancho, por eso se llama dos veces
void gancho(){
  glPushMatrix();
    // escalo el cubo para que sea mas fino en el eje z y alargado hacia arriba
    glScalef(1, 2, 0.5);
    // subo una unidad para que la parte inferior este en el origen 
    // y que las rotaciones se apliquen correctamente
    glTranslatef(0, 1, 0);
    glScalef(2, 2, 2);
    cubo.draw();
  glPopMatrix();
}

void coli(){
  if(cubo1.collision(objX, objY, objZ))
    c = true;
  else
    c = false;
}

// funcion que dibuja la pieza final, es la que instancia a las otras dos
void dibujar_pieza(){
  // Todos los if else se usan para calcular la rotación que tendra que tener 
  // en los ejes x y z en función de la rotación que tiene en el eje y
  float rx, rz;
  if(angY < 0)
    angY += 360;
  if(fmod(angY, 360)<90){
    rz = fmod(angY, 90)/90;
    rx = 1-rz;
  }
  else if(fmod(angY, 360)<180){
    rz = 1-(fmod(angY, 90)/90);
    rx = -(1-rz);
  }
  else if(fmod(angY, 360)<270){
    rz = -(fmod(angY, 90)/90);
    rx = -(1+rz);
  }
  else if(fmod(angY, 360)<360){
    rz = -(1-(fmod(angY, 90)/90));
    rx = (1+rz);
  }

  // se comienza a dibujar la pieza
  glPushMatrix();
    // estas transformaciones se aplican a todo lo que sigue
    glRotatef(angX, rx, 0, -rz);
    glRotatef(angY, 0, 1, 0);
    // esfera grande que actua como base para el brazo
    glScalef(5, 5, 5);
    ColorSeleccion(1);
    esfera.draw();
    glScalef(0.2, 0.2, 0.2);
    glTranslatef(0, 4, 0);
    // primer segmento
    segmento();
    
    glTranslatef(0, 11.5, 0);
    // desago la rotación del eje y para que los segmentos que no sean el primero 
    // queden siempre perpendiculares a la superficie
    glRotatef(-angY, 0, 1, 0);
    // aplico la rotación en los ejes x y z de este segmento especifico
    glRotatef(angX2, rx, 0, -rz);
    glRotatef(angY, 0, 1, 0);
    // dibujo una esfera que actua como unión de ambos segmentos
    ColorSeleccion(2);
    glScalef(3, 3, 3);
    esfera.draw();
    glScalef(0.3, 0.3, 0.3);
    glTranslatef(0, 2, 0);
    // segundo segmento
    segmento();

    glTranslatef(0, 11.5, 0);
    glRotatef(-angY, 0, 1, 0);
    // aplico el angulo de rotación concreto de este segmento
    // notese que no desago las rotaciones en x y en z anteriores, ya que estas afectan
    // al segemento actual
    glRotatef(angX3, rx, 0, -rz);
    glRotatef(angY, 0, 1, 0);
    // dibujo de nuevo la esfera de conexión
    ColorSeleccion(3);
    glScalef(3, 3, 3);
    esfera.draw();
    glScalef(0.3, 0.3, 0.3);
    glTranslatef(0, 2, 0);
    // tercer segmento
    segmento();

    glTranslatef(0, 10, 0);
    
    glPushMatrix();
      glRotatef(20, 1, 0, 0);
      glRotatef(-angX4, 1, 0, 0);
      glTranslatef(0, 0, 1);
      ColorSeleccion(4);
      // primer gancho
      gancho();
    glPopMatrix();

    glPushMatrix();
      glRotatef(-20, 1, 0, 0);
      glRotatef(angX4, 1, 0, 0);
      glTranslatef(0, 0, -1);
      ColorSeleccion(4);
      // segundo gancho
      gancho();
    glPopMatrix();

    if(angX4 > 34.5){
      float old_x = objX;
      float old_y = objY;
      float old_z = objZ;
			obtener_punto(trasnformaciones, 0, 0, 0);
      if(c)
        cubo1.move(objX-old_x, objY-old_y, objZ-old_z);
    }

  glPopMatrix();

  // glPointSize(5);
  // glBegin(GL_POINTS);
  //   glColor3f(1, 0, 0);
  //   glVertex3f(objX, objY, objZ);
  //   // std::cout << objX << ", " << objY << ", " << objZ << std::endl;
  // glEnd();
}

void imprime_matriz(float *matriz){
  std::cout << std::endl;
  for(int i=0; i<4; i++){
    std::cout << matriz[i] << ", " << matriz[i+4] << ", " << matriz[i+8] << ", " <<matriz[i+12] << std::endl;
  }
}

void transforma_punto(GLfloat* puntoLocal, GLfloat* matriz, GLfloat* puntoGlobal){
  for(int i=0; i<4; i++){
    puntoGlobal[i] = 0;
    for(int j=0; j<4; j++){
      puntoGlobal[i] += matriz[i+j*4]*puntoLocal[j];
    }
  }
}

void obtener_punto(void (*func)(), float x, float y, float z){
  glPushMatrix();
    // Se tiene que cargar la identidad y luego volver a aplicar todas las transformaciones
    // Eso es por que en GL_MODELVIEW_MATRIX se guardan tanto las transformaciones de los modelos como las de la vista
    glLoadIdentity();
    func();
    GLfloat transform[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, transform);
    // imprime_matriz(transform);
    float point[4] = {x, y, z, 1};
    float point2[4];
    transforma_punto(point, transform, point2);
    // std::cout << "punto: " << point2[0] << ", " << point2[1] << ", " << point2[2] << ", " << point2[3] << std::endl;
    objX = point2[0];
    objY = point2[1];
    objZ = point2[2];
  glPopMatrix();
}

void trasnformaciones(){
  float rx, rz;
  if(angY < 0)
    angY += 360;
  if(fmod(angY, 360)<90){
    rz = fmod(angY, 90)/90;
    rx = 1-rz;
  }
  else if(fmod(angY, 360)<180){
    rz = 1-(fmod(angY, 90)/90);
    rx = -(1-rz);
  }
  else if(fmod(angY, 360)<270){
    rz = -(fmod(angY, 90)/90);
    rx = -(1+rz);
  }
  else if(fmod(angY, 360)<360){
    rz = -(1-(fmod(angY, 90)/90));
    rx = (1+rz);
  }

  glRotatef(angX, rx, 0, -rz);
  glRotatef(angY, 0, 1, 0);
  glTranslatef(0, 15.5, 0);
  glRotatef(-angY, 0, 1, 0);
  glRotatef(angX2, rx, 0, -rz);
  glRotatef(angY, 0, 1, 0);
  glTranslatef(0, 13.5, 0);
  glRotatef(-angY, 0, 1, 0);
  glRotatef(angX3, rx, 0, -rz);
  glRotatef(angY, 0, 1, 0);
  if(angX3 < 180)
    glTranslatef(0, 2+(angX3*2/180), angX3/60);
  else
    glTranslatef(0, 2+((360-angX3)*2/180), -(360-angX3)/60);
  glTranslatef(0, 10, 0);
}

/**	void Dibuja( void )

Procedimiento de dibujo del modelo. Es llamado por glut cada vez que se debe redibujar.

**/

// float color2[4] = { 0, 0.7059, 0.8471, 1 };
// float color3[4] = { 0.2, 0.8, 0.3, 1 };

bool b = true;

void dibujoEscena(bool ver){
  static GLfloat  pos[4] = { 5.0, 5.0, 10.0, 0.0 };	// Posicion de la fuente de luz

  float  color[4] = { 1, 1, 1, 1 };

  glPushMatrix ();		// Apila la transformacion geometrica actual

  glClearColor (0.5, 0.5, 0.5, 1.0);	// Fija el color de fondo a un gris claro

  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Inicializa el buffer de color y el Z-Buffer

  transformacionVisualizacion ();	// Carga transformacion de visualizacion

  glLightfv (GL_LIGHT0, GL_POSITION, pos);	// Declaracion de luz. Colocada aqui esta fija en la escena

  if(ver)
    ejesCoordenadas.draw();			// Dibuja los ejes

  if(b){
    esfera.asignarTextura("texturas/agua.jpg");
    cubo.asignarTextura("texturas/magma.jpg");
    cubo1.asignarTextura("texturas/dado.jpg");
    b = false;
  }

  glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, color);

  if(ver)
    glEnable(GL_TEXTURE_2D);
  dibujar_pieza();
  cubo1.drawText();
  if(ver)
    glDisable(GL_TEXTURE_2D);

  glPopMatrix ();		// Desapila la transformacion geometrica
}

int pick(int x, int y, int *i){
  GLint viewport[4];
  unsigned char data[4];

  glGetIntegerv(GL_VIEWPORT, viewport);
  glDisable(GL_DITHER);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
  dibujoEscena(false);
  glEnable(GL_DITHER);
  glEnable(GL_LIGHTING);
  //glEnable(GL_TEXTURE_2D);
  glFlush();
  //glFinish();

  glReadPixels(x, viewport[3]-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

  *i = data[0];

  glutPostRedisplay();

  // std::cout << "ID del objeto seleccionado: " << *i << std::endl;
  return *i;
}

void Dibuja (void)
{
  glEnable(GL_LIGHTING);
  glEnable(GL_DITHER);

  dibujoEscena(true);

  glutSwapBuffers ();		// Intercambia el buffer de dibujo y visualizacion
}

/**	void idle()

Procedimiento de fondo. Es llamado por glut cuando no hay eventos pendientes.

**/

void idle (int v)
{
  if(animacion){
    if(!volver){
      if(angY < 90){
        angY += velY;
        if(angY > 90)
          angY = 90;
      }
      else if(angY == 90 && angX < 30){
        angX += velX;
        if(angX > 30)
          angX = 30;
      }
      else if(angY == 90 && angX2 < 30){
        angX2 += velX2;
        if(angX2 > 30)
          angX2 = 30;
      }
      else if(angY == 90 && angX3 < 30){
        angX3 += velX3;
        if(angX3 > 30)
          angX3 = 30;
      }
      else if(angY == 90 && angX4 < 35){
        angX4 += velX4;
        if(angX4 > 35)
          angX4 = 35;
      }
      else if(angY == 90 && angX4 == 35)
        volver = true;
    } 
    else{
      if(angY == 90 && angX4 > 0){
        angX4 -= velX4;
        if(angX4 < 0)
          angX4 = 0;
      }
      else if(angY == 90 && angX3 > 0){
        angX3 -= velX3;
        if(angX3 < 0)
          angX3 = 0;
      }
      else if(angY == 90 && angX2 > 0){
        angX2 -= velX2;
        if(angX2 < 0)
          angX2 = 0;
      }
      else if(angY == 90 && angX > 0){
        angX -= velX;
        if(angX < 0)
          angX = 0;
      }
      else if(angY <= 90 && angY > 0){
        angY -= velY;
        if(angY < 0)
          angY = 0;
      }
      else if(angY == 0 && angX < 30){
        angX += velX;
        if(angX > 30)
          angX = 30;
      }
      else if(angY == 0 && angX2 < 30){
        angX2 += velX2;
        if(angX2 > 30)
          angX2 = 30;
      }
      else if(angY == 0 && angX3 < 30){
        angX3 += velX3;
        if(angX3 > 30)
          angX3 = 30;
      }
      else if(angY == 0 && angX4 < 35){
        angX4 += velX4;
        if(angX4 > 35)
          angX4 = 35;
      }
      else if(angY == 0 && angX4 == 35)
        volver = false;
    }
  }
  glutPostRedisplay ();		// Redibuja
  glutTimerFunc (30, idle, 0);	// Vuelve a activarse dentro de 30 ms
}
