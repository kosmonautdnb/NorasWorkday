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
#include "T_DLNAY.HPP"
#include "DELAUNTR.HPP" // Delaunator

/**
* A function to sort triangles by their first vertex index, so sort by camera depth (vertices are sorted that way already before).
*
* @param a The first input element for quicksort.
* @param b The second input element for quicksort.
* @return An int that tells if a<b(-1) or a==b(0) or a>b(1)
* @example qsort(&triangles[0],triangles.size(),sizeof(Triangle),triangleSortFunc);
*/
static int triangleSortFunc(const void *a, const void *b) {
  const LandscapeTriangle *v0 = (const LandscapeTriangle *)a;
  const LandscapeTriangle *v1 = (const LandscapeTriangle *)b;
  return v0->p[0] - v1->p[0];
}

/// the camera position, world space X, for sorting with elementSortFunc
static float eSortCX = 0;
/// the camera position, world space Y, for sorting with elementSortFunc
static float eSortCY = 0;
/// the camera position, world space Z, for sorting with elementSortFunc
static float eSortCZ = 0;

/**
* A function to sort LandscapeElement* by their distance to the camera/viewer
*
* @param a The first input element for quicksort.
* @param b The second input element for quicksort.
* @return An int that tells if a<b(-1) or a==b(0) or a>b(1)
* @example qsort(&triangles[0],triangles.size(),sizeof(Triangle),triangleSortFunc);
*/
static int elementSortFunc(const void *a, const void *b) {
  const LandscapeElement *v0 = *((const LandscapeElement **)a);
  const LandscapeElement *v1 = *((const LandscapeElement **)b);
  const float dx0 = v0->x - eSortCX;
  const float dy0 = v0->y - eSortCY;
  const float dz0 = v0->z - eSortCZ;
  const float dx1 = v1->x - eSortCX;
  const float dy1 = v1->y - eSortCY;
  const float dz1 = v1->z - eSortCZ;
  return -((int)(dx1 * dx1 + dy1 * dy1 + dz1 * dz1) - (int)(dx0 * dx0 + dy0 * dy0 + dz0 * dz0));
}

/**
* Constructor, initializing the indices with -1 (which is actually invalid).
*/
LandscapeTriangle::LandscapeTriangle() {
  p[0] = p[1] = p[2] = -1;
}

/**
* Constructor, initializing the indices with values.
* 
* @param p0 A vertex index for the first vertex.
* @param p1 A vertex index for the second vertex.
* @param p2 A vertex index for the third vertex.
*/
LandscapeTriangle::LandscapeTriangle(int p0, int p1, int p2) {
  p[0] = p0; 
  p[1] = p1; 
  p[2] = p2;
}

/**
* Constructor with a Landscape* holding the elements and heightmap
* It also constructs the Delaunator instance.
* You need to delete the _scape by yourself.
*
* @param _scape The Landscape object to be used by this class/instance.
*/
LandscapeRaw::LandscapeRaw(class Landscape *_scape) {
  scape = _scape;
  delau = new Delaunator();
}

/**
* Destructor of the LandscapeRaw class, if a Delaunator was set it deletes it and sets it NULL.
*/
LandscapeRaw::~LandscapeRaw() {
  if (delau != NULL) {delete[] delau; delau = NULL;}
}

/**
* A function to collect all elements by the distance to the camera. The distance can be scaled.
* It is implicitely called by update().
*
* @param cameraPos This is the current camera position for the actual collecting.
* @param detailScale This can be used to scale the distance to the camera of the elements, resulting in more (or less) elements/detail.
* @example scape->collectElements(Vector(0,0,0));
*/
void LandscapeRaw::collectElements(const Vector &cameraPos, const double detailScale) {
  scape->collectLandscape(&elements, cameraPos.x, cameraPos.y, cameraPos.z, detailScale);
  eSortCX = cameraPos.x;
  eSortCY = cameraPos.y;
  eSortCZ = cameraPos.z;
  qsort(&elements[0], elements.size(), sizeof(LandscapeElement*), elementSortFunc); // sort by camera distance (nearest first)
}

/**
* This function does the delaunay triangulation of the before collected elements. It has a switch/case for elements belonging to the ground, currently.
* It is implicitely called by update().
*/
void LandscapeRaw::delaunay() {
  triangles.clear();
  points.clear();
  types.clear();
  vertices.clear();
  parameters.clear();
  for (int i = 0; i < elements.size(); i++) {
    LandscapeElement *e = elements[i];
    switch(e->type) {
      case LANDSCAPE_TYPE_WATER:
      case LANDSCAPE_TYPE_STONE:
      case LANDSCAPE_TYPE_HEIGHT:
      case LANDSCAPE_TYPE_ROAD: {
        points.push_back(e->x); points.push_back(e->z);
        types.push_back(e->type);
        vertices.push_back(Vector(e->x, e->y, e->z));
        parameters.push_back(Vector(e->v0/255.0, e->v1/255.0, e->v2/255.0));
      } break;
    }
  }
  delau->delaunator(&points);
  for (int j = 0; j < delau->triangles.size()/3; j++) {
    triangles.push_back(LandscapeTriangle(delau->triangles[j*3+0],delau->triangles[j*3+1],delau->triangles[j*3+2]));
  }
  qsort(&triangles[0],triangles.size(),sizeof(LandscapeTriangle),triangleSortFunc);
}

/**
* This function creates the ground triangle map and collects all elements from the Landscape* object resolved by the camera/viewer position and a detailScale.
*
* @param cameraPos This is the current camera position for the actual collecting.
* @param detailScale This can be used to scale the distance to the camera of the elements, resulting in more (or less) elements/detail.
* @example scape->update(Vector(0,0,0));
*/
void LandscapeRaw::update(const Vector &cameraPos, const double detailScale) {
  collectElements(cameraPos, detailScale);
  delaunay();
}
