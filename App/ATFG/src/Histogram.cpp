///////////////////////////////////////////////////////////////////////////////
// histogram.cpp
// =============
// generate histogram and equalize the histogram to enhance image contrast
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2006-09-25
// UPDATED: 2006-09-25
///////////////////////////////////////////////////////////////////////////////

#include "Histogram.h"
#include <cstring>                      // for memset
#include <iostream>
using namespace std;


///////////////////////////////////////////////////////////////////////////////
// generate histogram of an 8-bit greyscale image
///////////////////////////////////////////////////////////////////////////////
bool dip::getHistogram(const unsigned char *image, int width, int height, unsigned int *histo, int histoSize)
{
    // check NULL pointer
    if(!image || !histo)
        return false;

    int i;
    int imageSize = width * height;

    // backup the current addresses
    const unsigned char* imagePtr = image;
    unsigned int* histoPtr = histo;

    // clear the array before accumulate
    memset(histo, 0, histoSize*sizeof(int));

    // traverse all pixels and accumulate the count of same values
    i = 0;
    while(i++ < imageSize)
    {
        ++*(histo + *image++);          // histo[image[i]]++;
    }

    // restore pointer addresses
    histo = histoPtr;
    image = imagePtr;
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// transform 8-bit input image by using histogram equalization
// The input pixels are remapped using the look-up table. The look-up table is
// generated from the cumulative histogram.
///////////////////////////////////////////////////////////////////////////////
bool dip::equalizeHistogram(const unsigned char *in, int width, int height, unsigned char *out)
{
    // check pointers
    if(!in || !out) return false;

    const int HISTOGRAM_SIZE = 256;         // for 8-bit image
    const float MAX_VALUE = 255.0f;         // max value in 8-bit image

    int i;
    unsigned int sum;
    int imageSize = width * height;
    float scale = MAX_VALUE / imageSize;    // scale factor ,so the values in LUT are from 0 to MAX_VALUE
    unsigned int* histogram = 0;            // histogram bin
    unsigned char* lut = 0;                 // look-up table

    // allocate histogram bin and look-up table
    histogram = new unsigned int[HISTOGRAM_SIZE];
    lut = new unsigned char[HISTOGRAM_SIZE];

    // get histogram
    dip::getHistogram(in, width, height, histogram, HISTOGRAM_SIZE);

    i = 0;
    sum = 0;
    while(i < HISTOGRAM_SIZE)
    {
        // cumulative sum is used as LUT
        sum += histogram[i];

        // build look-up table
        lut[i] = (unsigned char)(sum * scale);
        ++i;
    }

    // re-map input pixels by using LUT
    i = 0;
    while(i < imageSize)
    {
        out[i] = lut[in[i]];
        ++i;
    }

    // decallocate memory
    delete [] histogram;
    delete [] lut;
    return true;
}



///////////////////////////////////////////////////////////////////////////////
// generate cumulative histogram of an 8-bit greyscale image
///////////////////////////////////////////////////////////////////////////////
bool dip::getSumHistogram(const unsigned char *image, int width, int height, unsigned int *sumHisto, int histoSize)
{
    // check NULL pointer
    if(!image || !sumHisto)
        return false;

    // allocate temp histogram bin
    unsigned int *histo = new unsigned int[histoSize];

    // get histogram
    dip::getHistogram(image, width, height, histo, histoSize);

    // clear the array before accumulate
    memset(sumHisto, 0, histoSize*sizeof(int));

    // accumulate sum
    int i = 0;
    unsigned int sum = 0;
    while(i < histoSize)
    {
        sum += histo[i];
        sumHisto[i] = sum;
        ++i;
    }

    delete [] histo;
    return true;
}



