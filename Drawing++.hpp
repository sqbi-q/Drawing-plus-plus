#include <vector>
#include <png.h>
#include <cassert>
#include <cmath>
#include <iostream>


#define DEFAULT_DRAWING_SHAPE_FUNCS 1


namespace Drawing{

    struct Color{
        Color(void) {};
        Color (double r, double g, double b, double a){
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
        double r, g, b, a;
    };



    typedef struct Point { 
        Point(double x, double y){
            this->x = x;
            this->y = y;
        }
        Point(double xy){
            this->x = xy;
            this->y = xy;
        }
        Point operator+(const Point &p) const {
            return Point(p.x+x, p.y+y);
        }
        Point operator-(const Point &p) const {
            return Point(p.x-x, p.y-y);
        }
        Point operator*(const Point &p) const {
            return Point(p.x*x, p.y*y);
        }
        Point operator/(const Point &p) const {
            return Point(p.x/x, p.y/y);
        }
        double x, y; 
    } Point;



    struct Drawable;
    using shape_fn_ptr = int(*)(Drawing::Drawable* drawable, png_uint_32 x, png_uint_32 y);



    struct Drawable{
        Drawable(void) {};
        shape_fn_ptr shape_fn = nullptr;
        std::vector<Point> points;
        virtual Color getPixel(png_uint_32 x, png_uint_32 y) { return Color(0, 0, 0, 0); }
    };



    struct Figure : Drawable {
        Figure(void) {};
        Figure(Color color, shape_fn_ptr shape_fn, std::vector<Point> points){
            this->color = color;
            this->shape_fn = shape_fn;
            this->points = points;
        }

        Color color;
        Color getPixel(png_uint_32 x, png_uint_32 y) { return color; }
    };



    struct ImageFile : Drawable {
        public:
            ImageFile(void) {};
            ImageFile(const char* filename);

            void loadPNGFile(const char* filename);
            Color getPixel(png_uint_32 x, png_uint_32 y);
                    
        private:
            png_bytep *m_rowBufferPtrs = nullptr;
    };



    class Canvas{
        public:
            Canvas(void) {};
            // Canvas(const Canvas& rhs);
            Canvas(png_uint_32 width, png_uint_32 height,
                int bitDepth = 8, int colorType = PNG_COLOR_TYPE_RGBA,
                int interlaceMethod = PNG_INTERLACE_NONE,
                int compressMethod = PNG_COMPRESSION_TYPE_DEFAULT,
                int filterMethod = PNG_FILTER_TYPE_DEFAULT);
            ~Canvas();

            /*
                Unto thy, who is't shall command assign unto ye `Canvas(void) {}` default constructeth'r:
                Doth not t, thee shall suff'r by `operator=` and swap function
            */

            // Canvas& operator=(Canvas rhs){
            //     std::cout << "TEST" << std::endl;
            //     assert(rhs.m_pngPtr != nullptr);
            //     assert(rhs.m_infoPtr != nullptr);
            //     assert(rhs.m_rowBufferPtrs != nullptr);

            //     png_uint_32 width = png_get_image_width(rhs.m_pngPtr, rhs.m_infoPtr);
            //     png_uint_32 height = png_get_image_height(rhs.m_pngPtr, rhs.m_infoPtr);

            //     initImage(rhs.m_pngPtr, rhs.m_infoPtr);
                
            //     initBuffer();
            //     *m_rowBufferPtrs = *rhs.m_rowBufferPtrs;
            //     for(int y = 0; y < height; y++) {
            //         *(m_rowBufferPtrs[y]) = *(rhs.m_rowBufferPtrs[y]);
            //     }

            //     return *this;
            // }

            // Canvas& operator=(Canvas rhs){
            //     assert(rhs.m_pngPtr != nullptr);
            //     assert(rhs.m_infoPtr != nullptr);
            //     assert(rhs.m_rowBufferPtrs != nullptr);

            //     png_uint_32 width = png_get_image_width(rhs.m_pngPtr, rhs.m_infoPtr);
            //     png_uint_32 height = png_get_image_height(rhs.m_pngPtr, rhs.m_infoPtr);

            //     initImage(rhs.m_pngPtr, rhs.m_infoPtr);
                
            //     initBuffer();
            //     *m_rowBufferPtrs = *rhs.m_rowBufferPtrs;
            //     for(int y = 0; y < height; y++) {
            //         *(m_rowBufferPtrs[y]) = *(rhs.m_rowBufferPtrs[y]);
            //     }

            //     return *this;
            // }
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

            void addDrawable(Drawable *const &drawable){
                m_drawables.push_back(drawable);
            }
            void draw();

            double compare(Canvas &canvasB);

            void bufferToFile(const char* filepath);

            std::size_t getDrawablesSize(void) const { return m_drawables.size(); }
            Drawing::Drawable* getDrawable(const unsigned index) { return m_drawables.at(index); }
            void setDrawable(Drawable* drawable, const unsigned index) { m_drawables[index] = drawable; }

        private:
            std::vector<Drawable*> m_drawables;
            png_structp m_pngPtr = nullptr;
            png_infop m_infoPtr = nullptr;
            png_bytep *m_rowBufferPtrs = nullptr;
    };



#if DEFAULT_DRAWING_SHAPE_FUNCS

    static int shape_rect_filled(Drawing::Drawable* drawable, png_uint_32 x, png_uint_32 y){
        assert(drawable->points.size() >= 2);
        return x >= drawable->points[0].x && x <= drawable->points[1].x &&
                y >= drawable->points[0].y && y <= drawable->points[1].y;
    }


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

#endif
}