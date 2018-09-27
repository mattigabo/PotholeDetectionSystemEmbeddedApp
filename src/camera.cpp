//
// Created by Xander on 26/9/2018.
//

#include "iot/camera.h"

namespace phd::iot::camera {
    cv::Mat fetch() {
        return cv::Mat1b(1, 1, CV_32SC3);
    }
}