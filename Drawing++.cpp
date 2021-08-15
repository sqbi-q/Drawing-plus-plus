#include "Drawing++.hpp"
#include <cassert>
#include <iostream>


Drawing::ImageFile::ImageFile(const char* filename) {
    loadPNGFile(filename);
}

Drawing::ImageFile::~ImageFile() {
    png_destroy_read_struct(&m_pngPtr, &m_infoPtr, NULL);
}

void Drawing::ImageFile::loadPNGFile(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) abort();

    m_pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!m_pngPtr) abort();

    m_infoPtr = png_create_info_struct(m_pngPtr);
    if(!m_infoPtr) abort();

    if(setjmp(png_jmpbuf(m_pngPtr))) abort();

    png_init_io(m_pngPtr, fp);

    png_read_info(m_pngPtr, m_infoPtr);

    png_uint_32 width      = png_get_image_width(m_pngPtr, m_infoPtr);
    png_uint_32 height     = png_get_image_height(m_pngPtr, m_infoPtr);
    png_byte color_type = png_get_color_type(m_pngPtr, m_infoPtr);
    png_byte bit_depth  = png_get_bit_depth(m_pngPtr, m_infoPtr);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if(bit_depth == 16)
        png_set_strip_16(m_pngPtr);
    
    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(m_pngPtr);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(m_pngPtr);

    if(png_get_valid(m_pngPtr, m_infoPtr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(m_pngPtr);

    if(color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(m_pngPtr, 0xFF, PNG_FILLER_AFTER);

    if(color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(m_pngPtr);

    png_read_update_info(m_pngPtr, m_infoPtr);


    if (m_rowBufferPtrs) abort();

    m_rowBufferPtrs = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        m_rowBufferPtrs[y] = (png_byte*) malloc(png_get_rowbytes(m_pngPtr, m_infoPtr));
    }

    png_read_image(m_pngPtr, m_rowBufferPtrs);

    fclose(fp);
}

Drawing::Color Drawing::ImageFile::getPixel(png_uint_32 x, png_uint_32 y){    
    assert(m_rowBufferPtrs != NULL);

    Drawing::Color color;
    png_bytep row = m_rowBufferPtrs[y];

    color.r = row[x] / 255.0;
    color.g = row[x+1] / 255.0;
    color.b = row[x+2] / 255.0;
    color.a = row[x+3] / 255.0;

    return color;
}



Drawing::Canvas::Canvas(png_uint_32 width, png_uint_32 height,
    int bitDepth, int colorType, int interlaceMethod, int compressMethod, int filterMethod){
    
    initImage(width, height, bitDepth, colorType, interlaceMethod, compressMethod, filterMethod);
    initBuffer();
}


Drawing::Canvas::~Canvas(){
    png_uint_32 height = png_get_image_height(m_pngPtr, m_infoPtr);
    for(int y = 0; y < height; y++) {
        free(m_rowBufferPtrs[y]);
    }
    free(m_rowBufferPtrs);
    png_destroy_write_struct(&m_pngPtr, &m_infoPtr);
}


void Drawing::Canvas::initImage(png_uint_32 width, png_uint_32 height,
    int bitDepth, int colorType, int interlaceMethod, int compressMethod, int filterMethod){

    m_pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!m_pngPtr) abort();

    m_infoPtr = png_create_info_struct(m_pngPtr);
    if (!m_infoPtr) abort();

    if (setjmp(png_jmpbuf(m_pngPtr))) abort();

    png_set_IHDR(
        m_pngPtr,
        m_infoPtr,
        width, height,
        bitDepth,
        colorType,
        interlaceMethod,
        compressMethod,
        filterMethod
    );
}

void Drawing::Canvas::initBuffer(Color bgColor){
    png_uint_32 height = png_get_image_height(m_pngPtr, m_infoPtr);
    png_uint_32 width = png_get_image_width(m_pngPtr, m_infoPtr);
    size_t rowbytes = png_get_rowbytes(m_pngPtr, m_infoPtr);

    m_rowBufferPtrs = (png_bytep*) malloc(height*sizeof(png_bytep));
    for(unsigned y=0; y<height; y++) {
        m_rowBufferPtrs[y] = (png_byte*) malloc(rowbytes);
        
        for (unsigned x=0; x<rowbytes; x+=4){
            m_rowBufferPtrs[y][x] = bgColor.r;
            m_rowBufferPtrs[y][x+1] = bgColor.g;
            m_rowBufferPtrs[y][x+2]= bgColor.b;
            m_rowBufferPtrs[y][x+3] = bgColor.a;
        }
    }
}

static double mix(double x, double y, double a){
    return x*(1-a) + y*a;
}

static double max(double x, double y){
    if (x < y) return y;
    return x;
}

void drawPixel(png_bytep pixel, std::vector<std::unique_ptr<Drawing::Drawable>> &drawables, 
    png_uint_32 x, png_uint_32 y, png_byte channels){

    double r = pixel[0];
    double g = pixel[1];
    double b = pixel[2];
    double a = pixel[3];

    // std::cout << "PREDEBUG" << std::endl;

    for (auto& drawable : drawables){
        const int shape = (drawable->shape_fn != nullptr) ? 
            drawable->shape_fn(drawable, x, y) : 1;
        // const int shape = drawable->shape_fn(drawable, x, y);

        Drawing::Color color = drawable->getPixel(x*channels, y);

        // std::cout << "pixel b4: " << std::endl;
        // std::cout << "\tr: " << r << std::endl;
        // std::cout << "\tg: " << g << std::endl;
        // std::cout << "\tb: " << b << std::endl;
        // std::cout << "\ta: " << a << std::endl;

        a = max(a, color.a*shape);
        r = mix(r, color.r*shape,   color.a*shape);
        g = mix(g, color.g*shape,   color.a*shape);
        b = mix(b, color.b*shape,   color.a*shape);

        // std::cout << "pixel aftR: " << std::endl;
        // std::cout << "\tr: " << r << std::endl;
        // std::cout << "\tg: " << g << std::endl;
        // std::cout << "\tb: " << b << std::endl;
        // std::cout << "\ta: " << a << std::endl;
    }  

    // std::cout << "POSTDEBUG" << std::endl;

    pixel[0] = (png_byte) round(r*255);
    pixel[1] = (png_byte) round(g*255);
    pixel[2] = (png_byte) round(b*255);
    pixel[3] = (png_byte) round(a*255);

    // std::cout << "colored after: " << std::endl;
    // std::cout << "\tr: " << (unsigned) pixel[0] << std::endl;
    // std::cout << "\tg: " << (unsigned) pixel[1] << std::endl;
    // std::cout << "\tb: " << (unsigned) pixel[2] << std::endl;
    // std::cout << "\ta: " << (unsigned) pixel[3] << std::endl;
}

void Drawing::Canvas::draw(){
    assert(m_rowBufferPtrs != NULL);

    png_uint_32 height = png_get_image_height(m_pngPtr, m_infoPtr);
    png_uint_32 width = png_get_image_width(m_pngPtr, m_infoPtr);
    png_byte channels = png_get_channels(m_pngPtr, m_infoPtr);

    for (png_uint_32 y=0; y<height; y++){
        for (png_uint_32 x=0; x<width; x++){
            png_bytep pixel = &m_rowBufferPtrs[y][x*channels];
            drawPixel(pixel, m_drawables, x, y, channels);
        }
    }
}

void Drawing::Canvas::bufferToFile(const char* filepath){
    FILE *fp = fopen(filepath, "wb");
    if (!fp) abort();

    assert(m_rowBufferPtrs != NULL);
    assert(m_pngPtr != NULL);
    assert(m_infoPtr != NULL);

    png_init_io(m_pngPtr, fp);
    png_write_info(m_pngPtr, m_infoPtr);

    png_write_image(m_pngPtr, m_rowBufferPtrs);
    png_write_end(m_pngPtr, NULL);

    fclose(fp);
}


























// #include "Drawing++.hpp"
// #include <cmath>
// #include <cstring>
// #include <cassert>


// static double mix(double x, double y, double a){
//     return x*(1-a) + y*a;
// }

// static double max(double x, double y){
//     if (x < y) return y;
//     return x;
// }


// static void drawPixel(png_uint_16p pixel, Drawing::Drawable *drawableList, int nargs, double x, double y){
//     double r=1, g=1, b=1, a=1;

//     for (unsigned i=0; i<nargs; i++){
//         const double shape = drawableList[i].shape_fn(&(drawableList[i]), x, y);
        
//         // subtractive color mixing
//         a = max(a, drawableList[i].color->alpha*shape);
//         r = mix(r, drawableList[i].color->red*shape, drawableList[i].color->alpha*shape);
//         g = mix(g, drawableList[i].color->green*shape, drawableList[i].color->alpha*shape);
//         b = mix(b, drawableList[i].color->blue*shape, drawableList[i].color->alpha*shape);
//     }

//     if (a > 0){
//         if (a > 1){
//             if (r > 1) r = 1;
//             if (g > 1) g = 1;
//             if (b > 1) b = 1;
//             a = 1;
//         }

//         /* And fill in the pixel: */
//         pixel[0] = (png_uint_16)/*SAFE*/round(r * 65535);
//         pixel[1] = (png_uint_16)/*SAFE*/round(g * 65535);
//         pixel[2] = (png_uint_16)/*SAFE*/round(b * 65535);
//         pixel[3] = (png_uint_16)/*SAFE*/round(a * 65535);
//     }else
//         pixel[3] = pixel[2] = pixel[1] = pixel[0] = 0;
// }

// static void drawPixel(png_uint_16p pixel, std::vector<Drawing::Drawable> drawables, double x, double y){
//     drawPixel(pixel, drawables.data(), drawables.size(), x, y);
// }


// Drawing::Canvas::Canvas(png_uint_32 width, png_uint_32 height){
//     initImage(width, height);
//     initBuffer();
// }

// Drawing::Canvas::~Canvas(){
//     free(m_buffer);
// }


// void Drawing::Canvas::initImage(const png_uint_32 width, const png_uint_32 height){
//     memset(&m_image, 0, sizeof m_image);
//     m_image.version = PNG_IMAGE_VERSION;
//     m_image.opaque = NULL;
//     m_image.width = width;
//     m_image.height = height;
//     m_image.format = PNG_FORMAT_LINEAR_RGB_ALPHA;
//     m_image.flags = 0;
//     m_image.colormap_entries = 0;
// }

// void Drawing::Canvas::initBuffer(){
//     m_buffer = (png_uint_16p) malloc(PNG_IMAGE_SIZE(m_image));
// }

// void Drawing::Canvas::addDrawable(const Drawable &drawable){
//     m_drawables.push_back(drawable);
// }

// void Drawing::Canvas::draw(){
//     assert(m_buffer != NULL);

//     for (png_uint_32 y=0; y<m_image.height; y++){
//         for (png_uint_32 x=0; x<m_image.width; x++){
//             drawPixel(m_buffer + (4*(x + y*m_image.width)), m_drawables, x, y);
//         }
//     }
// }

// void Drawing::Canvas::bufferToFile(const char* filepath){
//     FILE *fp = fopen(filepath, "wb");
//     if (!fp) abort();

//     assert(m_buffer != NULL);

//     if (!png_image_write_to_stdio(&m_image, fp, 0, m_buffer, 0, NULL))
//         fprintf(stderr, "Canvas: buffer write error: %s\n", m_image.message);

//     fclose(fp);
// }