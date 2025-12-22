/*
  MIT License
  
  Copyright (c) 2025 Stefan Mader
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef __T_LAYERS_HPP__
#define __T_LAYERS_HPP__

#include "HASHMAP.HPP"

/**
* A class for raw RGBA image data.
*/
class BitmapLayer {

public:

  /// The raw RGBA image data.
  unsigned int *data;
  /// The width(w) and height(h) of the image data.
  int w, h;

  /**
  * Constructor initializing the image with NULL.
  */
  BitmapLayer();

  /**
  * Constructor initializing the image with a memory representation of an rgba image.
  *
  * @param rgba The rgba memory representation of the bitmap pixels.
  * @param width The width of the image/bitmap.
  * @param height The height of the image/bitmap.
  * @example BitmapLayer *b = new BitmapLayer(pixels,1024,1024)
  */
  BitmapLayer(unsigned int *rgba, unsigned int width, unsigned int height);

  /**
  * Destructor, if data != NULL it deletes it and sets it to NULL.
  */  
  ~BitmapLayer();

  /**
  * Sets the layer to a memory representation of an rgba image.
  *
  * @param rgba The rgba memory representation of the bitmap pixels.
  * @param width The width of the image/bitmap.
  * @param height The height of the image/bitmap.
  * @example BitmapLayer *b = new BitmapLayer(pixels,1024,1024)
  */
  void set(unsigned int *rgba, unsigned int width, unsigned int height);

};

/**
* A class for holding bitmap layers, which may be used to represent a terrain, or level.
*/
class BitmapLayers {

public:

  /// The actual bitmap layers. Each layer with a name and rgba data.
  HashMap<String, BitmapLayer> layers;

  /**
  * Constructor of BitmapLayers.
  */
  BitmapLayers();

  /**
  * Destructor of BitmapLayers.
  */
  ~BitmapLayers();

  /**
  * A function to clear the current bitmap layer data.
  *
  * @example layers->free();
  */
  void free();

  /**
  * A function to load / add a Photoshop PSD with it's layers to the layers.
  *
  * @param name The filename of the PSD to load.
  * @param lowerCase This may be used to have all loaded layers with lower case names.
  * @example layers->loadPSD("map.psd");
  */
  void loadPSD(const char *name, bool lowerCase = true);
};

/**
* This function uses the alpha channel of a source pixel to fade this source pixel onto a dest pixel.
*
* @param d The destination 32bit rgba pixel.
* @param s The source 32bit rgba pixel with it's alpha channel in the upperst 8 bit.
* @param alphaScale This may be used to decrease the influence of the source pixels alpha channel. Valid values are 0.f .. 1.f. Increasing alpha is not possible due to no clamping.
* @example alpha(map[0],map[1]);
*/
void alpha(unsigned int &d, unsigned int s, float alphaScale = 1.f);

/**
* This function uses the alpha channel of a source pixel to maybe clear the destination pixel (with 0). The alpha channel is checked by a threshold.
*
* @param d The destination 32bit rgba pixel.
* @param s The source 32bit rgba pixel with it's alpha channel in the upperst 8 bit.
* @thresh a value from 0 .. 255 defining the alpha threshold
* @example remove(map[0],color,128);
*/
void remove(unsigned int &d, unsigned int s, int thresh);

/**
* This function downsamples a byte image by a factor using INTER_AREA filtering, thus doing some "advanced sort of mipmapping" so to say.
*
* @param data A pointer to the byte array pointer. The pointer gets modified by this function to a new byte image.
* @param w A pointer to the width of the source image. The width gets modified with the new width.
* @param h A pointer to the height of the source image. The height gets modified with the new height.
* @param div The value by which to divide the image size. Values below 1 result in nearest point filtered bigger image. Use other filters/functions for this <1 values.
* @example downsample(&pixels, &width, &height, 2.f);
*/
void downsample(unsigned char **data, int *w, int *h, float div);

#endif //__T_LAYERS_HPP__
