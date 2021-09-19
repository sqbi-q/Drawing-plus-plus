#include <vector>
#include <png.h>
#include <memory>
#include <stdlib.h>
#include <cassert>
#include <cmath>
#include <algorithm>

#define DEFAULT_DRAWING_FUNCS 1

namespace Drawing {
    struct Color{
        Color (void) {}
        Color (double r, double g, double b, double a){
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
        Color& operator*=(const Color &rhs) {
            multiply(rhs.r, rhs.g, rhs.b, rhs.a);
            return *this;
        }
        Color& operator*=(const double x) {
            multiply(x, x, x, x);
            return *this;
        }
        void multiply(double r, double g, double b, double a){
            this->r *= r;
            this->g *= g;
            this->b *= b;
            this->a *= a;
        }
        void multiplyRGB(double r, double g, double b) { multiply(r, g, b, 1); }
        double r, g, b, a;
    };
    inline Color operator*(Color lhs, const Color &rhs){
        return Color(lhs.r*rhs.r, lhs.g*rhs.g, lhs.b*rhs.b, lhs.a*rhs.a);
    }
    inline Color operator*(Color lhs, const double x) {
        return Color(lhs.r*x, lhs.g*x, lhs.b*x, lhs.a*x);
    }

    
    //behold true beauty!!! 
    struct Point : std::vector<double> {
        Point(void) {};
        Point(std::vector<double> coords) : std::vector<double>(coords) {}
        double x(void) const { return this->at(0); }
        double y(void) const { return this->at(1); }
        double z(void) const { return this->at(2); }
        double w(void) const { return this->at(3); }
        Point& operator*=(const Point &rhs){
            std::transform(this->begin(), this->end(), rhs.begin(), 
                this->begin(), std::multiplies<double>());
            return *this;
        }
    };
    inline Point operator*(Point lhs, const Point &rhs) {
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), 
            lhs.begin(), std::multiplies<double>());
        return lhs;
    }


    class Drawable;
    class Canvas;
    using draw_fn_ptr = void(*)(Drawable* drawable, Canvas* canvas);

    class Drawable {
        public:
            virtual Color getPixel(unsigned x, unsigned y) = 0;
            void setDrawFn(draw_fn_ptr drawFnPtr) { drawFn = drawFnPtr; }

            std::vector<Point> points;
            draw_fn_ptr drawFn = nullptr;
    };



    class Figure : public Drawable {
        public:
            Figure (void) {};
            Figure (Color bgColor, draw_fn_ptr drawFnPtr, 
                std::vector<Point> points);
            
            Color getPixel(unsigned x, unsigned y) { return m_bgColor; }

        private:
            Color m_bgColor;
    };

    class ImageFile : public Drawable {
        public:
            ImageFile(void) {};
            ImageFile(const char* filename);

            void loadPNGFile(const char* filename);
            Color getPixel(png_uint_32 x, png_uint_32 y);
            
        private:
            png_bytep *m_rowBufferPtrs = nullptr;
    };



    class Canvas {
        public:
            Canvas(void) {};
            Canvas(const Canvas& canvas);
            Canvas(png_uint_32 width, png_uint_32 height,
                int bitDepth = 8, int colorType = PNG_COLOR_TYPE_RGBA,
                int interlaceMethod = PNG_INTERLACE_NONE,
                int compressMethod = PNG_COMPRESSION_TYPE_DEFAULT,
                int filterMethod = PNG_FILTER_TYPE_DEFAULT);
            ~Canvas();

            Canvas& operator=(Canvas rhs);

            //init new image [width x height]
            void initImage(const png_uint_32 width, const png_uint_32 height, 
                int bitDepth = 8, int colorType = PNG_COLOR_TYPE_RGBA,
                int interlaceMethod = PNG_INTERLACE_NONE,
                int compressMethod = PNG_COMPRESSION_TYPE_DEFAULT,
                int filterMethod = PNG_FILTER_TYPE_DEFAULT);
            
            //init new image and assign values from pngPtr and infoPtr
            void initImage(const png_structp &pngPtr, const png_infop &infoPtr);

            void initBuffer(
                Color bgColor = Color(1.0, 1.0, 1.0, 1.0)
            );

            //T Drawable to K (default T) shared ptr
            template<typename T, typename K = T>
            void addDrawable(const T& drawable){
                m_drawables.push_back(
                    std::static_pointer_cast<Drawable>(
                        std::make_shared<K>(drawable))
                );
            }
            void addDrawable(std::shared_ptr<Drawable> drawable){
                m_drawables.push_back(drawable);
            }

            void putPixel(png_uint_32 x, png_uint_32 y, Drawing::Color color);
            void setPixel(png_uint_32 x, png_uint_32 y, Drawing::Color color);
            
            void fillputPixels(png_uint_32 x1, png_uint_32 x2, png_uint_32 y, 
                Drawing::Color color);
            void fillputPixels(png_uint_32 x1, png_uint_32 x2, 
                png_uint_32 y1, png_uint_32 y2, Drawing::Color color);

            void fillsetPixels(png_uint_32 x1, png_uint_32 x2, png_uint_32 y, 
                Drawing::Color color);
            void fillsetPixels(png_uint_32 x1, png_uint_32 x2, 
                png_uint_32 y1, png_uint_32 y2, Drawing::Color color);


            void draw();
        
            double compare(Canvas &canvasB);

            void bufferToFile(const char* filepath);

            std::size_t getDrawablesSize(void) const { return m_drawables.size(); }
            // Drawing::Drawable* getDrawable(const unsigned index) { return m_drawables[index].get(); }

            std::shared_ptr<Drawable> getDrawable(const unsigned index) { 
                return m_drawables[index];
            }

            //T Drawable to K (default T) shared ptr
            template<typename T, typename K = T>
            void setDrawable(const T& drawable, const unsigned index) {
                m_drawables[index] = (
                    std::static_pointer_cast<Drawable>(
                        std::make_shared<K>(drawable))
                );
            }

            png_uint_32 getWidth(void) const { return png_get_image_width(m_pngPtr, m_infoPtr); }
            png_uint_32 getHeight(void) const { return png_get_image_height(m_pngPtr, m_infoPtr); }

        private:
            std::vector<std::shared_ptr<Drawable>> m_drawables;
            png_structp m_pngPtr = nullptr;
            png_infop m_infoPtr = nullptr;
            png_bytep *m_rowBufferPtrs = nullptr;
            void _copyConstructor(const Canvas& rhs);
    };

    #if DEFAULT_DRAWING_FUNCS
    void rect_filled(Drawing::Drawable *drawable, Drawing::Canvas* canvas);
    void triangle_filled(Drawing::Drawable *drawable, Drawing::Canvas* canvas);
    #endif
}