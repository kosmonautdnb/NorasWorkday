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
#include "O_WAVOBJ.HPP"
#include "SMPLOBJL.HPP"
#include "GL.H"
#include "IMAGE.HPP"
#include <math.h> // pow

/**
* Constructor of a single polygon/face of an OBJ representation.
*/
WAVOBJ_Face::WAVOBJ_Face() {
  numVerts=0;
}

/**
* Constructor of an WaveFront OBJ Material.
*/
WAVOBJ_Material::WAVOBJ_Material() {
  texture = 0; 
  colorDiffuse = Vector(1,1,1,1); 
  colorSpecular = Vector(1,1,1,1); 
  shininess = 1;
}

/**
* Constructor for an empty part.
*/
WAVOBJ_MeshPart::WAVOBJ_MeshPart() {
  materialId = -1;
  boundingRadius = -1;
}

/**
* Constructor creating an empty mesh.
*/
WAVOBJ_Mesh::WAVOBJ_Mesh() {
  boundingRadius = -1;
}

/**
* A function to calculate the bounding boxes and stuff of the mesh and of its batches.
*
* @param m The mesh to apply bounding boxes and stuff.
* @example preprocessMesh(mesh);
*/
static void preprocessMesh(WAVOBJ_Mesh *m) {
  Vector bbmax;
  Vector bbmin;
  {for (int i = 0; i < m->positions.size(); i++) {
    Vector *v = &m->positions[i];
    if (i==0 || v->x > bbmax.x) bbmax.x = v->x;
    if (i==0 || v->y > bbmax.y) bbmax.y = v->y;
    if (i==0 || v->z > bbmax.z) bbmax.z = v->z;
    if (i==0 || v->x < bbmin.x) bbmin.x = v->x;
    if (i==0 || v->y < bbmin.y) bbmin.y = v->y;
    if (i==0 || v->z < bbmin.z) bbmin.z = v->z;
  }}
  m->minBounding = bbmin;
  m->maxBounding = bbmax;
  m->center = (bbmin+bbmax)*0.5;
  m->boundingRadius = length(bbmax-m->center);
  {for (int i = 0; i < m->parts.size(); i++) {
    int l = 0;
    WAVOBJ_MeshPart *p = &m->parts[i];
    for (int j = 0; j < p->faces.size(); j++) {
      for (int k = 0; k < p->faces[j].numVerts; k++) {
        Vector *v = &m->positions[p->faces[j].p[k]];
        if (l==0 || v->x > bbmax.x) bbmax.x = v->x;
        if (l==0 || v->y > bbmax.y) bbmax.y = v->y;
        if (l==0 || v->z > bbmax.z) bbmax.z = v->z;
        if (l==0 || v->x < bbmin.x) bbmin.x = v->x;
        if (l==0 || v->y < bbmin.y) bbmin.y = v->y;
        if (l==0 || v->z < bbmin.z) bbmin.z = v->z;
        l++;
      }
    }
    p->minBounding = bbmin;
    p->maxBounding = bbmax;
    p->center = (bbmin+bbmax)*0.5;
    p->boundingRadius = length(bbmax-m->center);
  }}
}

/**
* A callback to be used for texture loading for WaveFront OBJ files.
*
* @param fileName The file name of the texture to load.
* @param type The type of the texture to load.
* @return The OpenGL texture handle of the created texture.
* @example unsigned int texHandle = SMPL_loadTexture("bitmap.png","map_Kd");
*/
static unsigned int SMPL_loadTexture(const String &fileName, const String &type) {
  uint32_t texture = 0;
  if (type != "map_Kd") return texture;
  RGBAImage image = RGBAImage::fromFile(fileName.c_str());
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  image.free();
  return texture;
}

/**
* A callback to be used for texture loading for WaveFront OBJ files.
* This one is scaling down the texture by 8.
*
* @param fileName The file name of the texture to load.
* @param type The type of the texture to load.
* @return The OpenGL texture handle of the created texture.
* @example unsigned int texHandle = SMPL_loadTexture3("bitmap.png","map_Kd");
*/
static unsigned int SMPL_loadTexture3(const String &fileName, const String &type) {
  uint32_t texture = 0;
  if (type != "map_Kd") return texture;
  RGBAImage image = RGBAImage::fromFile(fileName.c_str());
  double scale = 8;
  image = image.getResized(image.width/scale,image.height/scale);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  image.free();
  return texture;
}

