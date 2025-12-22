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
#ifndef __T_MAP_HPP__
#define __T_MAP_HPP__

#include "ARRAY.HPP"

/**
* This enum tells the actual element type of an LandscapeElement
*/
typedef enum {

  /// pure heightmap vertex/element (in one scenery all is grass), it incorporates the *boden value aswell.
  LANDSCAPE_TYPE_HEIGHT = 0,
  /// a road vertex/element
  LANDSCAPE_TYPE_ROAD = 1,
  /// a single tree
  LANDSCAPE_TYPE_TREE = 2,
  /// a single grass blade sprite
  LANDSCAPE_TYPE_GRASS = 3,
  /// a single flower sprite
  LANDSCAPE_TYPE_FLOWER = 4,
  /// an elevated stone like element vertex/element
  LANDSCAPE_TYPE_STONE = 5,
  /// a water vertex/element
  LANDSCAPE_TYPE_WATER = 6,
  /// a 3D object int the landscape with it's "center" point
  LANDSCAPE_TYPE_OBJECT = 7

} LandscapeType;

/**
* This class represents single landscape element. 
* With 4 byte alignment.
*/
class LandscapeElement {

public:

  /// 4 bytes, the LandscapeType and 3 parameters
  unsigned char type,v0,v1,v2;
  /// 4 byte, The distance from the camera when this element actually pops up (and gets collected/visible)
  float distanceThresholdSquared;
  /// 4*3 byte, element position in world space
  float x,y,z;

};

/**
* This class represents all the collected landscape points which may be triangulated using Delaunay or just beeing painted as 3D Objects and so on.
*/
class Landscape {

public:

  /// the array with all the landscape elements
  Array<LandscapeElement> scape;

  /// The minimum and maximum in X dimension of the positions of the landscape.
  float minX, maxX;
  /// The minimum and maximum in Y dimension of the positions of the landscape. The heightmap is scaled to these values.
  float minY, maxY;
  /// The minimum and maximum in Z dimension of the positions of the landscape.
  float minZ, maxZ;

  /// The width (x) and height(z) of the *map_height and *map_boden array.
  unsigned int map_height_w, map_height_h;
  /// the heightmap with values from 0..65535 rescaled to minY/maxY at evaluation. Gets deleted at destruction.
  unsigned short *map_height;
  /// the bodenmap needed for the empty regions of the heightmap. Gets deleted at destruction.
  unsigned char *map_boden;

  /**
  * Constructors the Landscape object.
  *
  * @param x0 the minimum X coordinate that will be folded in the lookups
  * @param z0 the minimum Z coordinate that will be folded in the lookups
  * @param x1 the maximum X coordinate that will be folded in the lookups                                                                                                                            
  * @param z1 the maximum Z coordinate that will be folded in the lookups                                                                                                                            
  * @param y0 the minimum height of the heightmap (minimum Y coordinate).
  * @param y1 the maximum height of the heightmap (maximum Y coordinate).
  * @example new Landscape(-250,-250,250,250,0,1000.f)
  */
  Landscape(float x0, float z0, float x1, float z1, float y0, float y1);

  /**
  * Deletes the height map and boden map if not NULL.
  */
  ~Landscape();


  /**
  * Collects all the landscape elements which are in a certain distance from the camera. 
  * The distance can also be controlled by the "detailScale".
  *
  * @param elements An array that gets filled with the landscape elements that are near the camera. My Array class doesn't realloc on array shrinks.
  * @param cx The cameras X position in world space.
  * @param cy The cameras Y position in world space.
  * @param cz The cameras Z position in world space.
  * @param detailScale This gets multiplied by all of the elements distanceThresholdSquared properties to have them fade in earlier or later, 1.0 is a good default.
  * @example scape->collectLandscape(&elements, 0,0,0, 1.0);
  */
  void collectLandscape(Array<LandscapeElement*> *elements, const float cx, const float cy, const float cz, const float detailScale);

  /**
  * Removes landscape elements with the given type
  *
  * @param type The type of the elements to remove e.g. LANDSCAPE_TYPE_OBJECT
  * @example scape->removeElementsWithType(LANDSCAPE_TYPE_OBJECT);
  */
  void removeElementsWithType(unsigned int type);

