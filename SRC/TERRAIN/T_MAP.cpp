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
#include "T_MAP.HPP"
#include <math.h> // floor,sqrt,sin..

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
Landscape::Landscape(float x0, float z0, float x1, float z1, float y0, float y1) {map_height = NULL; map_boden = NULL; minX = x0; maxX = x1; minZ = z0; maxZ = z1; minY = y0; maxY = y1;}

/**
* Deletes the height map and boden map if not NULL.
*/
Landscape::~Landscape() {if (map_height != NULL) {delete[] map_height; map_height = NULL;} if (map_boden != NULL) {delete[] map_boden; map_boden = NULL;}}


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
void Landscape::collectLandscape(Array<LandscapeElement*> *elements, const float cx, const float cy, const float cz, const float detailScale) {
  const float k = detailScale;
  elements->clear();
  LandscapeElement *e = &scape[0];
  for(int i = scape.size() - 1; i >= 0; i--) {
    const float dx = e->x - cx;
    const float dy = e->y - cy;
    const float dz = e->z - cz;
    if (dx*dx + dy*dy + dz*dz < e->distanceThresholdSquared*k) {
      elements->push_back(e);
    }
    e++;
  }
}

/**
* Removes landscape elements with the given type
*
* @param type The type of the elements to remove e.g. LANDSCAPE_TYPE_OBJECT
* @example scape->removeElementsWithType(LANDSCAPE_TYPE_OBJECT);
*/
void Landscape::removeElementsWithType(unsigned int type) {
  for(int i = scape.size() - 1; i >= 0; i--) {
    if (scape[i].type == type) {
      scape.erase(i,1);
    }
  }
}

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
float Landscape::getHeight(float x, float z, int xa, int za) {
  if (x < minX) x = minX;
  if (z < minZ) z = minZ;
  if (x > maxX-0.001f) x = maxX-0.001f;
  if (z > maxZ-0.001f) z = maxZ-0.001f;
  const float xf = (x - minX)*map_height_w/(maxX-minX);
  const float zf = (z - minZ)*map_height_h/(maxZ-minZ);
  const int xi = floor(xf);
  const int zi = floor(zf);
  const int xi_ = xi + xa;
  const int zi_ = zi + za;
  const int xi0 = xi_ < 0 ? 0 : (xi_ >= map_height_w ? (map_height_w-1) : xi_);
  const int zi0 = zi_ < 0 ? 0 : (zi_ >= map_height_h ? (map_height_h-1) : zi_);
  const int xi1 = (xi0 + 1) >= map_height_w ? xi0 : (xi0 + 1);
  const int zi1 = (zi0 + 1) >= map_height_h ? zi0 : (zi0 + 1);
  const float fx = xf - (double)xi;
  const float fy = zf - (double)zi;
  const unsigned short p00 = map_height[xi0+zi0*map_height_w];
  const unsigned short p10 = map_height[xi1+zi0*map_height_w];
  const unsigned short p11 = map_height[xi1+zi1*map_height_w];
  const unsigned short p01 = map_height[xi0+zi1*map_height_w];
  const float top = ((float)p10 - (float)p00) * fx + (float)p00;
  const float btm = ((float)p11 - (float)p01) * fx + (float)p01;
  return (float)((btm-top)*fy+top)*(maxY-minY)/65535.0+minY;

}

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
float Landscape::getHeightBox(float x, float z, int rad) {
  float h,w;
  h = w = 0;
  for (int zi = -rad; zi <= rad; zi++) {
    for (int xi = -rad; xi <= rad; xi++) {
      h += getHeight(x,z,xi,zi);
      w += 1.f;
    }
  }
  return w>0?h/w:0;
}

