#include <vector>
#include <png.h>
#include <cassert>
#include <cmath>


#define DEFAULT_DRAWING_SHAPE_FUNCS 1


namespace Drawing{

    struct Color{
        Color(double r, double g, double b, double a) {
            red = r;
            green = g;
            blue = b;
            alpha = a;
        }
        
        double red;
        double green;
        double blue;
        double alpha;
    };


    typedef struct Point { 
        Point(double x, double y){
            this->x = x;
            this->y = y;
        }

        double x, y; 
    } Point;


    class Drawable;

    typedef int (*shape_fn_ptr)(const Drawable *drawable, double x, double y);

    struct Drawable{
        Drawable(void) {}
        Drawable(Color *color, shape_fn_ptr shape_fn,
            double lineWidth, std::vector<Point> points){
            this->color = color;
            this->shape_fn = shape_fn;
            this->lineWidth = lineWidth;
            this->points = points;
        }

        const struct Color *color;
        shape_fn_ptr shape_fn;
        double lineWidth; /* line width, 0 for 'filled' */
        std::vector<Point> points;
    };


    class Canvas{
        public:
            Canvas(void) {};
            Canvas(png_uint_32 width, png_uint_32 height);
            ~Canvas();
            
            void initImage(const png_uint_32 width, const png_uint_32 height);
            void initBuffer();

            void addDrawable(const Drawable &drawable);
            void draw();

            void bufferToFile(const char* filepath);

            std::size_t getDrawablesSize(void) const { return m_drawables.size(); }
            Drawable& getDrawable(const unsigned index) { return m_drawables.at(index); }

        private:
            std::vector<Drawable> m_drawables;
            png_uint_16p m_buffer;
            png_image m_image;
    };



#if DEFAULT_DRAWING_SHAPE_FUNCS

    static int shape_rect_filled(const Drawing::Drawable *drawable, double x, double y){
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

    static int shape_triangle_filled(const Drawing::Drawable *drawable, double x, double y){
        assert(drawable->points.size() >= 3);
        return _isPointInTriangle(Point(x, y), drawable->points[0], drawable->points[1], drawable->points[2]);
    }

#endif
}