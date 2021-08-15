#include <vector>
#include <cassert>
#include "../../Drawing++.hpp" //include Drawing++ header file

//user-defined shape function
static int shape_square_filled(Drawing::Drawable* drawable, png_uint_32 x, png_uint_32 y){
    assert(drawable->points.size() >= 2);
    
    return x >= drawable->points[0].x && x <= drawable->points[1].x &&
            y >= drawable->points[0].y && y <= drawable->points[1].y;
}


int main(){
    Drawing::Canvas canvas(640, 480); //Create Drawing::Canvas 640x480 size

    canvas.addDrawable(new Drawing::Figure(
        Drawing::Color(1.0, 0.0, 0.0, 1.0),
        shape_square_filled,
        std::vector<Drawing::Point>{ 
            Drawing::Point(370, 280), Drawing::Point(450, 400) 
        }
    )); 

    canvas.addDrawable(new Drawing::Figure(
        Drawing::Color(0.0, 1.0, 0.0, 0.5),
        shape_square_filled,
        std::vector<Drawing::Point>{ 
            Drawing::Point(370+20, 280+20), Drawing::Point(450+20, 400+20) 
        }
    ));

    canvas.addDrawable(new Drawing::Figure(
        Drawing::Color(0.0, 0.0, 1.0, 1.0),
        shape_square_filled,
        std::vector<Drawing::Point>{ 
            Drawing::Point(370+40, 280+40), Drawing::Point(450+40, 400+40) 
        }
    )); 

    canvas.draw(); //draw every drawable in canvas to buffer
    canvas.bufferToFile("./output.png"); //output buffer to file "./output.png"
    
    return 0;
}