  /**
  * Gets the interpolated height from the heightmap from world space coordinates. 
  * The height is between minY and maxY. 
  * X and Z are clamped to the heightmap, so the borders are "replicant" their height values.
  *
  * @param x A world space X position.
  * @param z A world space Z position.
  * @param xa An element addition for x(width), so e.g. box blurs of the heightmap are possible.
  * @param za An element addition for z(height), so e.g. box blurs of the heightmap are possible.
  * @return The height of the heightmap at this position.
  * @example float height = scape->getHeight(225.5, 225.5);
  */
  float getHeight(float x, float z, int xa = 0, int za = 0);

  /**
  * Gets a box filtered height from the heightmap from world space coordinates. 
  * The height is between minY and maxY. 
  * X and Z are clamped to the heightmap, so the borders are "replicant" their height values.
  *
  * @param x A world space X position.
  * @param z A world space Z position.
  * @param rad A element size radius for the box kernel. It gets add on x(width) and z(height) at lookup. The kernel is in x and z (rad*2+1) in size.
  * @return The box filtered height of the heightmap at this position.
  * @example float height = scape->getHeightBox(225.5, 225.5,3);
  */
  float getHeightBox(float x, float z, int rad);

  /**
  * Puts a height onto the heightmap.
  *
  * @param x A world space X position.
  * @param z A world space Z position.
  * @param height A world space height(y).
  * @example scape->putHeight(225.5, 225.5,1000.f);
  */
  void putHeight(float x, float z, float height);

  /**
  * Generates the LANDSCAPE_TYPE_HEIGHT elements from the given heightmap and bodenmap.
  *
  * @param mask A mask array also with w/h dimensions, telling if this element should be generated, if the mask is other than 0 at this position, no element gets generated.
  * @param map A heightmap array with w/h dimensions. Values range from 0..65535. And are converted to minY(before 0) .. maxY(before 65535).
  * @param w The dimension of the heightmap (and bodenmap and maskmap) in X (width).
  * @param h The dimension of the heightmap (and bodenmap and maskmap) in Z (height).
  * @param stepX the steps on the heightmap in X(width) to maybe generate a new element.
  * @param stepZ the steps on the heightmap in Z(height) to maybe generate a new element.
  * @param distFact A factor by which to scale the "first appearance on distance" values for the elements.
  * @param steepThresh A factor for how steep the curvature of the terrain has to be so that new elements must be added.
  * @param boden A boden array with same dimensions as the heightmap (w/h), to tell the floor/ground type of the LANDSCAPE_TYPE_HEIGHT elements here.
  * @example scape->setHeightMap(mask,map,1024,1024,1,1,1.f,1.f,boden);
  */
  void setHeightMap(unsigned char *mask, unsigned short *map, unsigned int w, unsigned int h, int stepX, int stepZ, float distFact, float steepThresh, unsigned char *boden);

  /**
  * Places the objects by an rgba picture with dots for each object.
  * The colorchannels are evaluated as values r=0 is no object, r = 4 is object with type 1, r = 8 is object with type 2..
  *
  * @param rgba For every pixel rgba telling of the object type to set here. r,g,b are evaluated if each element having 0..63 possible values.
  * @param w The object pictures width(x).
  * @param h The object pictures height(z).
  * @example scape->setObjects(objects,1024,1024);
  */
  void setObjects(unsigned int *rgba, unsigned int w, unsigned int h);

  /**
  * Places an empty vertex/landscapeelement is e.g. used to have borders around a Delaunay triangulation. Or to just tesselate points/regions finer.
  *
  * @param x2 An element space X(width) coordinate.
  * @param z2 An element space Z(height) coordinate.
  * @param w the resolution of the element map in x(width).
  * @param h the resolution of the element map in z(height).
  * @param distFact on which distance to the camera the element should appear at element collection.
  * @example scape->insertEmpty(512.2,512.2,1024,1024,750.f);
  */
  void insertEmpty(float x2, float z2, int w, int h, float distFact);

  /**
  * Calculates the exact position of the threshold in a cell (left,center,right). Gives "subpixel" position of the actual threshold (-1=left,0=center,1=right).
  *
  * @param center The center pixels threshold value.
  * @param left The left pixels threshold value.
  * @param right The right pixels threshold value.
  * @param the threshold value to actually locate on subpixel resolution.
  * @return the position of the actual threshold between left,center and right. If left=128 and center = 256 and right = 512 and threshold = 192 then return should be -0.5.
  * @example scape->hitThresh(256,128,512,192);
  */
  float hitThresh(float center, float left, float right, float thresh);

