#include <vector>
#include <png.h>

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

    class Drawable{
        public:
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

        private:
            std::vector<Drawable> m_drawables;
            png_uint_16p m_buffer;
            png_image m_image;
    };
}