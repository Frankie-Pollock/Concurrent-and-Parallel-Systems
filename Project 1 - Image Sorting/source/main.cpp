#define STB_IMAGE_IMPLEMENTATION  
#include <SFML/Graphics.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <future>
#include "stb_image.h"
#include <mutex>
#include <vector>
#include <iostream> 
#include "ImageCalc.h"
#include <fstream>
#include <iomanip>
#include <chrono>

namespace fs = std::filesystem;

// Define a structure to hold image data
struct Image {
    std::string filePath; // The file path of the image
    std::vector<unsigned char> data; // The raw image data
};

std::mutex imageMutex; // A mutex for syncing access to shared image data

// Function to calculate the scaling factor for an image based on screen dimensions
sf::Vector2f ScaleFromDimensions(const sf::Vector2u& textureSize, int screenWidth, int screenHeight)
{
    // Calculate the scaling factors for both width and height
    float scaleX = screenWidth / float(textureSize.x);
    float scaleY = screenHeight / float(textureSize.y);

    // Determine the minimum scaling factor to maintain aspect ratio
    float scale = std::min(scaleX, scaleY);

    // Return the scaling factors as a 2D vector
    return { scale, scale };
}

// Function to output performance to a csv file
void outputPerf(const std::string& processName, const std::chrono::system_clock::time_point& startTime, const std::chrono::system_clock::time_point& endTime) {
    // Calculate the execution time in seconds
    std::chrono::duration<double> executionTime = endTime - startTime;

    // Open a CSV file for writing
    std::ofstream outputFile("performance.csv", std::ios::app); // Appending to an existing file

    // Check if the file opened successfully
    if (outputFile.is_open()) {
        // Write the process name and execution time in seconds to CSV file
        outputFile << "Name of process: " << processName << "," << executionTime.count() << "\n";

        // Close the file
        outputFile.close();
    }
    else {
        // Error handling
        std::cerr << "Error: Unable to open the file for writing.\n";
    }
}

// Function to load image data
Image getImgData(const std::string& filePath) {
    int width, height, n;
    // Load image data from file using stbi_load function
    unsigned char* imageData = stbi_load(filePath.c_str(), &width, &height, &n, 0);
    // Check if image data was loaded
    if (imageData) {
        // Convert to a vector
        std::vector<unsigned char> imageDataVec(imageData, imageData + width * height * n);
        // Create an Image object to hold the image data and file path
        Image imgData;
        imgData.filePath = filePath;
        imgData.data = imageDataVec;
        // Free the memory allocated for the loaded image data
        stbi_image_free(imageData);
        return imgData; // Return
    }
    return Image(); // Return 
}

// Function to load images 
void t_lImage(const std::vector<std::string>& imageFilenames, std::vector<Image>& imageDataList) {
    for (const auto& filename : imageFilenames) {
        Image imgData = getImgData(filename);
        // Lock guard for thread protection on concurrent acess to ImageDataList
        std::lock_guard<std::mutex> lock(imageMutex);
        imageDataList.push_back(imgData);
    }
}

// Function to sort images based on hue values
void sortImagesByHue(std::vector<std::pair<float, std::string>>& sortedImages) {
    std::sort(sortedImages.begin(), sortedImages.end(), std::less<>());
}

