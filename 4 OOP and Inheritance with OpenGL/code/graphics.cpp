// $Id: graphics.cpp,v 1.11 2014-05-15 16:42:55-07 - - $

#include <iostream>
using namespace std;

#include <GL/freeglut.h>

#include "graphics.h"
#include "util.h"

int window::width = 640; // in pixels
int window::height = 480; // in pixels
vector<object> window::objects;
size_t window::selected_obj = 0;
string window::border_color = "red";
int window::border_thickness = 4;
int window::moveby = 4;
size_t window::next_num = 0;
mouse window::mus;
bool window::draw_border = false;

//object::draw
void object::draw() {
   //draw a border;
   //draw the object itself;
   pshape->draw (center, color);

   //draw the object number;
   if (get_obj_num() < 10 and 0 <= (signed) get_obj_num()) {
      string s = "";
      s += (get_obj_num() + '0');
      //cout << "object::draw(): string s: " << s << endl;
      const GLubyte WHITE[] = {255, 255, 255};
      glColor3ubv(WHITE);
      glRasterPos2i(center.xpos, center.ypos);
      glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, reinterpret_cast<const unsigned char*>(s.c_str()));
   }
}

//objects move function
void object::move (GLfloat delta_x, GLfloat delta_y){
	center.xpos += delta_x;
	center.ypos += delta_y;
	
	if(center.xpos > window::getwidth() ) {center.xpos = 0; } //goes past the right edge
	if(center.xpos < 0) {center.xpos = window::getwidth(); } //goes past the left edge
	if(center.ypos > window::getheight()) {center.ypos = 0; } //goes past the top edge
	if(center.ypos < 0) {center.ypos = window::getheight(); } //goes past the bottom edge
}

//object constructor
object::object (const shared_ptr<shape>& the_shape, vertex& the_vertex, 
        rgbcolor& color) : pshape(the_shape), center(the_vertex), color(color) {
}

// Executed when window system signals to shut down.
void window::close() {
   DEBUGF ('g', sys_info::execname() << ": exit ("
           << sys_info::exit_status() << ")");
   exit (sys_info::exit_status());
}

// Executed when mouse enters or leaves window.
void window::entry (int mouse_entered) {
   DEBUGF ('g', "mouse_entered=" << mouse_entered);
   window::mus.entered = mouse_entered;
   if (window::mus.entered == GLUT_ENTERED) {
      DEBUGF ('g', sys_info::execname() << ": width=" << window::width
           << ", height=" << window::height);
   }
   glutPostRedisplay();
}

// Called to display the objects in the window.
void window::display() {
   glClear (GL_COLOR_BUFFER_BIT);
   int i = 0;
   for (auto& object: window::objects){
	   draw_border = false;
	   if(i == (signed) selected_obj)
		   draw_border = true;
      object.draw();
	  i++;
   }
   i=0;
   mus.draw();
   glutSwapBuffers();
}

// Called when window is opened and when resized.
void window::reshape (int width, int height) {
   DEBUGF ('g', "width=" << width << ", height=" << height);
   window::width = width;
   window::height = height;
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D (0, window::width, 0, window::height);
   glMatrixMode (GL_MODELVIEW);
   glViewport (0, 0, window::width, window::height);
   glClearColor (0.25, 0.25, 0.25, 1.0);
   glutPostRedisplay();
}


// Executed when a regular keyboard key is pressed.
enum {BS=8, TAB=9, ESC=27, SPACE=32, DEL=127};
void window::keyboard (GLubyte key, int x, int y) {
   DEBUGF ('g', "key=" << (unsigned)key << ", x=" << x << ", y=" << y);
   window::mus.set (x, y);
   switch (key) {
      case 'Q': case 'q': case ESC:
         window::close();
         break;
      case 'H': case 'h':
         move_selected_object(-1, 0);
		 //cout<<"KEY: "<<curr_obj()<<endl;
         break;
      case 'J': case 'j':
         move_selected_object(0, -1);
		 //cout<<"KEY: "<<curr_obj()<<endl;
         break;
      case 'K': case 'k':
         move_selected_object(0, 1);
		 //cout<<"KEY: "<<curr_obj()<<endl;
         break;
      case 'L': case 'l':
         move_selected_object(1, 0);
		 //cout<<"KEY: "<<curr_obj()<<endl;
         break;
      case 'N': case 'n': case SPACE: case TAB:
		 if(objects.size() < 9)
			 selected_obj = (selected_obj + 1)%objects.size();
		 else
			 selected_obj = (selected_obj + 1)%9;
         break;
      case 'P': case 'p': case BS:
		 if(objects.size() < 9)
			 selected_obj = (selected_obj + objects.size() - 1)%objects.size();
		 else
			 selected_obj = (selected_obj - 1)%9;
         break;
      case '0'...'9':
		   if( (unsigned)(key - '0') >= window::objects.size() )
			   cerr<<"NO OBJECT ASSIGNED TO KEY"<<endl;
	      else selected_obj = key - '0';
         break;
      default:
         cerr << (unsigned char)key << ": invalid keystroke" << endl;
         break;
   }
   glutPostRedisplay();
}


