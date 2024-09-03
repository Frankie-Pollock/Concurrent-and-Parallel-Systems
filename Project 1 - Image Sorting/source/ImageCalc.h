#pragma once
#include <vector>
namespace ImageCalc {
    float mapHue(float hue);
    float calculateMedianHue(const std::vector<unsigned char>& imageDataVec);    
}
