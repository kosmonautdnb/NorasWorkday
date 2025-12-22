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
#ifndef __T_EDIT_HPP__
#define __T_EDIT_HPP__ 

#include "../STL/STRING.HPP"

/**
* A small class for an inplace landscape editing functionality
*/
class LandscapeEdit {

public:

  /// The landscape elements and heightmap.
  class Landscape *scape;
  /// The Delaunay representation of the landscape.
  class LandscapeRaw *raw;
  /// The placed objects in a png, the file name of the png.
  String objects_fileName;
  /// The placed objects png.
  class RGBAImage *objects;
  /// A pointer to the world space camera position vector.
  class Vector *cameraPos;
  /// A pointer to the detail scale factor. (maybe pointing mostly to a 1.0)
  const double *details;

  /**
  * Constructor with landscape and camera properties.
  *
  * @param _scape The landscape to update.
  * @param _raw The Delaunay landscape to update.
  * @param _cameraPos The camera position for Delaunay updates.
  * @param _details A pointer to a detailScale setting.
  * @example LandscapeEdit *e = new LandscapeEdit(scape,raw,&cameraPos,&details);
  */
  LandscapeEdit(class Landscape *_scape, class LandscapeRaw *_raw, class Vector *_cameraPos, const double *_details);

  /**
  * Destructor, destroys the RGBAImage (objects)
  */
  ~LandscapeEdit();
  
  /**
  * A function to set the actual "placed objects" png and to initially load it.
  *
  * @param fileName The png file name for the png with the placed objects (as pixels). If it is not there it gets created.
  * @param w The initial width. If the png isn't there it's newly generated with this width(x).
  * @param h The initial height. If the png isn't there it's newly generated with this height(z).
  * @example e->setObjectsFile("data/maps/1/objects.png",1024,1024);
  */
  void setObjectsFile(const String &fileName, int w, int h);

  /**
  * This function refreshes the objects in the actual game.
  */
  void refreshObjects();

  /**
  * This function places an additional object to the map.
  *
  * @param pos The world space position of the object.
  * @param objectId The objectID of the object to place.
  * @example edit->placeObject(&Vector(0,0,0), 1);
  */  
  void placeObject(class Vector *pos, int objectId);
  
  /**
  * This function rotates an object on the map around the Y axis.
  * It rotates all objects in a "certain" box radius around pos.
  *
  * @param pos The world space position of the object.
  * @param rotation a value 0..7 (8 directions).
  * @example edit->rotateObjectY(&Vector(0,0,0), 4); For a 360/8*4=180 degree rotation.
  */  
  void rotateObjectY(class Vector *pos, int rotation);
  
  /**
  * This function deletes an object/objects on the map.
  * It deletes all objects in a "certain" box radius around pos.
  *
  * @param pos The world space position of the object.
  * @example edit->removeObjects(&Vector(0,0,0));
  */  
  void removeObjects(const Vector *pos);

};

#endif // __T_EDIT_HPP__ 
