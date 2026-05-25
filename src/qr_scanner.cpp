#include <qrMqtt/qr_scanner.hpp>

#include <chrono>
#include <stdexcept>
#include <thread>

#include <opencv2/imgproc.hpp>

namespace qrmqtt
{
QrScanner::QrScanner(const Config::Camera &config)
  : config_(config), camera_(config.device_index)
{
    if (!camera_.isOpened())
    {
        throw std::runtime_error("Cannot open camera device " + std::to_string(config.device_index));
    }
    scanner_.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
}

std::string QrScanner::next(std::atomic<bool> &running)
{
    while (running.load())
    {
        cv::Mat frame;
        if (!camera_.read(frame))
        {
            throw std::runtime_error("Cannot read a frame from the camera.");
        }

        cv::Mat grey;
        cv::cvtColor(frame, grey, cv::COLOR_BGR2GRAY);
        zbar::Image image(grey.cols, grey.rows, "Y800", grey.data,
                          static_cast<unsigned long>(grey.total()));
        scanner_.scan(image);
        for (auto symbol = image.symbol_begin(); symbol != image.symbol_end(); ++symbol)
        {
            return symbol->get_data();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(config_.frame_delay_ms));
    }
    return "";
}
}
