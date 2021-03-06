#include <vector>
#include <cassert>
#include <chrono>
#include <iostream>
#include "../../Drawing++.hpp" //include Drawing++ header file

//user-defined draw function
static void filled_rect_func(Drawing::Drawable *drawable, Drawing::Canvas* canvas){
    const Drawing::Color pixel = drawable->getPixel(0, 0);
    canvas->fillputPixels(drawable->points[0].x(), drawable->points[1].x(), 
        drawable->points[0].y(), drawable->points[1].y(), pixel);
}


int main(){
    Drawing::Canvas canvas(640, 480); //Create Drawing::Canvas 640x480 size

    canvas.addDrawable(Drawing::Figure(
        Drawing::Color(1.0, 0.0, 0.0, 1.0),
        filled_rect_func,
        std::vector<Drawing::Point>{ 
            Drawing::Point({370, 280}), Drawing::Point({450, 400}) 
        }
    ));

    canvas.addDrawable(Drawing::Figure(
        Drawing::Color(0.0, 1.0, 0.0, 0.5),
        filled_rect_func,
        std::vector<Drawing::Point>{ 
            Drawing::Point({370+20, 280+20}), Drawing::Point({450+20, 400+20}) 
        }
    ));

    canvas.addDrawable(Drawing::Figure(
        Drawing::Color(0.0, 0.0, 1.0, 1.0),
        filled_rect_func,
        std::vector<Drawing::Point>{ 
            Drawing::Point({370+40, 280+40}), Drawing::Point({450+40, 400+40}) 
        }
    )); 

    canvas.draw(); //draw every drawable in canvas to buffer
    canvas.bufferToFile("./output.png"); //output buffer to file "./output.png"

    return 0;
}