/**
* A callback to be used for texture loading for WaveFront OBJ files.
* This one is making red to green/cyanish.
*
* @param fileName The file name of the texture to load.
* @param type The type of the texture to load.
* @return The OpenGL texture handle of the created texture.
* @example unsigned int texHandle = SMPL_loadTexture2("bitmap.png","map_Kd");
*/
unsigned int SMPL_loadTexture2(const String &fileName, const String &type) {
  uint32_t texture = 0;
  if (type != "map_Kd") return texture;
  RGBAImage image = RGBAImage::fromFile(fileName.c_str());
  for (int i = 0; i < image.width*image.height; i++) {
    unsigned int rgba = image.data[i];
    int r = rgba & 255;
    int g = (rgba>>8) & 255;
    int b = (rgba>>16) & 255;
    int a = (rgba>>24) & 255;
    int t = g;
    g = r;
    r = t;
    b = (b+g)*0.5;
    const float c = 3.f;
    const float k = 6.f;
    r = pow((float)r/255.f,c)*255.f*k;
    g = pow((float)g/255.f,c)*255.f*k;
    b = pow((float)b/255.f,c)*255.f*k;
    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    image.data[i] = r|(g<<8)|(b<<16)|(a<<24);
  }
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  image.free();
  return texture;
}

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
WAVOBJ_Mesh *loadOBJ(const char *fileName, bool genFaceNormals, float scale, const Vector &add, bool genVertexNormals) {
  SMPL_File *mesh = loadObj(fileName, true);
  if (mesh == NULL) return NULL;
  if (genFaceNormals) mesh->genFaceNormals();
  if (genVertexNormals) mesh->genVertexNormals();
  WAVOBJ_Mesh *m = new WAVOBJ_Mesh();
  {
    m->colors.resize(mesh->materialsById.size());
    for (int i = 0; i < mesh->materialsById.size(); i++) {
      m->colors[i] = mesh->materialsById[i]->diffuse;
    }
  }
  {
    m->positions.resize(mesh->vertices.size());
    for (int i = 0; i < mesh->vertices.size(); i++) {
      m->positions[i] = mesh->vertices[i] * scale + add;
    }
  }
  {
    m->normals.resize(mesh->normals.size());
    for (int i = 0; i < mesh->normals.size(); i++) {
      m->normals[i] = mesh->normals[i];
    }
  }
  {
    m->texCoords.resize(mesh->texCoords.size());
    for (int i = 0; i < mesh->texCoords.size(); i++) {
      m->texCoords[i] = mesh->texCoords[i];
    }
  }
  {
    m->parts.resize(mesh->objs.size());
    for (int i = 0; i < mesh->objs.size(); i++) {
      WAVOBJ_MeshPart p;
      p.faces.resize(mesh->objs[i].faceEnd-mesh->objs[i].faceStart);
      for (int j = 0; j < p.faces.size(); j++) {
        WAVOBJ_Face *f = &p.faces[j];
        f->numVerts = mesh->faces[j+mesh->objs[i].faceStart].c;
        for (int k = 0; k < f->numVerts; k++) {
          f->p[k] = mesh->faces[j+mesh->objs[i].faceStart].v[k];
          f->n[k] = mesh->faces[j+mesh->objs[i].faceStart].n[k];
          f->c[k] = mesh->faces[j+mesh->objs[i].faceStart].m;
          f->t[k] = mesh->faces[j+mesh->objs[i].faceStart].t[k];
        }
      }
      m->parts[i] = p;
    }
  }

  delete mesh;
  preprocessMesh(m);
  return m;
}                                       

