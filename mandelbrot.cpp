#include <cstdlib> // Needed for rand()
#include <ctime> // Needed to seed random number generator with a time value
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdlib.h>
#include <vector>

class Mandelbrot {
    public: 
        Mandelbrot(int aHeight, int aWidth) : canvas(aHeight,aWidth,CV_8UC3,cv::Scalar(0,0,0)) {
            height = aHeight;
            width = aWidth;
            min_x = -2.0;
            max_x = .5;
            min_y = -1.25;
            max_y = 1.25;
            iterations = 100;
            x_step = (max_x - min_x) / width;
            y_step = (max_y - min_y) / height;
        }

        cv::Vec2f toCoords(int x, int y) {
            float x_coord = min_x + x*x_step;
            float y_coord = max_y - y*y_step;
            return cv::Vec2f(x_coord,y_coord);
        }

        int mandelbrot(float x, float y) {
            std::complex<float> c(x,y);
            std::complex<float> z(0.0,0.0);
            int i;
            for(i = 0; i < iterations && abs(z) < 2.0; i++) {
                z = z*z + c;
            }
            if(i == iterations) {
                return 255;
            }
            return i;
        }

        void doMandelbrot() {
            for(int y = 0; y < height; y++) {
                float y_coord = max_y - y*y_step;
                for(int x = 0; x < width; x++) {
                    float x_coord = min_x + x*x_step;
                    int i = mandelbrot(x_coord,y_coord);
                    int r = i*8 > 255 ? 255 : i*8;
                    int g = i*2 > 255 ? 255 : i*2;
                    int b = i > 255 ? 255 : i;
                    canvas.at<cv::Vec3b>(y,x) = cv::Vec3b(b,g,r);
                }
            }
            cv::imshow("image",canvas);
            cv::waitKey(0);
        }

        void saveSnapshot() {
            std::string filename = "test.jpg";
            cv::imwrite(filename, canvas);
        };

    protected:
        int height;
        int width;
        int iterations;
        float min_x;
        float max_x;
        float min_y;
        float max_y;
        float x_step;
        float y_step;
        cv::Mat canvas;
};

int main(int argc, char** argv) {
    Mandelbrot m(atoi(argv[1]), atoi(argv[2]));
    m.doMandelbrot();
    m.saveSnapshot();
    return 0;
}