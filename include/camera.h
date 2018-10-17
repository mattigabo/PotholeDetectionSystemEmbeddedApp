//
// Created by Xander on 26/9/2018.
//

#ifndef POTHOLEDETECTIONEMBEDDEDAPP_CAMERA_H
#define POTHOLEDETECTIONEMBEDDEDAPP_CAMERA_H

#include <opencv2/core.hpp>

namespace phd::devices::camera {

    cv::Mat fetch(int device_id);

}

#endif //POTHOLEDETECTIONEMBEDDEDAPP_CAMERA_H
