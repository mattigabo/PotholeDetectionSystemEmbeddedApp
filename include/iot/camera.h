//
// Created by Xander on 26/9/2018.
//

#ifndef POTHOLEDETECTIONOBSERVER_CAMERA_H
#define POTHOLEDETECTIONOBSERVER_CAMERA_H

#include <opencv2/core.hpp>

namespace phd::iot::camera {

    cv::Mat fetch(int device_id);

}

#endif //POTHOLEDETECTIONOBSERVER_CAMERA_H
