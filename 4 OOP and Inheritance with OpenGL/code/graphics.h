// $Id: graphics.h,v 1.9 2014-05-15 16:42:55-07 - - $

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <memory>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "rgbcolor.h"
#include "shape.h"

class object {
   private:
      shared_ptr<shape> pshape;
      vertex center;
      rgbcolor color;
	  size_t obj_num = 0;
   public:
      // Default copiers, movers, dtor all OK.
      object(const shared_ptr<shape>&, vertex&, rgbcolor&);
      void draw();
      void move (GLfloat delta_x, GLfloat delta_y);
	  void set_obj_num(size_t x){obj_num = x;}
	  size_t get_obj_num(){return obj_num;}
};

class mouse {
      friend class window;
   private:
      int xpos {0};
      int ypos {0};
      int entered {GLUT_LEFT};
      int left_state {GLUT_UP};
      int middle_state {GLUT_UP};
      int right_state {GLUT_UP};
   private:
      void set (int x, int y) { xpos = x; ypos = y; }
      void state (int button, int state);
      void draw();
};


class window {
      friend class mouse;
   private:
      static int width;         // in pixels
      static int height;        // in pixels
      static vector<object> objects;
      static size_t selected_obj;
	  static string border_color;
	  static int border_thickness;
	  static int moveby;
      static mouse mus;
   private:
	  static size_t next_num;
      static void close();
      static void entry (int mouse_entered);
      static void display();
      static void reshape (int width, int height);
      static void keyboard (GLubyte key, int, int);
      static void special (int key, int, int);
      static void motion (int x, int y);
      static void passivemotion (int x, int y);
      static void mousefn (int button, int state, int x, int y);
   public:
      static void push_back (const object& obj) {
                  objects.push_back (obj); }
      static void setwidth (int width_) { width = width_; }
      static void setheight (int height_) { height = height_; }
      static void main();
      static int getwidth() {return width;}
      static int getheight() {return height;}
	  static size_t curr_obj(){return selected_obj; }
	  static void move_selected_object(int dx, int dy);
	  static size_t get_selected_obj(){return selected_obj;}
	  static void set_moveby(int dx) {moveby = dx;}
	  static void set_border(string color, int thickness);
	  static int get_border_thickness(){return border_thickness;}
	  static string get_border_color(){return border_color;}
	  static size_t get_next_num(){size_t n = next_num++; return n;}
	  static bool draw_border;
};

#endif

