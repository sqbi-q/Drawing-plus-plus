#include <vector>
#include <png.h>
#include <memory>
#include <stdlib.h>
#include <cassert>
#include <cmath>

#define MINMACRO(a, b) ( (a)<(b) ? (a) : (b) )


#define DEFAULT_DRAWING_SHAPE_FUNCS 1


namespace Drawing {
    struct Color{
        Color (void) {}
        Color (double r, double g, double b, double a){
            this->r = MINMACRO(r, 1);
            this->g = MINMACRO(g, 1);
            this->b = MINMACRO(b, 1);
            this->a = MINMACRO(a, 1);
        }
        double r, g, b, a;
    };

    class Drawable;
    using shape_fn_ptr = int(*)(Drawable* drawable, unsigned x, unsigned y);

    //behold true beauty!!! 
    struct Point : std::vector<double> {
        Point(std::vector<double> coords) : std::vector<double>(coords) {}
        double x(void) const { return this->at(0); }
        double y(void) const { return this->at(1); }
        double z(void) const { return this->at(2); }
        double w(void) const { return this->at(3); }
    };


    class Drawable {
        public:
            virtual Color getPixel(unsigned x, unsigned y) = 0;
            virtual shape_fn_ptr getShapeFn(void) = 0;
            std::vector<Point> points;
    };



    class Figure : public Drawable {
        public:
            Figure (void) {};
            Figure (Color bgColor, shape_fn_ptr shape_fn, 
                std::vector<Point> points);
            
            Color getPixel(unsigned x, unsigned y) { return m_bgColor; }
            shape_fn_ptr getShapeFn(void) { return m_shape_fn; }

        private:
            Color m_bgColor;
            shape_fn_ptr m_shape_fn;
    };


    static int wholeShape(Drawable* drawable, unsigned x, unsigned y) { return 1; }
    class ImageFile : public Drawable {
        public:
            ImageFile(void) {};
            ImageFile(const char* filename);

            void loadPNGFile(const char* filename);
            Color getPixel(png_uint_32 x, png_uint_32 y);
            shape_fn_ptr getShapeFn(void) { return wholeShape; }
                    
        private:
            png_bytep *m_rowBufferPtrs = nullptr;
    };



    class Canvas {
        public:
            Canvas(void) {};
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

            template<typename T>
            void addDrawable(const T& drawable){
                m_drawables.push_back(
                    std::dynamic_pointer_cast<Drawable>(
                        std::make_shared<T>(drawable)
                ));
            }
            void draw();
        
            double compare(Canvas &canvasB);

            void bufferToFile(const char* filepath);

        private:
            std::vector<std::shared_ptr<Drawable>> m_drawables;
            png_structp m_pngPtr = nullptr;
            png_infop m_infoPtr = nullptr;
            png_bytep *m_rowBufferPtrs = nullptr;
    };










#if DEFAULT_DRAWING_SHAPE_FUNCS

    static int shape_rect_filled(Drawable* drawable, png_uint_32 x, png_uint_32 y){
        assert(drawable->points.size() >= 2);
        return x >= drawable->points[0].x() && x <= drawable->points[1].x() &&
                y >= drawable->points[0].y() && y <= drawable->points[1].y();
    }
/*

    //https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
    //Code by Kornel Kisielewicz
    static float _sign(Point p1, Point p2, Point p3){
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
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

    static int shape_triangle_filled(Drawing::Drawable* drawable, png_uint_32 x, png_uint_32 y){
        assert(drawable->points.size() >= 3);
        return _isPointInTriangle(Point(x, y), drawable->points[0], drawable->points[1], drawable->points[2]);
    }
*/
#endif
}