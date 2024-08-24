

#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>


int main(int, char**)
{
    std::string image_path = cv::samples::findFile("../img/forest.jpg");
    cv::Mat img = cv::imread(image_path, cv::IMREAD_GRAYSCALE);
    cv::Mat rev_img = cv::Mat();

    if(img.empty())
    {
        std::cout << "Could not read the image: " << image_path << std::endl;
        return 1;
    }

    rev_img.reserveBuffer(img.elemSize());

    cv::flip(img, rev_img, 1);

// //
//     // Get the screen resolution
//     cv::Size screenResolution = cv::Size(1920, 1080);

//     // Calculate the aspect ratio of the original image
//     double aspectRatio = static_cast<double>(img.cols) / img.rows;

//     int newWidth, newHeight;
//     if (img.cols > screenResolution.width || img.rows > screenResolution.height) {
//         if (img.cols > img.rows) {
//             newWidth = screenResolution.width;
//             newHeight = static_cast<int>(newWidth / aspectRatio);
//         } else {
//             newHeight = screenResolution.height;
//             newWidth = static_cast<int>(newHeight * aspectRatio);
//         }
//     } else {
//         newWidth = img.cols;
//         newHeight = img.rows;
//     }
//

    // cv::Mat resizedImage;
    // cv::resize(rev_img, resizedImage, cv::Size(newWidth, newHeight));

    // cv::resizeWindow("Display window", newWidth, newHeight);

    cv::namedWindow("Display window", cv::IMREAD_GRAYSCALE); // Create a window for display.
    cv::imshow("Display window", rev_img);
    int k = cv::waitKey(0); // Wait for a keystroke in the window

    return 0;
}