/**
* Puts a height onto the heightmap.
*
* @param x A world space X position.
* @param z A world space Z position.
* @param height A world space height(y).
* @example scape->putHeight(225.5, 225.5,1000.f);
*/
void Landscape::putHeight(float x, float z, float height) {
  if (x<minX||x>=maxX) return;
  if (z<minZ||z>=maxZ) return;
  const float xf = (x - minX)*map_height_w/(maxX-minX);
  const float zf = (z - minZ)*map_height_h/(maxZ-minZ);
  const int xi = floor(xf);
  const int zi = floor(zf);
  if ((unsigned int)xi >= map_height_w-1) return;
  if ((unsigned int)zi >= map_height_h-1) return;
  float k = (float)(height - minY)/(maxY-minY)*65535.0;
  if (k < 0) k = 0;
  if (k > 65535.0) k = 65535.0;
  map_height[xi+zi*map_height_w] = k;
}

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
void Landscape::setHeightMap(unsigned char *mask, unsigned short *map, unsigned int w, unsigned int h, int stepX, int stepZ, float distFact, float steepThresh, unsigned char *boden) {
  const float COMMONDISTANCE = 750.f;
  map_boden = boden;
  map_height = map;
  map_height_w = w;
  map_height_h = h;
  int k = scape.size();
  scape.resize(k + (w/stepX)*(h/stepZ));
  for (int z = 0; z < h; z += stepZ) {
    int tz = h; while((z % tz)!=0) tz>>=1;
    for (int x = 0; x < w; x += stepX) {
      const bool border = x == 0 || x >= w-stepX || z == 0 || z >= h-stepZ;
      if (mask[x+z*w] && (!border)) continue;
      const unsigned short v__ = map[x+z*w];
      const unsigned short vn_ = map[((x-1)<0?0:(x-1))+z*w];
      const unsigned short vp_ = map[((x+1)>=w?w-1:(x+1))+z*w];
      const unsigned short v_n = map[x+((z-1)<0?0:(z-1))*w];
      const unsigned short v_p = map[x+((z+1)>=h?h-1:(z+1))*w];
      const float vx = (vn_+vp_)*0.5-v__;
      const float vy = (v_n+v_p)*0.5-v__;
      const float v = sqrt(vx*vx+vy*vy);
      if (v < steepThresh && (!border)) continue;
      LandscapeElement *e = &scape[k]; k++;
      e->x = (maxX-minX)*x/w+minX;
      e->y = (maxY-minY)*v__/65535.0+minY;
      e->z = (maxZ-minZ)*z/h+minZ;
      e->type = LANDSCAPE_TYPE_HEIGHT;
      float dx = vp_ - vn_;
      float dy = v_p - v_n;
      float d = sqrt(dx*dx + dy*dy);
      if (d!=0) {dx /=d; dy /= d;}
      e->v0 = 128+dx * 127;
      e->v1 = boden[x+z*w];
      int tx = w; while((x % tx)!=0) tx>>=1;
      float siz = (float)tx / w;
      if (tz < tx) siz = (float)tz / h;
      siz *= COMMONDISTANCE;
      siz *= distFact;
      if (border) if (((x+z)&7)==0) siz = maxX-minX+maxZ-minZ; // a bit too big
      e->distanceThresholdSquared = siz * siz;
    }
  }
  scape.resize(k);
}

