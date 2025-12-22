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
#include "T_EDIT.HPP"
#include "IMAGE.HPP"
#include "VECTOR.HPP"
#include "T_MAP.HPP"
#include "T_DLNAY.HPP"

/**
* Constructor with landscape and camera properties.
*
* @param _scape The landscape to update.
* @param _raw The Delaunay landscape to update.
* @param _cameraPos The camera position for Delaunay updates.
* @param _details A pointer to a detailScale setting.
* @example LandscapeEdit *e = new LandscapeEdit(scape,raw,&cameraPos,&details);
*/
LandscapeEdit::LandscapeEdit(class Landscape *_scape, class LandscapeRaw *_raw, class Vector *_cameraPos, const double *_details) {
  scape = _scape;
  raw = _raw;
  cameraPos = _cameraPos;
  details = _details;
  objects = NULL;
}

/**
* Destructor, destroys the RGBAImage (objects)
*/
LandscapeEdit::~LandscapeEdit() {
  if (objects != NULL) {
    objects->free();
    objects = NULL;
  }
}

/**
* A function to set the actual "placed objects" png and to initially load it.
*
* @param fileName The png file name for the png with the placed objects (as pixels). If it is not there it gets created.
* @param w The initial width. If the png isn't there it's newly generated with this width(x).
* @param h The initial height. If the png isn't there it's newly generated with this height(z).
* @example e->setObjectsFile("data/maps/1/objects.png",1024,1024);
*/
void LandscapeEdit::setObjectsFile(const String &fileName, int w, int h) {
  objects_fileName = fileName;
  static RGBAImage o; 
  o.free();
  o = RGBAImage::fromFile(objects_fileName.c_str());
  objects = &o;
  if (objects->data == NULL) {
    objects->width = w;
    objects->height = h;
    objects->data = new unsigned int[objects->width * objects->height];
    memset(objects->data, 0, sizeof(unsigned int) * objects->width * objects->height);
  }
}

/**
* This function refreshes the objects in the actual game.
*/
void LandscapeEdit::refreshObjects() {
  scape->removeElementsWithType(LANDSCAPE_TYPE_OBJECT);
  scape->setObjects(objects->data, objects->width, objects->height);
  raw->update(*cameraPos, *details);
}

/**
* This function places an additional object to the map.
*
* @param pos The world space position of the object.
* @param objectId The objectID of the object to place.
* @example edit->placeObject(&Vector(0,0,0), 1);
*/  
void LandscapeEdit::placeObject(class Vector *pos, int objectId) {
  const int px = (float)(pos->x - scape->minX) / (scape->maxX - scape->minX) * objects->width;
  const int pz = (float)(pos->z - scape->minZ) / (scape->maxZ - scape->minZ) * objects->height;
  if ((unsigned int)px >= objects->width) return;
  if ((unsigned int)pz >= objects->height) return;
  int r = (objectId & 63) * 4 + 2;
  int g = 0;
  int b = 0;
  int a = 0;
  objects->data[px+pz*objects->width] = r|(g<<8)|(b<<16)|(a<<24);
  objects->savePNG(objects_fileName.c_str());
  refreshObjects();
}

/**
* This function rotates an object on the map around the Y axis.
* It rotates all objects in a "certain" box radius around pos.
*
* @param pos The world space position of the object.
* @param rotation a value 0..7 (8 directions).
* @example edit->rotateObjectY(&Vector(0,0,0), 4); For a 360/8*4=180 degree rotation.
*/  
void LandscapeEdit::rotateObjectY(class Vector *pos, int rotation) {
  const int px = (float)(pos->x - scape->minX) / (scape->maxX - scape->minX) * objects->width;
  const int pz = (float)(pos->z - scape->minZ) / (scape->maxZ - scape->minZ) * objects->height;
  const int box = 2;
  bool some = false;
  for (int z = -box+pz; z <= box+pz; z++) {
    for (int x = -box+px; x <= box+px; x++) {
      if ((unsigned int)x >= objects->width) continue;
      if ((unsigned int)z >= objects->height) continue;
      const unsigned int rgba = objects->data[x+z*objects->width];
      int r = (rgba & 255);
      if (r != 0) {
        some = true;
        int g = rotation * 4 + 2;
        int b = ((rgba>>16) & 255);
        int a = ((rgba>>24) & 255);
        objects->data[x+z*objects->width] = r|(g<<8)|(b<<16)|(a<<24);
      }
    }
  }
  if (some) {
    objects->savePNG(objects_fileName.c_str());
    refreshObjects();
  }
}

/**
* This function deletes an object/objects on the map.
* It deletes all objects in a "certain" box radius around pos.
*
* @param pos The world space position of the object.
* @example edit->removeObjects(&Vector(0,0,0));
*/  
void LandscapeEdit::removeObjects(const Vector *pos) {
  const int px = (float)(pos->x - scape->minX) / (scape->maxX - scape->minX) * objects->width;
  const int pz = (float)(pos->z - scape->minZ) / (scape->maxZ - scape->minZ) * objects->height;
  const int box = 2;
  for (int z = -box+pz; z <= box+pz; z++) {
    for (int x = -box+px; x <= box+px; x++) {
      if ((unsigned int)x >= objects->width) continue;
      if ((unsigned int)z >= objects->height) continue;
      int r = 0;
      int g = 0;
      int b = 0;
      int a = 0;
      objects->data[x+z*objects->width] = r|(g<<8)|(b<<16)|(a<<24);
    }
  }
  objects->savePNG(objects_fileName.c_str());
  refreshObjects();
}