/**
* Loads an WaveFront .obj file with it's .mtl file if readable.
* This one is used for the trees in the game. They have a black shaft and texture sizes divided by a factor.
*
* @param fileName The name of the file to load. (*.obj with its *.mtl)
* @param genFaceNormals Set this to true if you need face normals calculated by yourself.
* @return The loaded mesh or NULL.
* @example WAVOBJ_Mesh *obj = loadTreeOBJ("tree.obj");
*/
WAVOBJ_Mesh *loadTreeOBJ(const char *fileName, bool genFaceNormals) {
  SMPL_File *mesh = loadObj(fileName, true);
  if (mesh == NULL) return NULL;
  if (genFaceNormals) mesh->genFaceNormals();
  mesh->loadTextures(SMPL_loadTexture3);
  int polys = 0;
  WAVOBJ_Mesh *m = new WAVOBJ_Mesh();
  {
    m->colors.resize(mesh->vertices.size());
    for (int i = 0; i < mesh->vertices.size(); i++) {
      float h = mesh->vertices[i].y/0.125;
      if (h < 0) h = 0;
      if (h > 1) h = 1;
      m->colors[i] = Vector(h,h,h);
    }
  }
  {
    m->positions.resize(mesh->vertices.size());
    for (int i = 0; i < mesh->vertices.size(); i++) {
      m->positions[i] = mesh->vertices[i];
    }
  }
  {
    m->normals.resize(mesh->normals.size());
    for (int i = 0; i < mesh->normals.size(); i++) {
      m->normals[i] = mesh->normals[i];
    }
  }
  {
    m->texCoords.resize(mesh->texCoords.size());
    for (int i = 0; i < mesh->texCoords.size(); i++) {
      m->texCoords[i] = mesh->texCoords[i];
    }
  }
  {
    m->parts.resize(mesh->objs.size());
    for (int i = 0; i < mesh->objs.size(); i++) {
      WAVOBJ_Material material;
      material.colorDiffuse = mesh->materialsById[i]->diffuse;
      material.colorSpecular = mesh->materialsById[i]->specular;
      material.texture = mesh->materialsById[i]->mapDiffuse.glHandle;
      m->materials.push_back(material);
      WAVOBJ_MeshPart p;
      p.faces.resize(mesh->objs[i].faceEnd-mesh->objs[i].faceStart);
      int k2 = 0;
      for (int j = 0; j < p.faces.size(); j++) {
        WAVOBJ_Face *f = &p.faces[k2];
        Vector v0 = mesh->vertices[mesh->faces[j+mesh->objs[i].faceStart].v[0]];
        Vector v1 = mesh->vertices[mesh->faces[j+mesh->objs[i].faceStart].v[1]];
        Vector v2 = mesh->vertices[mesh->faces[j+mesh->objs[i].faceStart].v[2]];
        Vector c = (v0+v1+v2)/3.0;
        double d = length(v0-c);
        const bool keepIt = d > 0.5||true;
        if (keepIt) {        
          f->numVerts = mesh->faces[j+mesh->objs[i].faceStart].c;
          for (int k = 0; k < f->numVerts; k++) {
            p.materialId = mesh->faces[j+mesh->objs[i].faceStart].m;
            f->p[k] = mesh->faces[j+mesh->objs[i].faceStart].v[k];
            f->n[k] = mesh->faces[j+mesh->objs[i].faceStart].n[k];
            f->c[k] = mesh->faces[j+mesh->objs[i].faceStart].v[k];
            f->t[k] = mesh->faces[j+mesh->objs[i].faceStart].t[k];
          }
          k2++;
          polys++;
        }
      }
      m->parts[i] = p;
    }
  }

  delete mesh;
  preprocessMesh(m);
  return m;
}                                       

/**
* A function that paints a single polygon of a WAVOBJ_Mesh
*
* @param m The mesh.
* @param f The face/polygon of the mesh.
* @example drawMeshPoly(mesh,poly);
*/
static void drawMeshPoly(WAVOBJ_Mesh *m, WAVOBJ_Face *f) {
  glBegin(f->numVerts == 3 ? GL_TRIANGLES : GL_QUADS);
  for (int i = 0; i < f->numVerts; i++) {
    if ((unsigned int)f->c[i]<m->colors.size()) glColor4dv(&m->colors[f->c[i]].x);
    if ((unsigned int)f->t[i]<m->texCoords.size()) glTexCoord2dv(&m->texCoords[f->t[i]].x);
    if ((unsigned int)f->n[i]<m->normals.size()) glNormal3dv(&m->normals[f->n[i]].x);
    glVertex4dv(&m->positions[f->p[i]].x);
  }
  glEnd();
}

/**
* A function to paint a WaveFront .obj with OpenGL.
*
* @param m The mesh to paint.
* @example paintMesh(mesh);
*/
void paintMesh(WAVOBJ_Mesh *m) {
  glColor4f(1,1,1,1);
  for (int i = 0; i < m->parts.size(); i++) {
    WAVOBJ_MeshPart *p = &m->parts[i];
    WAVOBJ_Material *material = NULL;
    if (p->materialId >= 0 && p->materialId < m->materials.size()) {
      material = &m->materials[p->materialId];
    }
    float c[4];
    bool textured = false;
    if (material != NULL) {
      Vector *d;
      d = &material->colorDiffuse; c[0] = d->x; c[1] = d->y; c[2] = d->z; c[3] = d->w; glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,c);
      d = &material->colorSpecular; c[0] = d->x; c[1] = d->y; c[2] = d->z; c[3] = d->w; glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,c);
      //d = &material->colorAmbient; c[0] = d->x; c[1] = d->y; c[2] = d->z; c[3] = d->w; glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,c);
      //d = &material->colorEmissive; c[0] = d->x; c[1] = d->y; c[2] = d->z; c[3] = d->w; glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,c);
      //c[0] = material->shininess; glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,c);
      glEnable(GL_TEXTURE_2D);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D,material->texture);
      textured = (material->texture != 0);
    }
    for (int j = 0; j < p->faces.size(); j++) {
      drawMeshPoly(m,&p->faces[j]);
    }
  }
}