// Function to handle the user interface
void UIHandler(const std::vector<Image>& imageDataList)
{
    // Define constants for the game window dimensions
    const int gameWidth = 800;
    const int gameHeight = 600;

    // Initialize variables
    int imageIndex = 0;

    // Create a SFML window with specified dimensions and title
    sf::RenderWindow window(sf::VideoMode(gameWidth, gameHeight, 32), "Image Fever",
        sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true); // Enable vertical sync to cap frame rate

    // Create a vector to store pairs of image hue values and their corresponding file paths
    std::vector<std::pair<float, std::string>> sortedImages;

    // Create SFML objects to handle textures and image data
    sf::Texture texture;

    // Create a vector with a future storing pairs of median hue and filename
    std::vector<std::future<std::pair<float, std::string>>> medianHueFutures;

    // Timer started for calculating time to get median hue
    auto tCalculateMedianHueStart = std::chrono::system_clock::now();

    // All images in list get pased to calculateMedianHue function and area assigned to a pair in future for later use
    for (const auto& image : imageDataList) {
        auto medianHueFuture = std::async(std::launch::async, [&image]() {
        float medianHue = ImageCalc::calculateMedianHue(image.data);
        return std::make_pair(medianHue, image.filePath);
    });
    medianHueFutures.push_back(std::move(medianHueFuture));
    }

    // For each feature stored in the vector, loop and add the hues with file path to sorted images
    for (size_t i = 0; i < imageDataList.size(); ++i) {
     auto result = medianHueFutures[i].get();
     sortedImages.push_back(result);
    }

    // End timer and output performance
    auto tCalculateMedianHueEnd = std::chrono::system_clock::now();
    outputPerf("Calculate Median Hue", tCalculateMedianHueStart, tCalculateMedianHueEnd);

    // Start timer for sorting hues
    auto tSortImagesByHueStart = std::chrono::system_clock::now();

    // Sort the images based on their hue values in parallel
    std::future<void> sortFuture = std::async(std::launch::async, sortImagesByHue, std::ref(sortedImages));

    // End timer and output performance
    auto tSortImagesByHueEnd = std::chrono::system_clock::now();
    outputPerf("Sort Images By Hue", tSortImagesByHueStart, tSortImagesByHueEnd);

    // Get the filename of the first image in the sorted list
    const auto& imageFilename = sortedImages[imageIndex].second;

    // Set the window title to display the index and filename of the current image
    window.setTitle(std::to_string(imageIndex + 1) + "_" + imageFilename);

    // Load the texture of the first image and update the sprite
    if (texture.loadFromFile(imageFilename))
    {
        sf::Sprite sprite(texture);
        sprite.setScale(ScaleFromDimensions(texture.getSize(), gameWidth, gameHeight));

        // Event loop
        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                // Check for events like window close or escape key press to quit the game
                if ((event.type == sf::Event::Closed) ||
                    ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
                {
                    window.close(); // Close the window and exit the loop
                    break;
                }

                // Handle window resize events
                if (event.type == sf::Event::Resized)
                {
                    // Create a view to keep the game aspect ratio consistent
                    sf::View view;
                    view.setSize(gameWidth, gameHeight);
                    view.setCenter(gameWidth / 2.f, gameHeight / 2.f);
                    window.setView(view);
                }

                // Handle key press events
                if (event.type == sf::Event::KeyPressed)
                {
                    // Change the currently displayed image based on arrow key presses
                    if (event.key.code == sf::Keyboard::Key::Left)
                        imageIndex = (imageIndex + sortedImages.size() - 1) % sortedImages.size();
                    else if (event.key.code == sf::Keyboard::Key::Right)
                        imageIndex = (imageIndex + 1) % sortedImages.size();

                    // Get the filename of the newly selected image
                    const auto& imageFilename = sortedImages[imageIndex].second;

                    // Update the window title to reflect the new image
                    window.setTitle(std::to_string(imageIndex + 1) + "_" + imageFilename);

                    // Load the texture of the newly selected image and update the sprite
                    if (texture.loadFromFile(imageFilename))
                    {
                        sprite = sf::Sprite(texture);
                        sprite.setScale(ScaleFromDimensions(texture.getSize(), gameWidth, gameHeight));
                    }
                }
            }

            // Clear the window and draw the current image sprite
            window.clear(sf::Color(0, 0, 0));
            window.draw(sprite);
            window.display();
        }
    }
}


// Entry point of the program
int main()
{
    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // Define the path to the folder containing the unsorted images
    const char* image_folder = "../unsorted";

    // Create a vector to store the file paths of the images
    std::vector<std::string> imageFilenames;

    // Iterate through the files in the specified folder
    for (auto& p : fs::directory_iterator(image_folder))
        imageFilenames.push_back(p.path().u8string());

    std::vector<Image> imageDataList;
    // Launch asynchronous threads to handle the UI
    std::future<void> uiThread = std::async(std::launch::async, UIHandler, std::cref(imageDataList));

    // Time how long it takes to complete Image Loading
    auto tLoadImagesStart = std::chrono::system_clock::now();
        t_lImage(imageFilenames, imageDataList);
    auto tLoadImagesEnd = std::chrono::system_clock::now();
        outputPerf("Load Images", tLoadImagesStart, tLoadImagesEnd);

    // Return a status indicating successful execution of the program
    return EXIT_SUCCESS;
}