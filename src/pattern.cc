/*
 * Copyright (c) 2009-2013 Javier G. Orlandi <javiergorlandi@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

//#include <png++/png.hpp>
#include <QImage>
#include "pattern.h"

Pattern::Pattern(double x, double y, double wSize, double hSize)
{
    init();
    origin.x() = x;
    origin.y() = y;
    width = wSize;
    height = hSize;
}

Pattern::Pattern(double wSize, double hSize)
{
    init();
    width = wSize;
    height = hSize;
    origin.x() = -width/2.;
    origin.y() = height/2.;
}

void Pattern::init()
{
    pattern = NULL;
    drawMode = PATTERN_DRAW_MODE_FILL;

    backgroundColor[0] = backgroundColor[1] = backgroundColor[2] = .9;
//    backgroundColor[0] = backgroundColor[1] = 0.6;
    patternColor[0] = patternColor[1] = patternColor[2] = 0.;
//    patternColor[2] = 0.2;
}

void Pattern::loadPatternFromFile(std::string file)
{
    //png::image< png::gray_pixel_1 > image(file, png::require_color_space< png::gray_pixel_1 >());
    QImage image = QImage(file.c_str());
    if(image.isNull())
    {
        std::cout << "Error. Could not load the pattern file: " << file << "\nQuitting...\n";
        exit(1);
    }
    fileName = file;

    widthCount = image.width();
    heightCount = image.height();

    // Pixel size vector (spacing) - if height <=0 conserve the aspect ratio of the image
    if(height <= 0)
    {
       height = width*double(heightCount)/double(widthCount);
       origin.y() = height/2.;
    }
    unitSize = Vector2d(width/double(widthCount), height/double(heightCount));

    pattern = new bool*[widthCount];
    for(size_t i = 0; i < widthCount; i++)
       pattern[i] = new bool[heightCount];

    // get_pixel reads (x,y)
    QRgb col;
    int ncol;
    for(size_t x = 0; x < widthCount; x++)
        for(size_t y = 0; y < heightCount; y++)
        {
            col = image.pixel(x,y);
//            pattern[x][y] = image.get_pixel(x,y);
            if(qGray(col) > 0)
                pattern[x][y] = true;
            else
                pattern[x][y] = false;
        }
}

void Pattern::createEmptyPattern(size_t wCount, size_t hCount)
{
    widthCount = wCount;
    heightCount = hCount;

    // Pixel size vector (spacing) 
    unitSize = Vector2d(width/double(widthCount), height/double(heightCount));

    pattern = new bool*[widthCount];
    for(size_t i = 0; i < widthCount; i++)
       pattern[i] = new bool[heightCount];

    for(size_t h = 0; h < heightCount; h++)
        for(size_t w = 0; w < widthCount; w++)
            pattern[w][h] = false;
}

Vector2d Pattern::getPosition(int x, int y)
{
    return origin+Vector2d(unitSize.x()*x, -unitSize.y()*y);
}

