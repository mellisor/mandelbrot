#include "ros/ros.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include "geometry_msgs/Twist.h"
#include "sensor_msgs/LaserScan.h"
#include <cstdlib> // Needed for rand()
#include <ctime> // Needed to seed random number generator with a time value
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <tf/transform_listener.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

using namespace boost::posix_time;

struct pose {
    float x;
    float y;
    float lv;
    float th;
};

class ParticleFilter {
    public:
        ParticleFilter(ros::NodeHandle& nh, int height, int width, int n, float nl, float na) : canvas(height,width,CV_8UC1) {
            srand(time(NULL));
            commandPub =  nh.subscribe("cmd_vel", 1, \
                &ParticleFilter::velCallback, this);
        
            // Create resizeable named window
            cv::namedWindow("Particle Filter", \
            CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
            // Create samples
            ps = new pose[n];
            samples = n;
            // Initialize samples
            for(int i = 0; i < n; i++) {
                ps[i].x = 0;
                ps[i].y = 0;
                ps[i].lv = 0;
                ps[i].th = 0;
            }
            // Set noise values
            lvG = nl;
            thG = na;
            lastPub = ros::Time::now();
        };

        void velCallback(const geometry_msgs::Twist::ConstPtr& msg) {
            // Get dt by finding subtracting current time from last callback time
            double dt = (ros::Time::now() - lastPub).toSec();
            lastPub = ros::Time::now();
            for(int i = 0; i < samples; i++) {
                // remove old samples
                plot(ps[i].x*5,ps[i].y*5,0);
                // Update samples position
                ps[i].x += ps[i].lv*cos(ps[i].th)*dt;
                ps[i].y += ps[i].lv*sin(ps[i].th)*dt;
                // Introduce some linear and angular noise to each point
                float lvNoise = ((rand() % 100) / 100.0)*lvG*2 - lvG;
                float thNoise = ((rand() % 100) / 100.0)*thG*2 - thG;
                ps[i].lv = msg->linear.x + lvNoise;
                ps[i].th += (msg->angular.z + thNoise)*dt;
                std::cout << ps[i].th << std::endl;
                // Place new samples
                plot(ps[i].x*5,ps[i].y*5,255);
            } 
        };

        // Update grayscale intensity on canvas pixel (x, y) (in robot coordinate frame)
        void plot(int x, int y, char value) {
            canvasMutex.lock();
            x+=canvas.rows/2;
            y+=canvas.cols/2;
            if (x >= 0 && x < canvas.rows && y >= 0 && y < canvas.cols) {
            canvas.at<char>(x, y) = value;
            }
            canvasMutex.unlock();
        };

        void spin() {
            ros::Rate scan_freq(10);
            int key = 0;
            // Show the canvas
            while( ros::ok ) {
                cv::imshow("Particle Filter", canvas);
                ros::spinOnce(); // Need to call this function often to allow ROS to process incoming messages
                key = cv::waitKey(1000/SPIN_RATE_HZ); // Obtain keypress from user; wait at most N milliseconds
                if (key == 'x' || key == 'X') {
                    break;
                }
                else if (key == ' ') {
                    saveSnapshot();
                }
            }
            ros::shutdown();
        };

        void saveSnapshot() {
            std::string filename = "filter" + to_iso_string(second_clock::local_time()) + ".jpg";
            canvasMutex.lock();
            cv::imwrite(filename, canvas);
            canvasMutex.unlock();
        };
    constexpr static int SPIN_RATE_HZ = 30;

    protected:
        ros::Subscriber commandPub; // Publisher to the current robot's velocity command topic
        pose *ps;
        int samples;
        cv::Mat canvas; // Occupancy grid canvas
        boost::mutex canvasMutex; // Mutex for occupancy grid canvas object
        float lvG; // Linear velocity noise
        float thG; // Angluar velocity noise
        ros::Time lastPub;
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "particle_filter");
    ros::NodeHandle n;
    if(argc < 4) {
        std::cout << "Invalid number of input arguments\nUSAGE: ./*** SAMPLES LINEARNOISE ANGULARNOISE" << std::endl;
        return EXIT_FAILURE;
    }
    int smpl = atoi(argv[1]);
    float nl = atof(argv[2]);
    float na = atof(argv[3]);
    ParticleFilter phil(n,500,500,smpl,nl,na);
    phil.spin();
    return EXIT_SUCCESS;
}