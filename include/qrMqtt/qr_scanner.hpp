#ifndef QRMQTT_QR_SCANNER_HPP
#define QRMQTT_QR_SCANNER_HPP

#include <atomic>
#include <string>

#include <opencv2/videoio.hpp>
#include <zbar.h>

#include <qrMqtt/config.hpp>

namespace qrmqtt
{
class QrScanner
{
  public:
    explicit QrScanner(const Config::Camera &config);
    std::string next(std::atomic<bool> &running);

  private:
    Config::Camera config_;
    cv::VideoCapture camera_;
    zbar::ImageScanner scanner_;
};
}

#endif
