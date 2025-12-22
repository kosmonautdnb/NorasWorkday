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
#include "T_COLL.HPP"
#include <string.h> // memset,memcpy..
#include <math.h> // floor
#include <stdlib.h> // NULL

/**
* Constructors the collision map and sets it to zero.
*
* @param x0 the minimum X coordinate that will be folded in the lookups
* @param z0 the minimum Z coordinate that will be folded in the lookups
* @param x1 the maximum X coordinate that will be folded in the lookups                                                                                                                            
* @param z1 the maximum Z coordinate that will be folded in the lookups                                                                                                                            
* @param w the width(X) of the collision map in elements
* @param h the height(Z) of the collision map in elements
* @example new LandscapeCollision(-250,-250,250,250,1024,1024)
*/
LandscapeCollision::LandscapeCollision(float x0, float z0, float x1, float z1, int w, int h) {
    width = w;
    height = h;
    minX = x0;
    maxX = x1;
    minZ = z0;
    maxZ = z1;
    data = new unsigned char[width * height];
    memset(data, 0, width * height * sizeof(unsigned char));
}

/**
* Deletes the collision map if not NULL.
*/
LandscapeCollision::~LandscapeCollision() {
  if (data != NULL) {delete[] data; data = NULL;}
}

/**
* Does a blur with a box kernel of size (boxSize*2+1) in both directions (w,h) 
* to provide better collision normals later.
*
* @param boxSize Size of the centered box kernel in width and height, actually the kernel is (boxSize*2+1) in size.
* @example scape->boxBlur(3);
*/
void LandscapeCollision::boxBlur(int boxSize) {
  unsigned char *oldData = new unsigned char[width*height];
  memcpy(oldData,data,width*height*sizeof(unsigned char));
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      float v = 0; float w = 0;
      for (int ky = -boxSize+y; ky <= boxSize+y; ky++) {
        for (int kx = -boxSize+x; kx <= boxSize+x; kx++) {
          if ((unsigned int)kx<width&&(unsigned int)ky<height) {
            v += oldData[kx+ky*width];
            w += 1.f;
          }
        }
      }
      if (w != 0) v /= w;
      data[x+y*width] = v;
    }
  }
  delete[] oldData;
}

/**
* Gives the element number in x (width) of the map by giving it an actually world X coordinate, not clipped or clamped.
*
* @param x A world space X coordinate.
* @return An int denoting the element of (*data) in x (width). Not clipped or clamped.
* @example int posX = scape->xCoord(225.5);
*/
int LandscapeCollision::xCoord(double x) {
  return (int)floor((x - minX)*width/(maxX-minX));
}

/**
* Gives the x coordinate of the element number in x (width) of the map (*data).
*
* @param x An element number in x (width).
* @return A double representing the actual x coordinate for that element number.
* @example double worldX = scape->xPos(512);
*/
double LandscapeCollision::xPos(int x) {
  return (double)x * (maxX-minX) / width + minX;
}

/**
* Gives the element number in z (height) of the map by giving it an actually world Z coordinate, not clipped or clamped.
*
* @param z A world space Z coordinate.
* @return An int denoting the element of (*data) in z (height). Not clipped or clamped.
* @example int posZ = scape->zCoord(225.5);
*/
int LandscapeCollision::zCoord(double z) {
  return (int)floor((z - minZ)*height/(maxZ-minZ));
}

/**
* Gives the z coordinate of the element number in z (height) of the map (*data).
*
* @param z An element number in z (height).
* @return A double representing the actual z coordinate for that element number.
* @example double worldZ = scape->zPos(512);
*/
double LandscapeCollision::zPos(int z) {
  return (double)z * (maxZ-minZ) / height + minZ;
}

