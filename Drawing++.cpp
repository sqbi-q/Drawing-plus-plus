#include "Drawing++.hpp"
#include <cmath>
#include <cstring>
#include <cassert>


static double mix(double x, double y, double a){
    return x*(1-a) + y*a;
}

static double max(double x, double y){
    if (x < y) return y;
    return x;
}


static void drawPixel(png_uint_16p pixel, Drawing::Drawable *drawableList, int nargs, double x, double y){
    double r=1, g=1, b=1, a=1;

    for (unsigned i=0; i<nargs; i++){
        const double shape = drawableList[i].shape_fn(&(drawableList[i]), x, y);
        
        // subtractive color mixing
        a = max(a, drawableList[i].color->alpha*shape);
        r = mix(r, drawableList[i].color->red*shape, drawableList[i].color->alpha*shape);
        g = mix(g, drawableList[i].color->green*shape, drawableList[i].color->alpha*shape);
        b = mix(b, drawableList[i].color->blue*shape, drawableList[i].color->alpha*shape);
    }

    if (a > 0){
        if (a > 1){
            if (r > 1) r = 1;
            if (g > 1) g = 1;
            if (b > 1) b = 1;
            a = 1;
        }

        /* And fill in the pixel: */
        pixel[0] = (png_uint_16)/*SAFE*/round(r * 65535);
        pixel[1] = (png_uint_16)/*SAFE*/round(g * 65535);
        pixel[2] = (png_uint_16)/*SAFE*/round(b * 65535);
        pixel[3] = (png_uint_16)/*SAFE*/round(a * 65535);
    }else
        pixel[3] = pixel[2] = pixel[1] = pixel[0] = 0;
}

static void drawPixel(png_uint_16p pixel, std::vector<Drawing::Drawable> drawables, double x, double y){
    drawPixel(pixel, drawables.data(), drawables.size(), x, y);
}


Drawing::Canvas::Canvas(png_uint_32 width, png_uint_32 height){
    initImage(width, height);
    initBuffer();
}

Drawing::Canvas::~Canvas(){
    free(m_buffer);
}


void Drawing::Canvas::initImage(const png_uint_32 width, const png_uint_32 height){
    memset(&m_image, 0, sizeof m_image);
    m_image.version = PNG_IMAGE_VERSION;
    m_image.opaque = NULL;
    m_image.width = width;
    m_image.height = height;
    m_image.format = PNG_FORMAT_LINEAR_RGB_ALPHA;
    m_image.flags = 0;
    m_image.colormap_entries = 0;
}

void Drawing::Canvas::initBuffer(){
    m_buffer = (png_uint_16p) malloc(PNG_IMAGE_SIZE(m_image));
}

void Drawing::Canvas::addDrawable(const Drawable &drawable){
    m_drawables.push_back(drawable);
}

void Drawing::Canvas::draw(){
    assert(m_buffer != NULL);

    for (png_uint_32 y=0; y<m_image.height; y++){
        for (png_uint_32 x=0; x<m_image.width; x++)
            drawPixel(m_buffer + 4*(x + y*m_image.width), m_drawables, x, y);
    }
}

void Drawing::Canvas::bufferToFile(const char* filepath){
    FILE *fp = fopen(filepath, "wb");
    if (!fp) abort();

    assert(m_buffer != NULL);

    if (!png_image_write_to_stdio(&m_image, fp, 0, m_buffer, 0, NULL))
        fprintf(stderr, "Canvas: buffer write error: %s\n", m_image.message);

    fclose(fp);
}