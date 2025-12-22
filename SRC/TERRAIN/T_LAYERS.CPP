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
#include "T_LAYERS.HPP"
#include "PSDIMAGE.HPP"

/**
* Constructor initializing the image with NULL.
*/
BitmapLayer::BitmapLayer() {
  data = NULL;
  w = h = 0;
}

/**
* Constructor initializing the image with a memory representation of an rgba image.
*
* @param rgba The rgba memory representation of the bitmap pixels.
* @param width The width of the image/bitmap.
* @param height The height of the image/bitmap.
* @example BitmapLayer *b = new BitmapLayer(pixels,1024,1024)
*/
BitmapLayer::BitmapLayer(unsigned int *rgba, unsigned int width, unsigned int height) {
  data = rgba;
  w = width;
  h = height;
}

/**
* Destructor, if data != NULL it deletes it and sets it to NULL.
*/  
BitmapLayer::~BitmapLayer() {
  if (data != NULL) {
    delete[] data;
    data = NULL;
  }
}

/**
* Sets the layer to a memory representation of an rgba image.
*
* @param rgba The rgba memory representation of the bitmap pixels.
* @param width The width of the image/bitmap.
* @param height The height of the image/bitmap.
* @example BitmapLayer *b = new BitmapLayer(pixels,1024,1024)
*/
void BitmapLayer::set(unsigned int *rgba, unsigned int width, unsigned int height) {
  if (data != NULL) {
    delete[] data;
    data = NULL;
  }
  data = rgba;
  w = width;
  h = height;
}

/**
* Constructor of BitmapLayers.
*/
BitmapLayers::BitmapLayers() {
}

/**
* Destructor of BitmapLayers.
*/
BitmapLayers::~BitmapLayers() {
  free();
}

/**
* A function to clear the current bitmap layer data.
*
* @example layers->free();
*/
void BitmapLayers::free() {
  layers.clear();
}

/**
* A function to load / add a Photoshop PSD with it's layers to the layers.
*
* @param name The filename of the PSD to load.
* @param lowerCase This may be used to have all loaded layers with lower case names.
* @example layers->loadPSD("map.psd");
*/
void BitmapLayers::loadPSD(const char *name, bool lowerCase) {
  PSDImage *img = new PSDImage();
  if(!img->Load(name)) {
    delete img; return;
  }
  int w = img->header.Columns;
  int h = img->header.Rows;
  for(int i=0; i < img->NumLayers(); i++) {
    int width, height, offset_x, offset_y;
    DWORD* imagedata = img->GetLayerData(i, width, height, offset_x, offset_y);
    if(imagedata != NULL) {
      if(img->LayerHidden(i)) {}
      unsigned int *l = new unsigned int[w*h];
      memset(l, 0, w * h * sizeof(unsigned int));
      String n = img->layers[i].name;
      layers[lowerCase ? toLower(n) : n].set(l,w,h);
      for (int y = 0; y < height; y++) {
        int ry = y + offset_y;
        if (ry < 0 || ry >= h) continue;
        for (int x = 0; x < width; x++) {
          int rx = x + offset_x;
          if (rx < 0 || rx >= w) continue;
          unsigned int c = imagedata[x + y * width];
          l[rx + ry * w] = (c & 0xff00ff00)|((c>>16)&0x00ff00ff)|((c<<16)&0x00ff00ff); // ARGB to RGBA
        }
      }
    }      
  }
  delete img;
}

/**
* This function uses the alpha channel of a source pixel to fade this source pixel onto a dest pixel.
*
* @param d The destination 32bit rgba pixel.
* @param s The source 32bit rgba pixel with it's alpha channel in the upperst 8 bit.
* @param alphaScale This may be used to decrease the influence of the source pixels alpha channel. Valid values are 0.f .. 1.f. Increasing alpha is not possible due to no clamping.
* @example alpha(map[0],map[1]);
*/
void alpha(unsigned int &d, unsigned int s, float alphaScale) {
  int a = ((s >> 24) & 255)*alphaScale;
  int r = ((s & 255)-(d & 255))*a/255+(d & 255);
  int g = (((s>>8) & 255)-((d>>8) & 255))*a/255+((d>>8) & 255);
  int b = (((s>>16) & 255)-((d>>16) & 255))*a/255+((d>>16) & 255);
  d = r|(g<<8)|(b<<16)|0xff000000;
}

/**
* This function uses the alpha channel of a source pixel to maybe clear the destination pixel (with 0). The alpha channel is checked by a threshold.
*
* @param d The destination 32bit rgba pixel.
* @param s The source 32bit rgba pixel with it's alpha channel in the upperst 8 bit.
* @thresh a value from 0 .. 255 defining the alpha threshold
* @example remove(map[0],color,128);
*/
void remove(unsigned int &d, unsigned int s, int thresh) {
  int a = (s >> 24) & 255;
  if (a >= thresh) {
    d = 0;
  }
}

/**
* This function downsamples a byte image by a factor using INTER_AREA filtering, thus doing some "advanced sort of mipmapping" so to say.
*
* @param data A pointer to the byte array pointer. The pointer gets modified by this function to a new byte image.
* @param w A pointer to the width of the source image. The width gets modified with the new width.
* @param h A pointer to the height of the source image. The height gets modified with the new height.
* @param div The value by which to divide the image size. Values below 1 result in nearest point filtered bigger image. Use other filters/functions for this <1 values.
* @example downsample(&pixels, &width, &height, 2.f);
*/
void downsample(unsigned char **data, int *w, int *h, float div) {
  int w2 = *w / div;
  int h2 = *h / div;
  int *n = new int[w2*h2];
  memset(n,0,w2*h2*sizeof(int));
  for (int y = 0; y < *h; y++) {
    int yp = y * h2 / *h;
    for (int x = 0; x < *w; x++) {
      int xp = x * w2 / *w;
      n[xp+yp*w2] += (int)(*data)[x+y*(*w)]|0x010000;
    }
  }
  unsigned char *n2 = new unsigned char[w2*h2];
  for (int i = 0; i < w2*h2; i++) {
    n2[i] = (n[i] & 0xffff)/(n[i]>>16);
  }
  delete[] *data;
  *data = n2;
  delete[] n;
  *w = w2;
  *h = h2;
}