/**
* Gives the interpolated "height" of the heightmap at world space coordinate x,z.
*
* @param x A world space coordinate in x.
* @param z A world space coordinate in z.
* @return A double representing the actual height of the "heightmap" at that world space coordinate (or 255, if outside the actual "heightmap").
* @example double height = scape->point(225.5,225.5);
*/
double LandscapeCollision::point(double x, double z) {
  const double x2 = (x - minX) * width / (maxX-minX);
  const double z2 = (z - minZ) * height / (maxZ-minZ);
  const int xp = (int)floor(x2);
  const int zp = (int)floor(z2);
  if ((unsigned int)xp >= width-1) return 255;
  if ((unsigned int)zp >= height-1) return 255;
  const double fx = x2 - (double)xp;
  const double fz = z2 - (double)zp;
  const unsigned char v__ = data[xp+zp*width];
  const unsigned char vp_ = data[(xp+1)+zp*width];
  const unsigned char vpp = data[(xp+1)+(zp+1)*width];
  const unsigned char v_p = data[xp+(zp+1)*width];
  const double up = ((double)vp_ - (double)v__) * fx + (double)v__;
  const double dn = ((double)vpp - (double)v_p) * fx + (double)v_p;
  return (dn-up) * fz + up;
}

/**
* Gives the collision value of the heightmap at world space coordinate x,z. The collision value is true if the height is >= 128.
*
* @param x A world space coordinate in x.
* @param z A world space coordinate in z.
* @return A bool denoting collision or not collision on the interpolated "heightmap". True if >= 128.
* @example double height = scape->point(225.5,225.5);
*/
bool LandscapeCollision::isPoint(double x, double z) {
  return point(x,z) >= 128.0;
}

/**
* Returns the "x,z" normal of the field at world position x,z, by using the heightmap, thus returning some sort of curvature.
*
* @param x A world space coordinate in x.
* @param z A world space coordinate in z.
* @param nx NULL or a pointer for the returning "collision" normal in x.
* @param nz NULL or a pointer for the returning "collision" normal in z.
* @example scape->normal(225.5,225.5,&nx,&nz);
*/
void LandscapeCollision::normal(double x, double z, double *nx, double *nz) {
  const double dx = (double)(maxX-minX) / width * 0.5;
  const double dz = (double)(maxZ-minZ) / height * 0.5;
  const double ax = point(x+dx,z) - point(x-dx,z);
  const double az = point(x,z+dz) - point(x,z-dz);
  double d = sqrt(ax*ax+az*az);
  if (d != 0) d = 1.0 / d;
  if (nx != NULL) *nx = -ax*d;
  if (nz != NULL) *nz = -az*d;
}

/**
* Places a circle/disc at world position x,z with the radius rad*2. The values in the disc are from 0..255, thus directly at rad it's approx. 128.
* It uses the maximum value of the field it self and of the circle/disc.
*
* @param x A world space coordinate in x.
* @param z A world space coordinate in z.
* @param rad A radius in world space
* @example scape->placeCircle(225.5,225.5,2.0);
*/
void LandscapeCollision::placeCircle(double x, double z, double rad) {
  const double rad2 = rad * 2.0;
  const int xp0 = xCoord(x-rad2);
  const int zp0 = zCoord(z-rad2);
  const int xp1 = xCoord(x+rad2);
  const int zp1 = zCoord(z+rad2);
  for (int zp = zp0; zp <= zp1; zp++) {
    const double dz = (zPos(zp) - z)/rad2;
    if ((unsigned int)zp >= height) continue;
    for (int xp = xp0; xp <= xp1; xp++) {
      if ((unsigned int)xp >= width) continue;
      const double dx = (xPos(xp) - x)/rad2;
      const double d = 1.0-sqrt(dx*dx+dz*dz);
       if (d > 0) {
        int k = d * 256.0;
        if (k > 255) k = 255;
        if (k < 0) k = 0;
        if (k > data[xp+zp*width]) data[xp+zp*width] = k;
      }
    }
  }
}

