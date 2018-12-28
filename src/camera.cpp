//
// Created by Xander on 26/9/2018.
//

#include "camera.h"
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video.hpp>

namespace phd {
    namespace devices {
        namespace camera {

            cv::Mat getDummyImage() {

                static cv::Mat dummy_img;
                static bool only_once = false;

                if (!only_once) {
                    dummy_img = cv::Mat(128, 128, CV_8UC3, cv::Scalar(1, 1, 1));
//                    std::cout << "OCV Image Type:" << dummy_img.type() << std::endl;
//                    std::cout << dummy_img.channels() << std::endl;
                    only_once = true;
                }

                return dummy_img;
            }

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
                    capture = getDummyImage();
                }

                pi_camera.release();

                return capture;
            }

        }
    }}