// Executed when a special function key is pressed.
void window::special (int key, int x, int y) {
   DEBUGF ('g', "key=" << key << ", x=" << x << ", y=" << y);
   window::mus.set (x, y);
   switch (key) {
      case GLUT_KEY_LEFT: move_selected_object (-1, 0); break;
      case GLUT_KEY_DOWN: move_selected_object (0, -1); break;
      case GLUT_KEY_UP: move_selected_object (0, +1); break;
      case GLUT_KEY_RIGHT: move_selected_object (+1, 0); break;
      case GLUT_KEY_F1: //select_object (1); break;
      case GLUT_KEY_F2: //select_object (2); break;
      case GLUT_KEY_F3: //select_object (3); break;
      case GLUT_KEY_F4: //select_object (4); break;
      case GLUT_KEY_F5: //select_object (5); break;
      case GLUT_KEY_F6: //select_object (6); break;
      case GLUT_KEY_F7: //select_object (7); break;
      case GLUT_KEY_F8: //select_object (8); break;
      case GLUT_KEY_F9: //select_object (9); break;
      case GLUT_KEY_F10: //select_object (10); break;
      case GLUT_KEY_F11: //select_object (11); break;
      case GLUT_KEY_F12: //select_object (12); break;
      default:
         cerr << (unsigned)key << ": invalid function key" << endl;
         break;
   }
   glutPostRedisplay();
}


void window::motion (int x, int y) {
   DEBUGF ('g', "x=" << x << ", y=" << y);
   window::mus.set (x, y);
   glutPostRedisplay();
}

void window::passivemotion (int x, int y) {
   DEBUGF ('g', "x=" << x << ", y=" << y);
   window::mus.set (x, y);
   glutPostRedisplay();
}

void window::mousefn (int button, int state, int x, int y) {
   DEBUGF ('g', "button=" << button << ", state=" << state
           << ", x=" << x << ", y=" << y);
   window::mus.state (button, state);
   window::mus.set (x, y);
   glutPostRedisplay();
}

void window::move_selected_object(int xdir, int ydir) {
        objects.at(selected_obj).move(xdir * moveby, ydir * moveby);
}

void window::set_border(string s, int thickness){
	border_color = s;
	border_thickness = thickness;
}

void window::main () {
   static int argc = 0;
   glutInit (&argc, nullptr);
   glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE);
   glutInitWindowSize (window::width, window::height);
   glutInitWindowPosition (128, 128);
   glutCreateWindow (sys_info::execname().c_str());
   glutCloseFunc (window::close);
   glutEntryFunc (window::entry);
   glutDisplayFunc (window::display);
   glutReshapeFunc (window::reshape);
   glutKeyboardFunc (window::keyboard);
   glutSpecialFunc (window::special);
   glutMotionFunc (window::motion);
   glutPassiveMotionFunc (window::passivemotion);
   glutMouseFunc (window::mousefn);
   DEBUGF ('g', "Calling glutMainLoop()");
   glutMainLoop();
}


void mouse::state (int button, int state) {
   switch (button) {
      case GLUT_LEFT_BUTTON: left_state = state; break;
      case GLUT_MIDDLE_BUTTON: middle_state = state; break;
      case GLUT_RIGHT_BUTTON: right_state = state; break;
   }
}

void mouse::draw() {
   static rgbcolor color ("green");
   ostringstream text;
   text << "(" << xpos << "," << window::height - ypos << ")";
   if (left_state == GLUT_DOWN) text << "L"; 
   if (middle_state == GLUT_DOWN) text << "M"; 
   if (right_state == GLUT_DOWN) text << "R"; 
   if (entered == GLUT_ENTERED) {
      void* font = GLUT_BITMAP_HELVETICA_18;
      glColor3ubv (color.ubvec);
      glRasterPos2i (10, 10);
      glutBitmapString (font, (GLubyte*) text.str().c_str());
   }
}

