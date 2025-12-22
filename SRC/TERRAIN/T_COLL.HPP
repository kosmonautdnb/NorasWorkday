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
#ifndef __T_COLL_HPP__
#define __T_COLL_HPP__

/**
* This class collides elements on a heightmap like structure with the threshold >=128,
* if above this threshold it's denoting a collision.
*/
class LandscapeCollision {

public:

  /// A "heightmap", it will be created by the constructor.
  unsigned char *data;
  /// The width and height of the data "heightmap" array.
  int width, height;
  /// The minimum and maximum in X dimension of the positions of the "heightmap".
  double minX,maxX;
  /// The minimum and maximum in Z dimension of the positions of the "heightmap".
  double minZ,maxZ;

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
  LandscapeCollision(float x0, float z0, float x1, float z1, int w, int h);

  /**
  * Deletes the collision map if not NULL.
  */
  ~LandscapeCollision();

  /**
  * Does a blur with a box kernel of size (boxSize*2+1) in both directions (w,h) 
  * to provide better collision normals later.
  *
  * @param boxSize Size of the centered box kernel in width and height, actually the kernel is (boxSize*2+1) in size.
  * @example scape->boxBlur(3);
  */
  void boxBlur(int boxSize);

  /**
  * Gives the element number in x (width) of the map by giving it an actually world X coordinate, not clipped or clamped.
  *
  * @param x A world space X coordinate.
  * @return An int denoting the element of (*data) in x (width). Not clipped or clamped.
  * @example int posX = scape->xCoord(225.5);
  */
  int xCoord(double x);

  /**
  * Gives the x coordinate of the element number in x (width) of the map (*data).
  *
  * @param x An element number in x (width).
  * @return A double representing the actual x coordinate for that element number.
  * @example double worldX = scape->xPos(512);
  */
  double xPos(int x);

  /**
  * Gives the element number in z (height) of the map by giving it an actually world Z coordinate, not clipped or clamped.
  *
  * @param z A world space Z coordinate.
  * @return An int denoting the element of (*data) in z (height). Not clipped or clamped.
  * @example int posZ = scape->zCoord(225.5);
  */
  int zCoord(double z);

  /**
  * Gives the z coordinate of the element number in z (height) of the map (*data).
  *
  * @param z An element number in z (height).
  * @return A double representing the actual z coordinate for that element number.
  * @example double worldZ = scape->zPos(512);
  */
  double zPos(int z);

  /**
  * Gives the interpolated "height" of the heightmap at world space coordinate x,z.
  *
  * @param x A world space coordinate in x.
  * @param z A world space coordinate in z.
  * @return A double representing the actual height of the "heightmap" at that world space coordinate (or 255, if outside the actual "heightmap").
  * @example double height = scape->point(225.5,225.5);
  */
  double point(double x, double z);

  /**
  * Gives the collision value of the heightmap at world space coordinate x,z. The collision value is true if the height is >= 128.
  *
  * @param x A world space coordinate in x.
  * @param z A world space coordinate in z.
  * @return A bool denoting collision or not collision on the interpolated "heightmap". True if >= 128.
  * @example double height = scape->point(225.5,225.5);
  */
  bool isPoint(double x, double z);

  /**
  * Returns the "x,z" normal of the field at world position x,z, by using the heightmap, thus returning some sort of curvature.
  *
  * @param x A world space coordinate in x.
  * @param z A world space coordinate in z.
  * @param nx NULL or a pointer for the returning "collision" normal in x.
  * @param nz NULL or a pointer for the returning "collision" normal in z.
  * @example scape->normal(225.5,225.5,&nx,&nz);
  */
  void normal(double x, double z, double *nx, double *nz);

  /**
  * Places a circle/disc at world position x,z with the radius rad*2. The values in the disc are from 0..255, thus directly at rad it's approx. 128.
  * It uses the maximum value of the field it self and of the circle/disc.
  *
  * @param x A world space coordinate in x.
  * @param z A world space coordinate in z.
  * @param rad A radius in world space
  * @example scape->placeCircle(225.5,225.5,2.0);
  */
  void placeCircle(double x, double z, double rad);

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
  void placeMask(unsigned char *map, unsigned int w, unsigned int h, float scale, float add);

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
  bool collideLine(const double x0, const double z0, const double x1, const double z1, double *xh, double *zh, double *xn, double *zn);

};

#endif // __T_COLL_HPP__
