#include <vector>
#include <cassert>
#include "../Drawing++.hpp"

 
static int shape_square_filled(const Drawing::Drawable *drawable, double x, double y){
    assert(drawable->points.size() >= 2);
    return x >= drawable->points[0].x && x <= drawable->points[1].x &&
            y >= drawable->points[0].y && y <= drawable->points[1].y;
}


int main(){
    Drawing::Canvas canvas(640, 480);
    
    canvas.addDrawable(Drawing::Drawable(
        new Drawing::Color(1.0, 0.0, 0.0, 1.0),
        shape_square_filled, 0.0,
        std::vector<Drawing::Point>{ 
            Drawing::Point(370, 280), Drawing::Point(450, 400) 
        }
    ));

    canvas.addDrawable(Drawing::Drawable(
        new Drawing::Color(0.0, 1.0, 0.0, 0.5),
        shape_square_filled, 0.0,
        std::vector<Drawing::Point>{ 
            Drawing::Point(370+20, 280+20), Drawing::Point(450+20, 400+20)
        }
    ));

    canvas.addDrawable(Drawing::Drawable(
        new Drawing::Color(0.0, 0.0, 1.0, 1.0),
        shape_square_filled, 0.0,
        std::vector<Drawing::Point>{ 
            Drawing::Point(370+40, 280+40), Drawing::Point(450+40, 400+40)
        }
    ));

    canvas.draw();


    canvas.bufferToFile("./output.png");
    
    return 0;
}