  /**
  * A function for placing trees into the elements map. (LANDSCAPE_TYPE_TREE)
  *
  * @remarks uses srand();
  * @param mask A mask array also with w/h dimensions, telling if this element should be generated, if the mask is other than 0 at this position, no element gets generated.
  * @param map A map with values != 0 denoting trees.
  * @param w The size of the map (and mask) in X(width).
  * @param h The size of the map (and mask) in Z(height).
  * @param randomModulo every "nth" element may be placed. Some sort of randomized selection for the elements.
  * @example scape->setTrees(mask,trees,1024,1024,16);
  */
  void setTrees(unsigned char *mask, unsigned char *map, unsigned int w, unsigned int h, int randomModulo);

  /**
  * A function for placing grass blades into the elements map. (LANDSCAPE_TYPE_GRASS)
  *
  * @remarks uses srand();
  * @param mask A mask array also with w/h dimensions, telling if this element should be generated, if the mask is other than 0 at this position, no element gets generated.
  * @param map A map with values != 0 denoting trees.
  * @param w The size of the map (and mask) in X(width).
  * @param h The size of the map (and mask) in Z(height).
  * @param randomModulo every "nth" element may be placed. Some sort of randomized selection for the elements.
  * @example scape->setGrass(mask,trees,1024,1024,16);
  */
  void setGrass(unsigned char *mask, unsigned char *map, unsigned int w, unsigned int h, int randomModulo);

  /**
  * A function for placing stone structures into the elements map. (LANDSCAPE_TYPE_STONE)
  * Ideally the stones are already removed/masked at setHeightMap().
  *
  * @remarks uses srand();
  * @param mask A mask array also with w/h dimensions, telling if this element should be generated, if the mask is other than 0 at this position, no element gets generated.
  * @param map A map with heightmap like values.
  * @param w The size of the map (and mask) in X(width).
  * @param h The size of the map (and mask) in Z(height).
  * @param threshOuter the stone margin (map>=thresh) means stone structure here.
  * @param threshCleanup the cleanup margin (map>=thresh) means stone structure needs another triangles here to mark full triangles as stone structures.
  * @example scape->setStones(mask,trees,1024,1024,128,64);
  */
  void setStones(unsigned char *map, unsigned int w, unsigned int h, int threshOuter, int threshCleanup);

  /**
  * A function for placing water into the elements map. (LANDSCAPE_TYPE_WATER)
  * Ideally the water is already removed/masked at setHeightMap().
  *
  * @remarks uses srand();
  * @param map A map with heightmap like values.
  * @param w The size of the map in X(width).
  * @param h The size of the map in Z(height).
  * @param threshOuter the water margin (map>=thresh) means water structure here.
  * @param threshCleanup the cleanup margin (map>=thresh) means water structure needs another triangles here to mark full triangles as water structures.
  * @example scape->setWater(map,1024,1024,128,64);
  */
  void setWater(unsigned char *map, unsigned int w, unsigned int h, int threshOuter, int threshCleanup);

  /**
  * A function for placing flower points into the elements map. (LANDSCAPE_TYPE_FLOWER)
  *
  * @remarks uses srand();
  * @param mask A mask array also with w/h dimensions, telling if this element should be generated, if the mask is other than 0 at this position, no element gets generated.
  * @param map A map with values != 0 denoting trees.
  * @param w The size of the map (and mask) in X(width).
  * @param h The size of the map (and mask) in Z(height).
  * @param randomModulo every "nth" element may be placed. Some sort of randomized selection for the elements.
  * @example scape->setFlowers(mask,trees,1024,1024,16);
  */
  void setFlowers(unsigned char *mask, unsigned char *map, unsigned int w, unsigned int h, int randomModulo);

  /**
  * A function for placing ways/roads into the elements map. (LANDSCAPE_TYPE_ROAD)
  * Ideally the roads are already removed/masked at setHeightMap().
  *
  * @remarks uses srand();
  * @param map A map with heightmap like values.
  * @param w The size of the map in X(width).
  * @param h The size of the map in Z(height).
  * @param threshWayOuter the way margin (map>=thresh) means way structure here.
  * @param threshWayInner the way margin for the inner way structure, (map>=thresh) means way structure here.
  * @param threshCleanupMuchOuta the cleanup margin (map>=thresh) means way structure needs another triangles here to mark full triangles as way structures.
  * @example scape->setRoads(map,1024,1024,100,128,64);
  */
  void setRoads(unsigned char *map, unsigned int w, unsigned int h, int threshWayOuter, int threshWayInner,int threshCleanupMuchOuta);

};

#endif // __T_MAP_HPP__
