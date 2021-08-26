#include "../../Drawing++.hpp"
#include <iostream>


int main(){
    Drawing::Canvas canvasA(512, 512);
    canvasA.addDrawable(Drawing::ImageFile("./Lenna_(test_image).png"));
    canvasA.draw();

    Drawing::Canvas canvasB(512, 512);
    canvasB.addDrawable(Drawing::ImageFile("./mustachegirl.png"));
    canvasB.draw();

    double cmp = canvasA.compare(canvasB);
    std::cout << "Compare value: " << cmp << std::endl;
    std::cout << "1/cmp: " << 1/(cmp) << std::endl;

    return 0;
}