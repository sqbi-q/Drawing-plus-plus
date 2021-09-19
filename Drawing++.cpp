#include "Drawing++.hpp"

Drawing::Figure::Figure(Color bgColor,
    draw_fn_ptr drawFnPtr, std::vector<Point> points){
    
    this->drawFn = drawFnPtr;
    m_bgColor = bgColor;
    this->points = points;
}


static void _imageFileDrawFn(Drawing::Drawable* drawable, Drawing::Canvas* canvas){
    for(png_uint_32 x=0; x<canvas->getWidth(); x++){  
        for(png_uint_32 y=0; y<canvas->getHeight(); y++){
            canvas->putPixel(x, y, drawable->getPixel(x, y));
        }
    }
}

Drawing::ImageFile::ImageFile(const char* filename) {
    loadPNGFile(filename);
    setDrawFn(_imageFileDrawFn);
}


void Drawing::ImageFile::loadPNGFile(const char* filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) abort();

    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!pngPtr) abort();

    png_infop infoPtr = png_create_info_struct(pngPtr);
    if(!infoPtr) abort();

    if(setjmp(png_jmpbuf(pngPtr))) abort();

    png_init_io(pngPtr, fp);

    png_read_info(pngPtr, infoPtr);

    png_uint_32 width      = png_get_image_width(pngPtr, infoPtr);
    png_uint_32 height     = png_get_image_height(pngPtr, infoPtr);
    png_byte color_type = png_get_color_type(pngPtr, infoPtr);
    png_byte bit_depth  = png_get_bit_depth(pngPtr, infoPtr);

    // Read any color_type into 8bit depth, RGBA format.
    // See http://www.libpng.org/pub/png/libpng-manual.txt

    if(bit_depth == 16)
        png_set_strip_16(pngPtr);
    
    if(color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(pngPtr);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(pngPtr);

    if(png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(pngPtr);

    if(color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(pngPtr, 0xFF, PNG_FILLER_AFTER);

    if(color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(pngPtr);

    png_read_update_info(pngPtr, infoPtr);

    if (m_rowBufferPtrs) abort();

    m_rowBufferPtrs = (png_bytep*)malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        m_rowBufferPtrs[y] = (png_byte*) malloc(png_get_rowbytes(pngPtr, infoPtr));
    }

    png_read_image(pngPtr, m_rowBufferPtrs);

    fclose(fp);

    png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
}

Drawing::Color Drawing::ImageFile::getPixel(png_uint_32 x, png_uint_32 y){
    assert(m_rowBufferPtrs != NULL);

    Drawing::Color color;
    png_bytep row = m_rowBufferPtrs[y];
    const png_byte channels = 4;

    color.r = row[channels*x] / 255.0;
    color.g = row[channels*x+1] / 255.0;
    color.b = row[channels*x+2] / 255.0;
    color.a = row[channels*x+3] / 255.0;

    return color;
}


void Drawing::Canvas::_copyConstructor(const Drawing::Canvas& canvas){
    assert(canvas.m_pngPtr != nullptr);
    assert(canvas.m_infoPtr != nullptr);
    assert(canvas.m_rowBufferPtrs != nullptr);

    png_uint_32 width = png_get_image_width(canvas.m_pngPtr, canvas.m_infoPtr);
    png_uint_32 height = png_get_image_height(canvas.m_pngPtr, canvas.m_infoPtr);

    initImage(canvas.m_pngPtr, canvas.m_infoPtr);
    initBuffer();

    for(int y = 0; y < height; y++) {
        *(m_rowBufferPtrs[y]) = *(canvas.m_rowBufferPtrs[y]);
    }
}

Drawing::Canvas::Canvas(const Drawing::Canvas& canvas){
    _copyConstructor(canvas);
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


Drawing::Canvas& Drawing::Canvas::operator=(Canvas rhs){
    _copyConstructor(rhs);
    return *this;
}



void createPngStructs(png_structp *pngPtr, png_infop *infoPtr){
    *pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!(*pngPtr)) abort();

    *infoPtr = png_create_info_struct(*pngPtr);
    if (!infoPtr) abort();

    if (setjmp(png_jmpbuf(*pngPtr))) abort();
}

void Drawing::Canvas::initImage(png_uint_32 width, png_uint_32 height,
    int bitDepth, int colorType, int interlaceMethod, int compressMethod, int filterMethod){

    createPngStructs(&m_pngPtr, &m_infoPtr);
    png_set_IHDR(
        m_pngPtr, m_infoPtr, width, height,
        bitDepth, colorType, interlaceMethod,
        compressMethod, filterMethod
    );
}

void Drawing::Canvas::initImage(const png_structp &pngPtr, const png_infop &infoPtr){

    createPngStructs(&m_pngPtr, &m_infoPtr);
    png_set_IHDR(
        m_pngPtr, m_infoPtr,
        png_get_image_width(pngPtr, infoPtr),
        png_get_image_height(pngPtr, infoPtr),
        png_get_bit_depth(pngPtr, infoPtr),
        png_get_color_type(pngPtr, infoPtr),
        png_get_interlace_type(pngPtr, infoPtr),
        png_get_compression_type(pngPtr, infoPtr),
        png_get_filter_type(pngPtr, infoPtr)
    );
}

void Drawing::Canvas::initBuffer(Color bgColor){
    png_uint_32 height = png_get_image_height(m_pngPtr, m_infoPtr);
    png_uint_32 width = png_get_image_width(m_pngPtr, m_infoPtr);
    size_t rowbytes = png_get_rowbytes(m_pngPtr, m_infoPtr);

    m_rowBufferPtrs = (png_bytep*) malloc(height*sizeof(png_bytep));
    for(unsigned y=0; y<height; y++) {
        m_rowBufferPtrs[y] = (png_byte*) malloc(rowbytes);
        
        //set default background color
        for (unsigned x=0; x<rowbytes; x+=4){
            m_rowBufferPtrs[y][x] =     bgColor.r * 255;
            m_rowBufferPtrs[y][x+1] =   bgColor.g * 255;
            m_rowBufferPtrs[y][x+2] =   bgColor.b * 255;
            m_rowBufferPtrs[y][x+3] =   bgColor.a * 255;
        }
    }
}



#define mixColor(oldColor, alphaOld, newColor, alphaNew) \
    oldColor*alphaOld*(1-alphaNew) + newColor*alphaNew

/*
alphaMixConst = alphaOld * negAlphaConst
negAlphaConst = (1-alphaNew)
newColorConst = newColor * alphaNew
*/
#define mixColor2(oldColor, alphaMixConst, newColorConst) \
    oldColor*alphaMixConst + newColorConst


void Drawing::Canvas::putPixel(
    png_uint_32 x, png_uint_32 y, Drawing::Color color){
    
    const png_byte channels = png_get_channels(m_pngPtr, m_infoPtr);
    png_bytep pixel = &m_rowBufferPtrs[y][x*channels]; //C = {0...255}
    const double a = pixel[3] / (int) 255;
    const double alphaMix = a * (1-color.a);

    const double _255mulAlpha = 255*color.a;
    color.multiplyRGB(_255mulAlpha, _255mulAlpha, _255mulAlpha); //newColorConstant created

    pixel[0] = mixColor2(pixel[0], alphaMix, color.r);
    pixel[1] = mixColor2(pixel[1], alphaMix, color.g);
    pixel[2] = mixColor2(pixel[2], alphaMix, color.b);
}

void Drawing::Canvas::setPixel(
    png_uint_32 x, png_uint_32 y, Drawing::Color color){

    const png_byte channels = png_get_channels(m_pngPtr, m_infoPtr);
    color.multiplyRGB(255, 255, 255);

    m_rowBufferPtrs[y][x*channels] = color.r;
    m_rowBufferPtrs[y][x*channels+1] = color.g;
    m_rowBufferPtrs[y][x*channels+2] = color.b;
}

void Drawing::Canvas::fillputPixels(
    png_uint_32 x1, png_uint_32 x2, png_uint_32 y, 
    Drawing::Color color){
    
    const png_byte channels = png_get_channels(m_pngPtr, m_infoPtr);
    const double negAlpha = 1-color.a;
    const double _255mulAlpha = 255*color.a;
    color.multiplyRGB(_255mulAlpha, _255mulAlpha, _255mulAlpha); //newColorConstant created

    png_bytep row = m_rowBufferPtrs[y];

    for (png_uint_32 x=x1; x<x2; x++){

        const double bufferAlpha = *(row+x*channels+3) / (int) 255;
        const double alphaMix = bufferAlpha*negAlpha;

        row[x*channels] = mixColor2(row[x*channels], alphaMix, color.r);
        row[x*channels+1] = mixColor2(row[x*channels+1], alphaMix, color.g);
        row[x*channels+2] = mixColor2(row[x*channels+2], alphaMix, color.b);
    }
}
void Drawing::Canvas::fillputPixels(png_uint_32 x1, png_uint_32 x2,
    png_uint_32 y1, png_uint_32 y2, Drawing::Color color){

    const png_byte channels = png_get_channels(m_pngPtr, m_infoPtr);
    const double negAlpha = 1-color.a;
    const double _255mulAlpha = 255*color.a;
    color.multiplyRGB(_255mulAlpha, _255mulAlpha, _255mulAlpha); //newColorConstant created

    for (png_uint_32 y=y1; y<y2; y++){
        png_bytep row = m_rowBufferPtrs[y];

        for (png_uint_32 x=x1; x<x2; x++){

            const double bufferAlpha = *(row+x*channels+3) / (int) 255;
            const double alphaMix = bufferAlpha*negAlpha;

            row[x*channels] = mixColor2(row[x*channels], alphaMix, color.r);
            row[x*channels+1] = mixColor2(row[x*channels+1], alphaMix, color.g);
            row[x*channels+2] = mixColor2(row[x*channels+2], alphaMix, color.b);
        }
    }
}

void Drawing::Canvas::fillsetPixels(
    png_uint_32 x1, png_uint_32 x2, png_uint_32 y, 
    Drawing::Color color){
    
    const png_byte channels = png_get_channels(m_pngPtr, m_infoPtr);
    color.multiplyRGB(255, 255, 255);

    png_bytep row = m_rowBufferPtrs[y];

    for (png_uint_32 x=x1; x<x2; x++){
        row[x*channels] = (png_byte) color.r;
        row[x*channels+1] = (png_byte) color.g;
        row[x*channels+2] = (png_byte) color.b;
    }
}
void Drawing::Canvas::fillsetPixels(png_uint_32 x1, png_uint_32 x2,
    png_uint_32 y1, png_uint_32 y2, Drawing::Color color){
    
    const png_byte channels = png_get_channels(m_pngPtr, m_infoPtr);
    color.multiplyRGB(255, 255, 255);

    for (png_uint_32 y=y1; y<y2; y++){
        png_bytep row = m_rowBufferPtrs[y];

        for (png_uint_32 x=x1; x<x2; x++){
            row[x*channels] = (png_byte) color.r;
            row[x*channels+1] = (png_byte) color.g;
            row[x*channels+2] = (png_byte) color.b;
        }
    }
}




void Drawing::Canvas::draw(){
    assert(m_rowBufferPtrs != nullptr);
    assert(m_pngPtr != nullptr);
    assert(m_infoPtr != nullptr);

    for (auto& drawable : m_drawables){
        if(drawable->drawFn != nullptr)
            drawable->drawFn(drawable.get(), this);
    }
}

double Drawing::Canvas::compare(Canvas &canvasB){
    assert(m_rowBufferPtrs != NULL);
    assert(m_pngPtr != NULL);
    assert(m_infoPtr != NULL);

    png_uint_32 height = png_get_image_height(m_pngPtr, m_infoPtr);
    png_uint_32 width = png_get_image_width(m_pngPtr, m_infoPtr);
    png_byte channels = png_get_channels(m_pngPtr, m_infoPtr);
    Canvas *canvasA = this;

    double sumSquareDiff = 0.0;
    double pixelA_sumSquare = 0.0;
    double pixelB_sumSquare = 0.0;

    for (png_uint_32 y=0; y<height; y++){
        for (png_uint_32 x=0; x<width; x++){
            png_bytep pixelA = &canvasA->m_rowBufferPtrs[y][x*channels];
            png_bytep pixelB = &canvasB.m_rowBufferPtrs[y][x*channels];
            
            double channelSumA = (double)pixelA[0]+(double)pixelA[1]+(double)pixelA[2]+(double)pixelA[3];
            double channelSumB = (double)pixelB[0]+(double)pixelB[1]+(double)pixelB[2]+(double)pixelB[3];

            double diff = channelSumA - channelSumB;
            sumSquareDiff += diff*diff;

            pixelA_sumSquare += channelSumA*channelSumA;
            pixelB_sumSquare += channelSumB*channelSumB;
        }
    }

    return sumSquareDiff/sqrt(pixelA_sumSquare*pixelB_sumSquare);
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

void Drawing::rect_filled(Drawing::Drawable *drawable, Drawing::Canvas* canvas){
    const Drawing::Color pixel = drawable->getPixel(0, 0);
    canvas->fillputPixels(drawable->points[0].x(), drawable->points[1].x(), 
        drawable->points[0].y(), drawable->points[1].y(), pixel);
}

// static inline double slope(Drawing::Point &A, Drawing::Point &B){
//     return (B.y() - A.y()) / (B.x() - A.x());
// }
static double invSlope(Drawing::Point &A, Drawing::Point &B){
    return (B.x() - A.x()) / (B.y() - A.y());
}

// static double line(double x, Drawing::Point &A, Drawing::Point &B){
//     return slope(A, B) * (x - B.x()) + B.y();
// }
// static double line_zero(double y, Drawing::Point &A, Drawing::Point &B){
//     if (B.x()-A.x() == 0) return A.x();
//     return (y-B.y())/slope(A,B) + B.x();
// }
static double line_zero(double y, Drawing::Point &A, Drawing::Point &B){
    if (B.y()-A.y() == 0) return A.x();
    return (y-B.y())*invSlope(A,B) + B.x();
}

void Drawing::triangle_filled(Drawing::Drawable *drawable, Drawing::Canvas* canvas){
    const Drawing::Color color = drawable->getPixel(0, 0);

    std::sort(drawable->points.begin(), drawable->points.end(), [](Drawing::Point a, Drawing::Point b) {
        return a.y() < b.y();
    });

    for(png_uint_32 y=ceil(drawable->points[0].y()); y<drawable->points[2].y(); y++){
        
        png_uint_32 x1 = line_zero(y, drawable->points[0], drawable->points[2]);
        png_uint_32 x2;

        if (y < drawable->points[1].y())
            x2 = line_zero(y, drawable->points[0], drawable->points[1]);
        else
            x2 = line_zero(y, drawable->points[1], drawable->points[2]);

        if(x1 > x2) 
            std::swap(x1, x2);

        canvas->fillputPixels(
            x1, x2,
            y, color
        );
    }
}