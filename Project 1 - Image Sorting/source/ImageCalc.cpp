#include "ImageCalc.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>

namespace ImageCalc {

    float mapHue(float hue) {
        // Is the hue within the range [0, 1]
        hue = fmod(hue, 1.0f);
        if (hue < 0.0f)
            hue += 1.0f;

        // Map hue values from warm to cool using formula y = frac(x + 1/6)
        return fmod(hue + 1.0f / 6.0f, 1.0f);
    }

    float calculateMedianHue(const std::vector<unsigned char>& imageDataVec) {
        // Ensure the image data is not empty
        if (imageDataVec.empty()) {
            return 0.0f; // Return 0 if there is no image data
        }

        // Calculate hue values via conversion to HSV
        std::vector<float> hues;
        hues.reserve(16);
        for (size_t i = 0; i < imageDataVec.size(); i += 3) {
            float r = imageDataVec[i] / 255.0f;
            float g = imageDataVec[i + 1] / 255.0f;
            float b = imageDataVec[i + 2] / 255.0f;
            // Find max RGB value, lowest and the difference between the two
            float max = std::max({ r, g, b });
            float min = std::min({ r, g, b });
            float diff = max - min;
            // find which channel is more domninant
            float hue = 0.0f;
            if (diff != 0.0f) {
                if (max == r) {
                    hue = (g - b) / diff + ((g < b) ? 6 : 0);
                }
                else if (max == g) {
                    hue = (b - r) / diff + 2;
                }
                else {
                    hue = (r - g) / diff + 4;
                }
                hue *= 60.0f;
                if (hue < 0.0f) {
                    hue += 360.0f; // Is the hue in [0, 360] range
                }
            }
            //  Push back
            hues.push_back(hue);
        }
        // Calculate median hue
        size_t size = hues.size();
        float medianHue = (size % 2 == 0) ?
            (hues[size / 2 - 1] + hues[size / 2]) / 2.0f :
            hues[size / 2];
        // Return value from function mapHue
        return mapHue(medianHue / 360.0f);
    }

}
