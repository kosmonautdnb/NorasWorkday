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
#ifndef __T_DLNAY_HPP__
#define __T_DLNAY_HPP__ 

#include "T_MAP.HPP"
#include "VECTOR.HPP"
#include "ARRAY.HPP"

/**
* A simple class representing a triangle of a landscape with it's 3 vertex indices.
*/
class LandscapeTriangle {

public:

  /// The vertex indices of the triangle.
  int p[3];

  /**
  * Constructor, initializing the indices with -1 (which is actually invalid).
  */
  LandscapeTriangle();

  /**
  * Constructor, initializing the indices with values.
  * 
  * @param p0 A vertex index for the first vertex.
  * @param p1 A vertex index for the second vertex.
  * @param p2 A vertex index for the third vertex.
  */
  LandscapeTriangle(int p0, int p1, int p2);

};

/**
* A simple class to collect all that is needed for a camera view of the landscape including Delaunay triangulation of the ground triangles.
*/
class LandscapeRaw {

public:

  /// the Landscape heightmap and elements and so on.
  class Landscape *scape;
  /// the Delaunay triangulation function. (Delaunator)
  class Delaunator *delau;
  /// Array for intermediate Delaunay properties (2D vertices) to triangulate. Actually the only source for Delaunay. Arrays don't reallocate on shrinking.
  Array<double> points;
  /// Array with the collected elements for the current view.  Arrays don't reallocate on shrinking.
  Array<class LandscapeElement*> elements;
  /// Array with the Delaunay triangulated triangles for the current view.  Arrays don't reallocate on shrinking.
  Array<LandscapeTriangle> triangles;

  /// Array with the vertex types of the (elements).  Arrays don't reallocate on shrinking.
  Array<int> types;
  /// Array with the 3D positions of the (elements).  Arrays don't reallocate on shrinking.
  Array<Vector> vertices;
  /// Array with the parameters of the (elements) (v0,v1,v2).  Arrays don't reallocate on shrinking.
  Array<Vector> parameters;

  /**
  * Constructor with a Landscape* holding the elements and heightmap
  * It also constructs the Delaunator instance.
  * You need to delete the _scape by yourself.
  *
  * @param _scape The Landscape object to be used by this class/instance.
  */
  LandscapeRaw(class Landscape *_scape);

  /**
  * Destructor of the LandscapeRaw class, if a Delaunator was set it deletes it and sets it NULL.
  */
  ~LandscapeRaw();

  /**
  * A function to collect all elements by the distance to the camera. The distance can be scaled.
  * It is implicitely called by update().
  *
  * @param cameraPos This is the current camera position for the actual collecting.
  * @param detailScale This can be used to scale the distance to the camera of the elements, resulting in more (or less) elements/detail.
  * @example scape->collectElements(Vector(0,0,0));
  */
  void collectElements(const Vector &cameraPos, const double detailScale = 1.0);

  /**
  * This function does the delaunay triangulation of the before collected elements. It has a switch/case for elements belonging to the ground, currently.
  * It is implicitely called by update().
  */
  void delaunay();

  /**
  * This function creates the ground triangle map and collects all elements from the Landscape* object resolved by the camera/viewer position and a detailScale.
  *
  * @param cameraPos This is the current camera position for the actual collecting.
  * @param detailScale This can be used to scale the distance to the camera of the elements, resulting in more (or less) elements/detail.
  * @example scape->update(Vector(0,0,0));
  */
  void update(const Vector &cameraPos, const double detailScale = 1.0);

};

#endif //__T_DLNAY_HPP__ 
