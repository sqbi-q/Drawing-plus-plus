# Drawing++

Drawing++ is C++ library for drawing user-defined shapes using libpng.

## Compiling

Compile Drawing++.cpp with libpng flags (from `libpng-config` or link manually).

g++ example:
```sh
#compile to Drawing++.o
g++ -c Drawing++.cpp `libpng-config --libs --cflags`

#compile example code
g++ Example/Example.cpp Drawing++.o `libpng-config --libs --cflags`
#g++ Example/Example.cpp Drawing++.cpp `libpng-config --libs --cflags`
```

## Basic usage, drawing squares on canvas:
```c++
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
```
`Output image:`

![output image](Examples/ThreeSquares/output.png)

---

## Loading image to canvas:
```c++
#include "../../Drawing++.hpp"

static int mustache_shape(Drawing::Drawable* drawable, png_uint_32 _x, png_uint_32 _y){
    //whole function in ./Examples/LoadPNG/LoadPNG.cpp
}

int main(){
    Drawing::Canvas canvas(512, 512);
    
    canvas.addDrawable(new Drawing::ImageFile(
        "./Lenna_(test_image).png"
    )); 
    //Create imageFile and add to canvas
    //const char* filepath is file path to PNG image file
    
    canvas.addDrawable(new Drawing::Figure(
        Drawing::Color(0.3, 0.1, 0.0, 1.0), mustache_shape,
        std::vector<Drawing::Point>{ 
            Drawing::Point(512, 512), Drawing::Point(0.25), 
            Drawing::Point(0.6, 0.65) 
        }
    )); //Use mustache shape!

    canvas.draw();
    canvas.bufferToFile("./mustachegirl.png");
    return 0;
}
```
`Output image:`

![output image](Examples/LoadPNG/mustachegirl.png)

## License
[MIT](https://choosealicense.com/licenses/mit/)
