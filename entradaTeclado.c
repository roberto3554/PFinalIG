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
modulo entradaTeclado.c
	Gestion de eventos de teclado
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>		// Libreria de utilidades de OpenGL
#include "practicasIG.h"


/** 

Imprime en la consola las instrucciones del programa

**/
void printHelp ()
{

  printf ("\n\n     Prácticas de Informática Gráfica	Curso 2023-2024");
  printf ("\n\n Dpto. Lenguajes y Sistemas Informaticos");
  printf ("\n E.T.S.I. Informatica		Univ. de Granada ");
  printf ("\n");
  printf ("\n Opciones: \n\n");
  printf ("h, H: Imprime informacion de ayuda \n");
  printf ("PgUp, PgDn: avanza y retrocede la cámara \n\n");
  printf ("+,-: avanza y retrocede la cámara \n\n");
  printf ("Teclas de movimiento de cursor: giran la camara\n");
  // Anyade la informacion de las opciones que introduzcas aqui !!       

  printf ("\n Escape: Salir");
  printf ("\n\n\n");
}




/* @teclado ---------------------------------------------------------------- */

/** 		void letra (unsigned char k, int x, int y)

Este procedimiento es llamado por el sistema cuando se pulsa una tecla normal
El codigo k es el ascii de la letra

Para anyadir nuevas ordenes de teclado coloca el correspondiente case.

Parametros de entrada:

k: codigo del caracter pulsado

x:

y:

**/

float rotxCamara = 30, rotyCamara = 45;
float dCamara = 10;

void letra (unsigned char k, int x, int y)
{

  switch (k)
    {
    // case 'h':
    // case 'H':
    //   printHelp ();		// H y h imprimen ayuda
    //   break;
    case '+':			// acerca la cámara
      dCamara -= 5.0;
      break;
    case '-':			// aleja la cámara
      dCamara += 5.0;
      break;
    case 'i':
    case 'I':
      luz = !luz;
      if(luz)
        glEnable(GL_LIGHTING);
      else
        glDisable(GL_LIGHTING);
      break;
    case 'C':
      angY--;
      break;
    case 'c':
      angY++;
      break;
    case 'V':
      angX--;
      break;
    case 'v':
      angX++;
      break;
    case 'B':
      angX2--;
      break;
    case 'b':
      angX2++;
      break;
    case 'N':
      angX3--;
      break;
    case 'n':
      angX3++;
      break;
    case 'M':
      if(angX4>0)
        angX4--;
      break;
    case 'm':
      if(angX4<35)
        angX4++;
      break;
    case 'a':
    case 'A':
      if(!animacion){
        angY  = 0.0;
        angX  = 0.0;
        angX2 = 0.0;
        angX3 = 0.0;
        angX4 = 0.0;
        volver = true;
      }
      animacion = !animacion;
      break;
    case 'F':
      velY-=0.5;
      if(velY<0)
        velY = 0;
      break;
    case 'f':
      velY+=0.5;
      break;
    case 'G':
      velX-=0.5;
      if(velX<0)
        velX = 0;
      break;
    case 'g':
      velX+=0.5;
      break;
    case 'H':
      velX2-=0.5;
      if(velX2<0)
        velX2 = 0;
      break;
    case 'h':
      velX2+=0.5;
      break;
    case 'K':
      velX3-=0.5;
      if(velX3<0)
        velX3 = 0;
      break;
    case 'k':
      velX3+=0.5;
      break;
    case 'L':
      velX4-=0.5;
      if(velX4<0)
        velX4 = 0;
      break;
    case 'l':
      velX4+=0.5;
      break;
    case 27:			// Escape  Terminar
      exit (0);
    default:
      return;
    }
  setCamara (rotxCamara, rotyCamara, 0.0, dCamara);
  glutPostRedisplay ();		// Algunas de las opciones cambian paramentros
}				// de la camara. Es necesario actualziar la imagen

/**		void especial(int k, int x, int y)
Este procedimiento es llamado por el sistema cuando se pulsa una tecla
especial. El codigo k esta definido en glut mediante constantes

Parametros de entrada:

k: codigo del caracter pulsado (definido en glut mediante constantes).

x:

y:

**/
void especial (int k, int x, int y)
{

  switch (k)
    {
    case GLUT_KEY_UP:
      rotxCamara += 5.0;	// Cursor arriba + rotacion x
      if (rotxCamara > 360)
	rotxCamara -= 360;
      break;
    case GLUT_KEY_DOWN:
      rotxCamara -= 5.0;
      if (rotxCamara < 0)
	rotxCamara += 360;
      break;
    case GLUT_KEY_LEFT:
      rotyCamara += 5.0;
      if (rotyCamara > 360)
	rotyCamara -= 360;
      break;
    case GLUT_KEY_RIGHT:
      rotyCamara -= 5.0;
      if (rotyCamara < 0)
	rotyCamara += 360;
      break;
    case GLUT_KEY_PAGE_DOWN:	// acerca la cámara
      dCamara -= 5.0;
      break;
    case GLUT_KEY_PAGE_UP:	// aleja la cámara
      dCamara += 5.0;
      break;
    default:
      return;
    }
  setCamara (rotxCamara, rotyCamara, 0.0, dCamara);
  glutPostRedisplay ();		// Actualiza la imagen (ver proc. letra)
}