/**
* Places a mask with width(x) and height(z) into the current height map.
* The mask can have different w,h than the current "heightmap" it will be filtered.
* The mask is scaled by (scale) and added an offset(add) before applied to the current "heightmap".
* It uses the maximum value of the field it self and of the mask to set.
*
* @param map The "collision" map with values between 0 and 255.
* @param w The element count in x (width) of the map to add.
* @param h The element count in z (height) of the map to add.
* @param scale A float scale factor to be applied to the map before offseting and placing the map element.
* @param add A float offset factor to be applied to the map after scalinf and before placing the map element. 0.5 means 128, 1.0 means 256.
* @example scape->placeMask(map,1024,1024,1.0,0.0);
*/
void LandscapeCollision::placeMask(unsigned char *map, unsigned int w, unsigned int h, float scale, float add) {
  for (int z = 0; z < height; z++) {
    for (int x = 0; x < width; x++) {
      const float x3 = (float)x * w / width;
      const float z3 = (float)z * h / height;
      const int x2 = (int)floor(x3);
      const int z2 = (int)floor(z3);
      const float x4 = x3 - x2;
      const float z4 = z3 - z2;
      const unsigned char v__ = map[x2+z2*w];
      const unsigned char vp_ = map[((x2+1)>=w?w-1:(x2+1))+z2*w];
      const unsigned char vpp = map[((x2+1)>=w?w-1:(x2+1))+((z2+1)>=h?h-1:(z2+1))*w];
      const unsigned char v_p = map[x2+((z2+1)>=h?h-1:(z2+1))*w];
      const float top = ((float)vp_ - (float)v__)*x4 + (float)v__;
      const float bottom = ((float)vpp - (float)v_p)*x4 + (float)v_p;
      const float t = (bottom - top)*z4+top;
      int k = t*scale+add*256.f;
      if (k>255) k = 255;
      if (k<0) k = 0;
      if (k > data[x+z*width]) data[x+z*width] = k;
    }
  }
}


/**
* Collides the line from world position x0,z0 to x1,z1 with the "heightmap" 
* and returns the approx. collision point and the normal of the map on that position.
* If x0,z0 where in a colliding area this function returns false (no collision).
* A collision happens, when the interpolated heightmap is >= 128 at that interpolated point.
*
* @param x0 The world space position of the start of the line in X
* @param z0 The world space position of the start of the line in Z
* @param x1 The world space position of the end of the line in X
* @param z1 The world space position of the end of the line in Z
* @param xh If a hit occured xh denotes the last non colliding world space position in X. Parameter may be NULL.
* @param zh If a hit occured zh denotes the last non colliding world space position in Z. Parameter may be NULL.
* @param xn If a hit occured xh denotes the last non colliding world space positions normal in X. Parameter may be NULL.
* @param zn If a hit occured zh denotes the last non colliding world space positions normal in Z. Parameter may be NULL.
* @return returns If a hit occured it returns true. If the line didn't collide with the heightmap (>=128) it returns false.
*/
bool LandscapeCollision::collideLine(const double x0, const double z0, const double x1, const double z1, double *xh, double *zh, double *xn, double *zn) {
  const bool hit = isPoint(x0,z0);
  if (xh != NULL) *xh = 0;
  if (zh != NULL) *zh = 0;
  if (xn != NULL) *xn = 0;
  if (zn != NULL) *zn = 0;
  if (hit) return false; // maybe remove this later again
  double xd = x1 - x0;
  double zd = z1 - z0;
  const double d = sqrt(xd*xd + zd*zd);
  if (fabs(d) < 0.00001) {if (hit) {if (xh != NULL) *xh = x0; if (zh != NULL) *zh = z0; } return hit; }
  xd /= d; zd /= d;
  double xp = x0;
  double zp = z0;
  const double stepX = (double)(maxX-minX)/width;
  const double stepZ = (double)(maxZ-minZ)/height;
  double step = stepX * 0.5; if (stepZ<stepX) step = stepZ * 0.5;
  const double stepSpeed = 0.1; step *= stepSpeed;
  while(1) {
    const double lx = xp;
    const double lz = zp;
    xp += xd * step;
    zp += zd * step;
    const double dx = xp - x0;
    const double dz = zp - z0;
    if (sqrt(dx*dx + dz*dz) > d) {
      if (isPoint(xp,zp)) {
        if (xh != NULL) *xh = lx;
        if (zh != NULL) *zh = lz;
        normal(lx,lz,xn,zn);
        return true;
      }
      return false;
    }
    if (isPoint(xp,zp)) {
      if (xh != NULL) *xh = lx;
      if (zh != NULL) *zh = lz;
      normal(lx,lz,xn,zn);
      return true;
    }
  }
  return false;
}
