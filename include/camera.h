//
// Created by Xander on 26/9/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CAMERA_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CAMERA_H

#include <opencv2/core.hpp>

namespace phd::devices::camera {

    cv::Mat fetch(int device_id);

}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CAMERA_H
