// $Id: interp.cpp,v 1.18 2015-02-19 16:50:37-08 - - $

#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

unordered_map<string,interpreter::interpreterfn>
interpreter::interp_map {
   {"define" , &interpreter::do_define },
   {"draw"   , &interpreter::do_draw   },
   {"border" , &interpreter::do_border},
   {"moveby", &interpreter::do_moveby},
};

unordered_map<string,interpreter::factoryfn>
interpreter::factory_map {
   {"text"          , &interpreter::make_text             },
   {"ellipse"       , &interpreter::make_ellipse          },
   {"circle"        , &interpreter::make_circle           },
   {"polygon"       , &interpreter::make_polygon          },
   {"rectangle"     , &interpreter::make_rectangle        },
   {"square"        , &interpreter::make_square           },
   {"diamond"       , &interpreter::make_diamond          },
   {"triangle"      , &interpreter::make_triangle         },
   {"right_triangle", &interpreter::make_right_triangle   },
   {"isosceles"     , &interpreter::make_isosceles        },
   {"equilateral"   , &interpreter::make_equilateral      }
};


interpreter::shape_map interpreter::objmap;

interpreter::~interpreter() {
   for (const auto& itor: objmap) {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret (const parameters& params) {
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error MEOW");
   interpreterfn func = itor->second;
   func (++begin, params.cend());
}

void interpreter::do_define (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.emplace (name, make_shape (++begin, end));
}


void interpreter::do_draw (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   /*
   cout << "name will be: " << begin[1] << endl;
   cout << "vertex 1 will be: " << begin[2] << endl;
   cout << "vertex 2 will be: " << begin[3] << endl;
   cout << "color will be: " << begin[0] << endl;
   */
   if (end - begin != 4) throw runtime_error ("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) {
      throw runtime_error (name + ": no such shape");
   }
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   rgbcolor color {begin[0]};
   object temp = object(itor->second, where, color);
   temp.set_obj_num(window::get_next_num());
   window::push_back(temp);
}

void interpreter::do_border(param begin, param end){
	if(end-begin != 2) throw runtime_error("syntax error");
	rgbcolor color {*begin++};
	window::set_border(color, (stoi)(*begin));
}

void interpreter::do_moveby(param begin, param end){
	if(end-begin != 1) throw runtime_error("syntax error");
	window::set_moveby((stoi)(begin[0]));
}

shape_ptr interpreter::make_shape (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) {
      throw runtime_error (type + ": no such shape");
   }
   factoryfn func = itor->second;
   return func (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string s = "";
   auto itor = fontcode.find(*begin);
   if (itor != fontcode.end()) {
        begin++;
        string s = "";
        string space = "";
        while (begin != end) {
            s += space;
            s += *begin;
            space = " ";
            begin++;
        }
        //cout << "make_text::string s: " << s << endl;
        return make_shared<text> (itor->second, s);
   } else {
       throw runtime_error (*begin + ": no such font");
   }
}

shape_ptr interpreter::make_ellipse (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   GLfloat width = GLfloat(stof(*begin++));
   GLfloat height = GLfloat(stof(*begin));
   return make_shared<ellipse> (width, height);
}

shape_ptr interpreter::make_circle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   //GLfloat() diameter = (GLfloat())
   //stoi(*begin, nullptr, int base = 10);
   return make_shared<circle> (GLfloat(stof(*begin)));
}

shape_ptr interpreter::make_polygon (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (begin == end) throw runtime_error( "EMPTY VERTEX LIST!");
   if ((end - begin) % 2 != 0) throw runtime_error( "ODD AMOUNT OF ENTRIES IN VERTEX LIST");
   vertex_list vL;
   while (begin != end){
     GLfloat left = GLfloat(stof(*begin++));
     GLfloat right = GLfloat(stof(*begin++));
     vL.push_back({left, right});
   }
   return make_shared<polygon> (vL);
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   GLfloat width = GLfloat(stof(*begin++));
   GLfloat height = GLfloat(stof(*begin));
   vertex_list vertices;
   vertices.push_back({GLfloat((0.0 - width)/2.0), GLfloat((0.0 - height)/2.0)}); //bottom-left
   vertices.push_back({GLfloat(width/2.0), GLfloat((0.0 - height)/2.0)}); //bottom-right
   vertices.push_back({GLfloat(width/2.0), GLfloat(height/2.0)}); //top-right
   vertices.push_back({GLfloat((0.0 - width)/2.0), GLfloat(height/2.0)}); //top-left
   return make_shared<rectangle> (vertices);
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   GLfloat width = GLfloat(stof(*begin));
   vertex_list vertices;
   vertices.push_back({GLfloat((0.0 - width)/2.0), GLfloat((0.0 - width)/2.0)}); 
   vertices.push_back({GLfloat(width/2.0), GLfloat((0.0 - width)/2.0)});
   vertices.push_back({GLfloat(width/2.0), GLfloat(width/2.0)});
   vertices.push_back({GLfloat((0.0 - width)/2.0), GLfloat(width/2.0)});
   return make_shared<square> (vertices);
}

shape_ptr interpreter::make_diamond (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   GLfloat width = GLfloat(stof(*begin++));
   GLfloat height = GLfloat(stof(*begin));
   vertex_list vertices;
   vertices.push_back({0, GLfloat(height/2.0)}); //top
   vertices.push_back({GLfloat((0.0 - width)/2.0), 0}); //left
   vertices.push_back({0, GLfloat((0.0 - height)/2.0)}); //bottom
   vertices.push_back({GLfloat((width/2.0)), 0}); //right
   return make_shared<diamond> (vertices);
}

shape_ptr interpreter::make_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   vertex_list vertices;
   while (begin != end){
     GLfloat left = GLfloat(stof(*begin++));
     GLfloat right = GLfloat(stof(*begin++));
     vertices.push_back({left, right});
   }
   return make_shared<triangle> (vertices);
}

shape_ptr interpreter::make_isosceles (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   GLfloat width = GLfloat(stof(*begin++));
   GLfloat height = GLfloat(stof(*begin));
   vertex_list vertices;
   vertices.push_back({GLfloat((0.0 - width)/2.0), 0}); //left corner of base
   vertices.push_back({0, height}); //top
   vertices.push_back({GLfloat(width/2.0), 0}); //right corner of base
   return make_shared<isosceles> (vertices);
}

shape_ptr interpreter::make_equilateral (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   GLfloat height = GLfloat(stof(*begin));
   vertex_list vertices;
   vertices.push_back({GLfloat((0.0 - height)/2.0), 0});
   vertices.push_back({0, height});
   vertices.push_back({GLfloat(height/2.0), 0});
   return make_shared<equilateral> (vertices);
}

shape_ptr interpreter::make_right_triangle (param begin, param end) {
  DEBUGF ('f', range (begin, end));
  GLfloat width = GLfloat(stof(*begin++));
  GLfloat height = GLfloat(stof(*begin));
  vertex_list vertices;
  vertices.push_back({width, 0});
  vertices.push_back({0,0});
  vertices.push_back({0, height});
  return make_shared<right_triangle> (vertices);
}









