// $Id: shape.cpp,v 1.7 2014-05-08 18:32:56-07 - - $

#include <typeinfo>
#include <unordered_map>
#include <cmath>
using namespace std;

#include "shape.h"
#include "util.h"
#include "graphics.h"
#include <GL/freeglut.h>


static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};


ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font, const string& textdata):
      glut_bitmap_font(glut_bitmap_font), textdata(textdata) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices): vertices(vertices) {
   DEBUGF ('c', this);
}

rectangle::rectangle (const vertex_list& vertices): polygon(vertices) {
   DEBUGF ('c', this);
}

square::square (const vertex_list& vertices): rectangle (vertices) {
   DEBUGF ('c', this);
}

diamond::diamond (const vertex_list& vertices): polygon(vertices) {
    DEBUGF ('c', this);
}

triangle::triangle (const vertex_list& vertices): polygon(vertices) {
    DEBUGF ('c', this);
}

isosceles::isosceles (const vertex_list& vertices): triangle(vertices) {
    DEBUGF ('c', this);
}

equilateral::equilateral (const vertex_list& vertices): triangle(vertices) {
    DEBUGF ('c', this);
}

right_triangle::right_triangle (const vertex_list& vertices): triangle(vertices) {
    DEBUGF ('c', this);
}

void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   rgbcolor thiscolor = color;
   glColor3ubv(thiscolor.ubvec3());   
   float xpos = window::getwidth()/2.0 + center.xpos;
   float ypos = window::getheight()/2.0 + center.ypos;
   glRasterPos2f(xpos, ypos);
   glutBitmapString(glut_bitmap_font, 
      reinterpret_cast<const unsigned char*>(textdata.c_str()));
    
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   DEBUGF ('d', "dimension: " << dimension);
   // dimension has the width and height GLfloats
   // center has the position GLfloats
   glBegin(GL_POLYGON);
   glEnable (GL_LINE_SMOOTH);
   glColor3ubv(color.ubvec);
   const float delta = 2 * M_PI / 32;
   for (float theta = 0; theta < 2 * M_PI; theta += delta) {
       float xpos = dimension.xpos/2 * cos(theta) + center.xpos;
       float ypos = dimension.ypos/2 * sin(theta) + center.ypos;
       glVertex2f(xpos, ypos);
   }
   glEnd();

	if(window::draw_border){
	   glLineWidth(window::get_border_thickness());
	   glBegin(GL_LINE_LOOP);
	   glEnable (GL_LINE_SMOOTH);
	   glColor3ubv(rgbcolor(window::get_border_color()).ubvec);
	   const float delta = 2 * M_PI / 32;
	   for (float theta = 0; theta < 2 * M_PI; theta += delta) {
		   float xpos = dimension.xpos/2 * cos(theta) + center.xpos;
		   float ypos = dimension.ypos/2 * sin(theta) + center.ypos;
		   glVertex2f(xpos, ypos);
	   }
	   glEnd();
	}

}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
   GLfloat avg_x;
   GLfloat avg_y;
   int i = 0;
   for (vertex v_iter : vertices) {
       avg_x *= (i + v_iter.xpos)/(i + 1);
       avg_y *= (i + v_iter.ypos)/(i + 1);
       i++;
   }
   glBegin(GL_POLYGON);
   glColor3ubv(color.ubvec);
   for (vertex v_iter : vertices) {
       GLfloat x = center.xpos + v_iter.xpos - avg_x;
       GLfloat y = center.ypos + v_iter.ypos - avg_y;
       glVertex2f(x,y);
   }
   glEnd();
   
   	if(window::draw_border){
	   glLineWidth(window::get_border_thickness());
	   glBegin(GL_LINE_LOOP);
	   glEnable (GL_LINE_SMOOTH);
	   glColor3ubv(rgbcolor(window::get_border_color()).ubvec);
	   for (vertex v_iter : vertices) {
		   GLfloat x = center.xpos + v_iter.xpos - avg_x;
		   GLfloat y = center.ypos + v_iter.ypos - avg_y;
		   glVertex2f(x,y);
	   }
	   glEnd();
	}
   
}

void rectangle::draw (const vertex& center, const rgbcolor& color) const {
  polygon::draw(center, color);
}

void square::draw (const vertex& center, const rgbcolor& color) const {
  polygon::draw(center, color);
}

void diamond::draw (const vertex& center, const rgbcolor& color) const {
  polygon::draw(center, color);
}

void triangle::draw (const vertex& center, const rgbcolor& color) const {
  polygon::draw(center, color);
}

void isosceles::draw (const vertex& center, const rgbcolor& color) const {
  polygon::draw(center, color);
}

void equilateral::draw (const vertex& center, const rgbcolor& color) const {
  polygon::draw(center, color);
}

void right_triangle::draw (const vertex& center, const rgbcolor& color) const {
  polygon::draw(center, color);
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

