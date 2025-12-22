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
#ifndef __O_WAVOBJ_HPP__
#define __O_WAVOBJ_HPP__

#include "ARRAY.HPP"
#include "VECTOR.HPP"

/**
* A class for a face of an WaveFront OBJ representation.
*/
class WAVOBJ_Face {

public:

  /// The vertex count, actually only 3 or 4.
  int numVerts;
  /// The vertex positions as indices.
  int p[4];
  /// The vertex normals as indices.
  int n[4];
  /// The vertex colors as indices.
  int c[4];
  /// The vertex texture coordinates as indices.
  int t[4];

  /**
  * Constructor of a single polygon/face of an OBJ representation.
  */
  WAVOBJ_Face();

};

/**
* A class for a material of an WaveFront OBJ representation.
*/
class WAVOBJ_Material {

public:

  /// The OpenGL texture handle.
  unsigned int texture;
  /// An rgba diffuse color.
  Vector colorDiffuse;
  /// An rgba specular color.
  Vector colorSpecular;
  /// An rgba ambient color.
  Vector colorAmbient;
  /// An rgba emissive color.
  Vector colorEmissive;
  /// The specular shininess coefficient.
  double shininess;

  /**
  * Constructor of an WaveFront OBJ Material.
  */
  WAVOBJ_Material();

};

/**
* A class for a "Batch" of vertices of a WaveFront OBJ representation.
*/
class WAVOBJ_MeshPart {

public:

  /// The faces.
  Array<WAVOBJ_Face> faces;
  /// The materialID/material index, -1 means no material.
  int materialId;
  /// The bounding box/sphere center.
  Vector center;
  /// The bounding sphere radius.
  double boundingRadius;
  /// The bounding boxes minimum component.
  Vector minBounding;
  /// The bounding boxes maximum component.
  Vector maxBounding;

  /**
  * Constructor for an empty part.
  */
  WAVOBJ_MeshPart();

};

/**
* A class for a WaveFront OBJ mesh.
*/
class WAVOBJ_Mesh {

public:

  /// The positions of the mesh.
  Array<Vector> positions;
  /// The normals of the mesh.
  Array<Vector> normals;
  /// The vertex colors of the mesh.
  Array<Vector> colors;
  /// The texture coordinates of the mesh.
  Array<Vector> texCoords;
  /// The "batches" of the mesh.
  Array<WAVOBJ_MeshPart> parts;
  /// The materials of the mesh.
  Array<WAVOBJ_Material> materials;
  /// The center of the bounding box/sphere.
  Vector center;
  /// The radius of the bounding sphere.
  double boundingRadius;
  /// The minimum components of the bounding box.
  Vector minBounding;
  /// The maximum components of the bounding box.
  Vector maxBounding;

  /**
  * Constructor creating an empty mesh.
  */
  WAVOBJ_Mesh();

};

/**
* Loads an WaveFront .obj file with it's .mtl file if readable.
* This one copies the material color into the vertex/face color.
* This one doesn't load the textures.
*
* @param fileName The name of the file to load. (*.obj with its *.mtl)
* @param genFaceNormals Set this to true if you need face normals calculated by yourself.
* @param scale A scale factor for the vertices of the object. 
* @param add An offset for the vertices of the object. 
* @param genVertexNormals Set this to true if you need vertex normals calculated by yourself.
* @return The loaded mesh or NULL.
* @example WAVOBJ_Mesh *obj = loadOBJ("girl.obj");
*/
WAVOBJ_Mesh *loadOBJ(const char *fileName, bool genFaceNormals = false, float scale = 1.f, const Vector &add = Vector(), bool genVertexNormals = false);

/**
* Loads an WaveFront .obj file with it's .mtl file if readable.
* This one is used for the trees in the game. They have a black shaft and texture sizes divided by a factor.
*
* @param fileName The name of the file to load. (*.obj with its *.mtl)
* @param genFaceNormals Set this to true if you need face normals calculated by yourself.
* @return The loaded mesh or NULL.
* @example WAVOBJ_Mesh *obj = loadTreeOBJ("tree.obj");
*/
WAVOBJ_Mesh *loadTreeOBJ(const char *fileName, bool genFaceNormals = false);

/**
* A function to paint a WaveFront .obj with OpenGL.
*
* @param m The mesh to paint.
* @example paintMesh(mesh);
*/
void paintMesh(WAVOBJ_Mesh *m);

/**
* A callback to be used for texture loading for WaveFront OBJ files.
* This one is making red to green/cyanish.
*
* @param fileName The file name of the texture to load.
* @param type The type of the texture to load.
* @return The OpenGL texture handle of the created texture.
* @example unsigned int texHandle = SMPL_loadTexture2("bitmap.png","map_Kd");
*/
unsigned int SMPL_loadTexture2(const String &fileName, const String &type);

#endif // __O_WAVOBJ_HPP__
