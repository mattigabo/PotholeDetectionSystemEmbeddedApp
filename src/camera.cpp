//
// Created by Xander on 26/9/2018.
//

#include "camera.h"
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

namespace phd::devices::camera {

    cv::Mat fetch(const int device_id) {

        cv::VideoCapture pi_camera;
        cv::Mat capture;

        pi_camera.open(device_id);

        if (pi_camera.isOpened()) {
            pi_camera.set(cv::CAP_PROP_FRAME_WIDTH,640);
            pi_camera.set(cv::CAP_PROP_FRAME_WIDTH,480);

            pi_camera.read(capture);
        } else {
            std::cerr << "Cannot open Raspi Camera" << std::endl;
            exit(-1);
        }

        pi_camera.release();

        return capture;
    }
}