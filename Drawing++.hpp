#include <vector>
#include <png.h>
#include <memory>
#include <stdlib.h>
#include <cassert>
#include <cmath>
#include <algorithm>

#define MINMACRO(a, b) ( (a)<(b) ? (a) : (b) )

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

    static void filled_rect_func(Drawing::Drawable *drawable, Drawing::Canvas* canvas){
        for(png_uint_32 y=0; y<canvas->getHeight(); y++){
            for(png_uint_32 x=0; x<canvas->getWidth(); x++){
        
                if (x < drawable->points[0].x() || x > drawable->points[1].x() ||
                    y < drawable->points[0].y() || y > drawable->points[1].y()) 
                        continue;
                Drawing::Color pixel = drawable->getPixel(x, y);
                canvas->putPixel(x, y, pixel);
            }
        }
    }


    //https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
    //Code by Kornel Kisielewicz
    static float _sign(Point p1, Point p2, Point p3){
        return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
    }
    static bool _isPointInTriangle (Point pt, Point v1, Point v2, Point v3){
        float d1, d2, d3;
        bool has_neg, has_pos;

        d1 = _sign(pt, v1, v2);
        d2 = _sign(pt, v2, v3);
        d3 = _sign(pt, v3, v1);

        has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(has_neg && has_pos);
    }
    //

    static void shape_triangle_filled(Drawing::Drawable* drawable, Drawing::Canvas* canvas){
        assert(drawable->points.size() >= 3);

        for(png_uint_32 y=0; y<canvas->getHeight(); y++){
            for(png_uint_32 x=0; x<canvas->getWidth(); x++){

                if (_isPointInTriangle(Point({(double)x, (double)y}), 
                    drawable->points[0], drawable->points[1], drawable->points[2])){
                    
                    Drawing::Color pixel = drawable->getPixel(x, y);
                    canvas->putPixel(x, y, pixel);
                }
            }
        }
    }

#endif




// #if DEFAULT_DRAWING_SHAPE_FUNCS

//     static int shape_rect_filled(Drawable* drawable, png_uint_32 x, png_uint_32 y){
//         assert(drawable->points.size() >= 2);
//         return x >= drawable->points[0].x() && x <= drawable->points[1].x() &&
//                 y >= drawable->points[0].y() && y <= drawable->points[1].y();
//     }

//     //https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
//     //Code by Kornel Kisielewicz
//     static float _sign(Point p1, Point p2, Point p3){
//         return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
//     }

//     static bool _isPointInTriangle (Point pt, Point v1, Point v2, Point v3){
//         float d1, d2, d3;
//         bool has_neg, has_pos;

//         d1 = _sign(pt, v1, v2);
//         d2 = _sign(pt, v2, v3);
//         d3 = _sign(pt, v3, v1);

//         has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
//         has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

//         return !(has_neg && has_pos);
//     }
//     //

//     static int shape_triangle_filled(Drawing::Drawable* drawable, png_uint_32 x, png_uint_32 y){
//         assert(drawable->points.size() >= 3);
//         return _isPointInTriangle(Point({(double)x, (double)y}), 
//             drawable->points[0], drawable->points[1], drawable->points[2]);
//     }
// #endif
}