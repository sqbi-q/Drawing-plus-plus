#include "../../Drawing++.hpp"
#include <memory>
#include <cmath>
#include <cassert>

static double step(double edge, double x){
    return !(x < edge);
}

static void mustache_draw_func(Drawing::Drawable *drawable, Drawing::Canvas* canvas){
    assert(drawable->points.size() >= 3);

    Drawing::Point res = drawable->points[0];
    Drawing::Point zoom = drawable->points[1];
    Drawing::Point pos = drawable->points[2];

    for(png_uint_32 _x=0; _x<canvas->getWidth(); _x++){  
        for(png_uint_32 _y=0; _y<canvas->getHeight(); _y++){

            double x = (_x / (double) res.x());
            double y = (_y / (double) res.y());

            x /= zoom[0];
            y /= zoom[0];

            x += 0.5 - pos.x()/zoom[0];
            y -= pos.y()/zoom[0];

            if (x < 0) continue;
            if (x > 1.0) continue;

            double a = std::abs(x-0.5);
            
            double lengthA = 1.920;
            double ampA = 0.772;
            double wA = sin(x*3.14)*ampA - (1.0/lengthA);
            
            double lengthB = 1.960;
            double ampB = 0.324;
            double wB = sin(x*3.14)*ampB - (1.0/lengthB);
            
            double f1 = step(y, std::abs(a*wA+0.25));
            double f2 = step(y, std::abs(a*(wB+0.5)));

            double f = (f1-f2);

            if(f){
                canvas->putPixel(_x, _y, drawable->getPixel(_x, _y));
            }
        }
    }
}

int main(){
    Drawing::Canvas canvas(512, 512);
    
    canvas.addDrawable(Drawing::ImageFile("./Lenna_(test_image).png"));
    canvas.addDrawable(Drawing::Figure(
        Drawing::Color(0.3, 0.1, 0.0, 1.0), mustache_draw_func,
        std::vector<Drawing::Point>{ 
            Drawing::Point({512, 512}), Drawing::Point({0.25}), 
            Drawing::Point({0.6, 0.65}) 
        }
    ));
    canvas.draw();

    canvas.bufferToFile("./mustachegirl.png");
    return 0;
}