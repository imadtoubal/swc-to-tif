#include <iostream>
#include <fstream>
#include <map>
#include <opencv2/opencv.hpp>

struct Point
{
    int x, y, z;
    float r;
};

std::vector<Point> readSWC(std::string fileName)
{
    std::vector<Point> points;
    std::string line, word;
    std::vector<std::string> row;

    std::ifstream file(fileName);
    if (file.is_open())
        while (std::getline(file, line))
        {
            if (line[0] == '#')
                continue;

            std::stringstream lineStream(line);
            std::string cell;
            std::vector<std::string> values;
            while (std::getline(lineStream, cell, ' '))
            {
                values.push_back(cell);
            }
            Point point;            
            point.x = std::round(std::stof(values[2]));
            point.y = std::round(std::stof(values[3]));
            point.z = std::round(std::stof(values[4]));
            point.r = std::stof(values[5]);
            points.push_back(point);
        }
    else
    {
        std::cout << "Unable to open SWC file";
        exit(1);
    }
    return points;
}

bool saveMultiPageTIFF(const cv::Mat& mask, std::string outfile)
{
    std::vector<cv::Mat> pages;
    pages.reserve(mask.size[0]);
    int h = mask.size[1], w = mask.size[2];
    for (int i = 0; i < mask.size[0]; i++)
    {

        cv::Range ranges[] = {
            cv::Range(i, i + 1),
            cv::Range::all(),
            cv::Range::all()};

        pages.push_back(mask(ranges).reshape(0, {h, w}));
    }

    bool success = cv::imwritemulti(outfile, pages);
    return success;
}

bool loadMultiPageTiffAs3D(std::string file, cv::Mat &outVolume, cv::Mat &outMask)
{

    std::vector<cv::Mat> tiffPages;
    bool success = cv::imreadmulti(file, tiffPages);
    if (!success)
    {
        std::cout << "Failed to load multi-page tiff file: " << file << std::endl;
        return false;
    }

    int zDim = tiffPages.size();
    int yDim = tiffPages[0].rows;
    int xDim = tiffPages[0].cols;
    int dtype = tiffPages[0].type();

    int dims[] = {zDim, yDim, xDim};

    outVolume = cv::Mat(3, dims, dtype);
    // TODO: Experiment with cv::SparseMat
    outMask = cv::Mat(3, dims, dtype);

    for (int i = 0; i < zDim; i++)
    {
        for (int r = 0; r < yDim; r++)
        {
            for (int c = 0; c < xDim; c++)
            {
                outVolume.at<uint8_t>(i, r, c) = tiffPages[i].at<uint8_t>(r, c);
                outMask.at<uint8_t>(i, r, c) = 0;
            }
        }
    }

    return true;
}

void brush3D(cv::Mat &image, int x, int y, int z, float diameter)
{
    float radiusSqr = 0.25 * diameter * diameter;

    for (int i = x - diameter; i < x + diameter; i++)
    {
        for (int j = y - diameter; j < y + diameter; j++)
        {
            for (int k = z - diameter; k < z + diameter; k++)
            {
                int distSqr = (i - x) * (i - x) + (j - y) * (j - y) + (k - z) * (k - z);
                if (distSqr <= radiusSqr)
                {
                    image.at<uint8_t>(k, j, i) = 255;
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    std::string inputImage, inputSWC, outputFile;

    if (argc != 4)
    {
        std::cout << "Usage: \n\t" << argv[0]
                  << " <input image> <input swc> <output tiff>\n";
        exit(1);
    }

    inputImage = argv[1];
    inputSWC = argv[2];
    outputFile = argv[3];

    cv::Mat image, mask;
    bool success = loadMultiPageTiffAs3D(inputImage, image, mask);
    if (!success)
    {
        printf("No image data \n");
        return -1;
    } 
    
    printf("Loaded image data \n");

    std::vector<Point> points = readSWC(inputSWC);
    printf("Loaded %lu points \n", points.size());

    for (Point point : points)
    {
        brush3D(mask, point.x, point.y, point.z, point.r * 2);
    }

    if(!saveMultiPageTIFF(mask, outputFile)) {
        printf("Failed to save tiff file \n");
        return -1;
    }
    std::cout << "Saved to: " << outputFile << std::endl;
    return 0;
}