/**
* Places the objects by an rgba picture with dots for each object.
* The colorchannels are evaluated as values r=0 is no object, r = 4 is object with type 1, r = 8 is object with type 2..
*
* @param rgba For every pixel rgba telling of the object type to set here. r,g,b are evaluated if each element having 0..63 possible values.
* @param w The object pictures width(x).
* @param h The object pictures height(z).
* @example scape->setObjects(objects,1024,1024);
*/
void Landscape::setObjects(unsigned int *rgba, unsigned int w, unsigned int h) {
  for (int z = 0; z < h; z++) {
    for (int x = 0; x < w; x++) {
      unsigned int p = rgba[x+z*w];
      int p0 = (p & 255) / 4; // 0-63
      if (p0 != 0) {
        int p1 = ((p>>8) & 255) / 4; // 0-63
        int p2 = ((p>>16) & 255) / 4; // 0-63
        int p3 = ((p>>24) & 255) / 4; // 0-63
        float px = (maxX-minX)*x/w+minX;
        float pz = (maxZ-minZ)*z/h+minZ;
        float py = getHeight(px,pz);
        scape.push_back(LandscapeElement());
        LandscapeElement *e = &scape.back();
        e->x = px;
        e->y = py;
        e->z = pz;
        e->v0 = p0;
        e->v1 = p1;
        e->v2 = p2;
        e->type = LANDSCAPE_TYPE_OBJECT;
        float siz = ((maxX-minX)+(maxZ-minZ))*0.05;
        switch(p0-1) {
        case 3: // portal
        case 4: {siz *= 3;} break; // dragon
        }
        e->distanceThresholdSquared = siz * siz;
      }
    }
  }
}

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
void Landscape::insertEmpty(float x2, float z2, int w, int h, float distFact) {
  scape.push_back(LandscapeElement());
  LandscapeElement *e = &scape.back();
  e->x = (maxX-minX)*x2/w+minX;
  e->z = (maxZ-minZ)*z2/h+minZ;
  e->y = getHeight(e->x,e->z);
  e->type = LANDSCAPE_TYPE_HEIGHT;
  int x = floor(x2);
  int z = floor(z2);
  if (x < 0) x = 0;
  if (z < 0) z = 0;
  if (x >= w) x = w-1;
  if (z >= h) z = h-1;
  unsigned short *map = map_height;
  const unsigned short v__ = map[x+z*w];
  const unsigned short vn_ = map[((x-1)<0?0:(x-1))+z*w];
  const unsigned short vp_ = map[((x+1)>=w?w-1:(x+1))+z*w];
  const unsigned short v_n = map[x+((z-1)<0?0:(z-1))*w];
  const unsigned short v_p = map[x+((z+1)>=h?h-1:(z+1))*w];
  float dx = vp_ - vn_;
  float dy = v_p - v_n;
  float d = sqrt(dx*dx + dy*dy);
  if (d!=0) {dx /=d; dy /= d;}
  e->v0 = 128+dx * 127;
  e->v1 = map_boden[x+z*w];
  e->distanceThresholdSquared = distFact;
}


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
float Landscape::hitThresh(float center, float left, float right, float thresh) {
  // center is always bigger thresh here
  if (left < thresh) {
    const float dist0 = left - thresh;
    const float dist1 = center - thresh;
    // 0 = dist0 * (1-a) + dist1 * a;
    // 0 = dist0 - dist0 * a + dist1 * a
    // -dist0 = -dist0 * a + dist1 * a
    // -dist0 = (-dist0 + dist1)*a
    // -dist0/(-dist0 + dist1) = a
    const float k = (-dist0+dist1);
    if (k == 0) return 0;
    return -dist0/k - 1;
  }
  if (right < thresh) {
    const float dist0 = center - thresh;
    const float dist1 = right - thresh;
    const float k = (-dist0+dist1);
    if (k == 0) return 0;
    return -dist0/k;
  }
  return 0;
}

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
void Landscape::setTrees(unsigned char *mask, unsigned char *map, unsigned int w, unsigned int h, int randomModulo) {
  const float COMMONDISTANCE = 200.f;
  const float DISTANCERAND = 200.f;
  srand(0);
  for (int z = 0; z < h; z++) {
    for (int x = 0; x < w; x++) {
      const int r1 = rand();
      const int r2 = rand();
      const int r3 = rand();
      const int r4 = rand();
      const int r5 = rand();
      const int r6 = rand() & 255;
      if (mask[x+z*w]) continue;
      const unsigned char v__ = map[x+z*w];
      if (v__!=0) {
        if ((r1 % randomModulo)==0) {
          scape.push_back(LandscapeElement());
          LandscapeElement *e = &scape.back();
          e->x = (maxX-minX)*(x)/w+minX;
          e->z = (maxZ-minZ)*(z)/h+minZ;
          int bigTree = r6/220 > 0 ? 1 : 0;
          e->y = getHeight(e->x,e->z)-0.25-0.75*bigTree;
          e->type = LANDSCAPE_TYPE_TREE;
          e->v0 = r3 & 255; // brightness
          e->v1 = r4 & 255; // height
          e->v2 = (r6 & 1)+bigTree*128; // tree scale 2
          const float siz = COMMONDISTANCE+(r5 & 255)/255.f*DISTANCERAND;
          const float d = siz * siz;
          e->distanceThresholdSquared = d;
        }
      }
    }
  }
}

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
void Landscape::setGrass(unsigned char *mask, unsigned char *map, unsigned int w, unsigned int h, int randomModulo) {
  const float DISTANCERAND = 200.f;
  srand(0);
  for (int z = 0; z < h; z++) {
    for (int x = 0; x < w; x++) {
      const int r1 = rand();
      const int r2 = rand();
      const int r3 = rand();
      const int r4 = rand();
      const int r5 = rand();
      const int r6 = rand();
      const int r7 = rand();
      if (mask[x+z*w]) continue;
      const unsigned char v__ = map[x+z*w];
      if (v__!=0) {
        const float px0 = (maxX-minX)*(x)/w+minX;
        const float py0 = (maxZ-minZ)*(z)/h+minZ;
        const float k = 3.f * (maxX-minX) / w;
        const float dx = getHeight(px0+k,py0) - getHeight(px0-k,py0);
        const float dy = getHeight(px0,py0+k) - getHeight(px0,py0-k);
        const float d = sqrt(dx*dx+dy*dy);
        bool notSteep = d < 0.5;
        if ((r1 % randomModulo)==0&&notSteep) {
          scape.push_back(LandscapeElement());
          float ox = (float)(r6 & 255)/255.f;
          float oz = (float)(r7 & 255)/255.f;
          LandscapeElement *e = &scape.back();
          e->x = (maxX-minX)*(x+ox)/w+minX;
          e->z = (maxZ-minZ)*(z+oz)/h+minZ;
          e->y = getHeight(e->x,e->z);
          e->type = LANDSCAPE_TYPE_GRASS;
          const float px = e->x * 0.25;
          const float pz = e->z * 0.25;
          float f = sin(px+pz+sin(px*0.4-pz*0.2)+cos(px*0.7)-sin(pz*0.9))*0.5+0.5;
          e->v0 = f*8+18+4; // lightness
          e->v1 = (r3 & 255); // overall size
          e->v2 = (r4 & 255); // width
          const float siz = DISTANCERAND * ((r5 & 255) / 255.f*0.75+0.25);
          const float d = siz * siz;
          e->distanceThresholdSquared = d;
        }
      }
    }
  }
}

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
void Landscape::setStones(unsigned char *map, unsigned int w, unsigned int h, int threshOuter, int threshCleanup) {
  const float COMMONDISTANCE = 250.f;
  srand(0);
  { // placing the actual stone heights here
    for (int z = 0; z < map_height_h; z++) {
      for (int x = 0; x < map_height_w; x++) {
        const int r2 = rand() & 255;
        const int rx = x * w / map_height_w;
        const int rz = z * h / map_height_h;
        const unsigned char v__ = map[rx+rz*w];
        if (v__ >= threshOuter) {
          const float ex = (maxX-minX)*x/w+minX;
          const float ez = (maxZ-minZ)*z/h+minZ;
          const float px = ex*0.1;
          const float pz = ez*0.1;
          float f = (sin(px+pz+sin(px)+cos(pz)) * 0.5 + 0.5 + 0.2)*3*(1.0+r2/255.f*0.25);
          const float y = getHeight(ex,ez)+f;
          putHeight(ex,ez,y);
        }
      }
    }
  }

  srand(0);
  for (int z = 0; z < h; z++) {
    for (int x = 0; x < w; x++) {
      const int r1 = rand() & 255;
      const unsigned char v__ = map[x+z*w];
      const unsigned char vn_ = map[((x-1)<0?0:(x-1))+z*w];
      const unsigned char vp_ = map[((x+1)>=w?w-1:(x+1))+z*w];
      const unsigned char v_n = map[x+((z-1)<0?0:(z-1))*w];
      const unsigned char v_p = map[x+((z+1)>=h?h-1:(z+1))*w];
      float gradX = (vp_ - vn_)/255.0;
      float gradZ = (v_p - v_n)/255.0;
      float grad = sqrt(gradX*gradX+gradZ*gradZ); grad=grad*grad*9;
      const float siz = COMMONDISTANCE * (grad*2+0.01);
      const float d = siz * siz;
      const bool some = v__ >= threshOuter;
      const bool someNotAround = (vn_ < threshOuter) || (vp_ < threshOuter) || (v_n < threshOuter) || (v_p < threshOuter);
      if (some && someNotAround) {
        scape.push_back(LandscapeElement());
        LandscapeElement *e = &scape.back();
        const float xd = hitThresh(v__,vn_,vp_,threshOuter);
        const float zd = hitThresh(v__,v_n,v_p,threshOuter);
        e->x = (maxX-minX)*(x+xd)/w+minX;
        e->z = (maxZ-minZ)*(z+zd)/h+minZ;
        e->y = getHeight(e->x,e->z);
        e->type = LANDSCAPE_TYPE_STONE;
        e->v0 = r1;
        e->distanceThresholdSquared = d;
      }
      const bool some3 = v__ >= threshCleanup;
      const bool someNotAround3 = (vn_ < threshCleanup) || (vp_ < threshCleanup) || (v_n < threshCleanup) || (v_p < threshCleanup);
      if (some3 && someNotAround3) {
        const float xd = hitThresh(v__,vn_,vp_,threshCleanup);
        const float zd = hitThresh(v__,v_n,v_p,threshCleanup);
        insertEmpty(x+xd,z+zd,w,h,d);
      }
    }
  }
}

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
void Landscape::setWater(unsigned char *map, unsigned int w, unsigned int h, int threshOuter, int threshCleanup) {
  const float COMMONDISTANCE = 400.f;
  int xk = w / 16;
  int zk = h / 16;
  srand(0);
  for (int z = 0; z < h; z++) {
    for (int x = 0; x < w; x++) {
      const unsigned char v__ = map[x+z*w];
      const unsigned char vn_ = map[((x-1)<0?0:(x-1))+z*w];
      const unsigned char vp_ = map[((x+1)>=w?w-1:(x+1))+z*w];
      const unsigned char v_n = map[x+((z-1)<0?0:(z-1))*w];
      const unsigned char v_p = map[x+((z+1)>=h?h-1:(z+1))*w];
      float gradX = (vp_ - vn_)/255.0;
      float gradZ = (v_p - v_n)/255.0;
      float grad = sqrt(gradX*gradX+gradZ*gradZ); grad = grad * grad * 9; grad = grad * grad * 9; grad = grad * grad * 9;
      const float siz = COMMONDISTANCE * (grad*3+0.01);
      const float d = siz * siz;
      const bool some = v__ >= threshOuter;
      const bool someNotAround = (vn_ < threshOuter) || (vp_ < threshOuter) || (v_n < threshOuter) || (v_p < threshOuter);
      const bool fillPoint = false;//some&&(((x % xk)==0)&&((z % zk)==0));
      if ((some && someNotAround) || fillPoint) {
        scape.push_back(LandscapeElement());
        LandscapeElement *e = &scape.back();
        const float xd = hitThresh(v__,vn_,vp_,threshOuter);
        const float zd = hitThresh(v__,v_n,v_p,threshOuter);
        e->x = (maxX-minX)*(x+xd)/w+minX;
        e->z = (maxZ-minZ)*(z+zd)/h+minZ;
        e->y = getHeight(e->x,e->z);
        e->type = LANDSCAPE_TYPE_WATER;
        e->distanceThresholdSquared = fillPoint ? COMMONDISTANCE : d;
      }
      const bool some3 = v__ >= threshCleanup;
      const bool someNotAround3 = (vn_ < threshCleanup) || (vp_ < threshCleanup) || (v_n < threshCleanup) || (v_p < threshCleanup);
      if (some3 && someNotAround3) {
        const float xd = hitThresh(v__,vn_,vp_,threshCleanup);
        const float zd = hitThresh(v__,v_n,v_p,threshCleanup);
        insertEmpty(x+xd,z+zd,w,h,d);
      }
    }
  }
}

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
void Landscape::setFlowers(unsigned char *mask, unsigned char *map, unsigned int w, unsigned int h, int randomModulo) {
  const float RANDDISTANCE = 75.f;
  srand(0);
  for (int z = 0; z < h; z++) {
    for (int x = 0; x < w; x++) {
      const int r1 = rand();
      const int r2 = rand();
      const int r3 = rand();
      const int r4 = rand();
      const int r5 = rand();
      const int r6 = rand();
      const int r7 = rand();
      if (mask[x+z*w]) continue;
      const unsigned char v__ = map[x+z*w];
      if (v__!=0) {
        if ((r1 % randomModulo)==0) {
          scape.push_back(LandscapeElement());
          LandscapeElement *e = &scape.back();
          e->x = (maxX-minX)*(x)/w+minX;
          e->z = (maxZ-minZ)*(z)/h+minZ;
          e->y = getHeight(e->x,e->z)+0.5;
          e->type = LANDSCAPE_TYPE_FLOWER;
          const float px = e->x * 0.5;
          const float pz = e->z * 0.5;
          float f = sin(px+pz+sin(px*0.4-pz*0.2)+cos(px*0.7)-sin(pz*0.9))*0.5+0.5;
          if ((r3 & 7)==0) f = (r4 & 7)/7.f;
          e->v0 = (int)(f*4) & 3; // color
          e->v1 = r5 & 255; // phase
          e->v2 = r7 & 255; // size
          const float siz = RANDDISTANCE * ((r5 & 255) / 255.f*0.75+0.25) * (e->v2/255.f*0.5+0.5);
          const float d = siz * siz;
          e->distanceThresholdSquared = d;
        }
      }
    }
  }
}

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
void Landscape::setRoads(unsigned char *map, unsigned int w, unsigned int h, int threshWayOuter, int threshWayInner,int threshCleanupMuchOuta) {

  srand(0);
  {
    for (int z = 0; z < map_height_h; z++) {
      for (int x = 0; x < map_height_w; x++) {
        const float px = (maxX-minX)*x/w+minX;
        const float pz = (maxZ-minZ)*z/h+minZ;
        const int rx = x * w / map_height_w;
        const int rz = z * h / map_height_h;
        const unsigned char v__ = map[rx+rz*w];
        if (v__ >= threshWayOuter) {
          const float f = (sin(px+pz+sin(px*0.4-pz*0.2)+cos(px*0.7)-sin(pz*0.9))*0.5+0.5)*0.5+0.5;
          const float depth = 0.25;
          const float y = getHeight(px,pz)-f*depth;
          putHeight(px,pz,y);
        }
      }
    }
  }
  for (int z = 0; z < h; z++) {
    for (int x = 0; x < w; x++) {
      const unsigned char v__ = map[x+z*w];
      const unsigned char vn_ = map[((x-1)<0?0:(x-1))+z*w];
      const unsigned char vp_ = map[((x+1)>=w?w-1:(x+1))+z*w];
      const unsigned char v_n = map[x+((z-1)<0?0:(z-1))*w];
      const unsigned char v_p = map[x+((z+1)>=h?h-1:(z+1))*w];
      float gradX = (vp_ - vn_)/255.0;
      float gradZ = (v_p - v_n)/255.0;
      float grad = sqrt(gradX*gradX+gradZ*gradZ); grad=grad*grad*9;
      const float siz = 500.0 * (grad*3+0.01);
      const float d = siz * siz;
      const bool some = v__ >= threshWayOuter;
      const bool someNotAround = (vn_ < threshWayOuter) || (vp_ < threshWayOuter) || (v_n < threshWayOuter) || (v_p < threshWayOuter);
      if (some && someNotAround) {
        scape.push_back(LandscapeElement());
        LandscapeElement *e = &scape.back();
        const float xd = hitThresh(v__,vn_,vp_,threshWayOuter);
        const float zd = hitThresh(v__,v_n,v_p,threshWayOuter);
        e->x = (maxX-minX)*(x+xd)/w+minX;
        e->z = (maxZ-minZ)*(z+zd)/h+minZ;
        e->y = getHeight(e->x,e->z);
        e->type = LANDSCAPE_TYPE_ROAD;
        e->v0 = 0;
        e->distanceThresholdSquared = d;
       
        if ((rand() & 7)==0) {
          scape.push_back(LandscapeElement());
          LandscapeElement *e = &scape.back();
          e->x = (maxX-minX)*(x+xd)/w+minX;
          e->z = (maxZ-minZ)*(z+zd)/h+minZ;
          e->y = getHeight(e->x,e->z);
          e->type = LANDSCAPE_TYPE_GRASS;
          e->v0 = (rand() % 0x04)+16; // lightness
          e->v1 = 200; // overall size
          e->v2 = 0; // width
          const float siz = 200.0 * ((rand() & 255) / 255.f*0.75+0.25);
          const float d = siz * siz;
          e->distanceThresholdSquared = d;
        }
      }
      const bool some2 = v__ >= threshWayInner;
      const bool someNotAround2 = (vn_ < threshWayInner) || (vp_ < threshWayInner) || (v_n < threshWayInner) || (v_p < threshWayInner);
      if (some2 && someNotAround2) {
        scape.push_back(LandscapeElement());
        LandscapeElement *e = &scape.back();
        const float xd = hitThresh(v__,vn_,vp_,threshWayInner);
        const float zd = hitThresh(v__,v_n,v_p,threshWayInner);
        e->x = (maxX-minX)*(x+xd)/w+minX;
        e->z = (maxZ-minZ)*(z+zd)/h+minZ;
        e->y = getHeight(e->x,e->z);
        e->type = LANDSCAPE_TYPE_ROAD;
        e->distanceThresholdSquared = d;
        const float px = e->x;
        const float pz = e->z;
        const float f = (sin(px+pz+sin(px*0.4-pz*0.2)+cos(px*0.7)-sin(pz*0.9))*0.5+0.5)*0.5+0.5;
        e->v0 = f*255.0;

        if ((rand() & 15)==0) {
          scape.push_back(LandscapeElement());
          LandscapeElement *e = &scape.back();
          e->x = (maxX-minX)*(x+xd)/w+minX;
          e->z = (maxZ-minZ)*(z+zd)/h+minZ;
          e->y = getHeight(e->x,e->z);
          e->type = LANDSCAPE_TYPE_GRASS;
          e->v0 = (rand() % 0x04)+16; // lightness
          e->v1 = 100; // overall size
          e->v2 = 0; // width
          const float siz = 200.0 * ((rand() & 255) / 255.f*0.75+0.25);
          const float d = siz * siz;
          e->distanceThresholdSquared = d;
        }
      }
      const bool some3 = v__ >= threshCleanupMuchOuta;
      const bool someNotAround3 = (vn_ < threshCleanupMuchOuta) || (vp_ < threshCleanupMuchOuta) || (v_n < threshCleanupMuchOuta) || (v_p < threshCleanupMuchOuta);
      if (some3 && someNotAround3) {
        const float xd = hitThresh(v__,vn_,vp_,threshCleanupMuchOuta);
        const float zd = hitThresh(v__,v_n,v_p,threshCleanupMuchOuta);
        insertEmpty(x+xd,z+zd,w,h,d);
      }
    }
  }
}
