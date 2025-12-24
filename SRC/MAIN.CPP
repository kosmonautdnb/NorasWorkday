#include <stdio.h>
#include <math.h>
#include "GL.H"
#include "ARRAY.HPP"
#include "VECTOR.HPP"
#include "MATRIX.HPP"
#include "TYPES.HPP"
#include "KEYMTRIX.HPP"
#include "IMAGE.HPP"
#include "SPRTEOBJ.HPP"
#include "TRUETYPE.HPP"
#include "TERRAIN/T_COLL.HPP"
#include "TERRAIN/T_MAP.HPP"
#include "TERRAIN/T_DLNAY.HPP"
#include "TERRAIN/T_LAYERS.HPP"
#include "TERRAIN/T_EDIT.HPP"
#include "OBJECTS/O_GLTF.HPP"
#include "OBJECTS/O_WAVOBJ.HPP"
#ifdef __WATCOMC__
#include <i86.h>
#endif // __WATCOMC__
#ifdef __DJGPP__
#include <dos.h>
#endif // __DJGPP__

#ifndef PI
#define PI 3.14159265358979323846
#endif // PI

double mouseX = 320, mouseY = 240;

#define EDITLANDSCAPE
//#define LOGFPS
#define GL_LINEAR2 GL_LINEAR
#define FASTTEXTURING GL_FALSE
#define TREERTTSIZE 360

struct meminfo {
    unsigned LargestBlockAvail;
    unsigned MaxUnlockedPage;
    unsigned LargestLockablePage;
    unsigned LinAddrSpace;
    unsigned NumFreePagesAvail;
    unsigned NumPhysicalPagesFree;
    unsigned TotalPhysicalPages;
    unsigned FreeLinAddrSpace;
    unsigned SizeOfPageFile;
    unsigned Reserved[3];
} MemInfo;

void setBiosCursor(int x, int y) {
    union REGS r;
    memset(&r,0,sizeof(r));
    r.w.ax = 0x200;
    r.h.bh = 0;
    r.h.dl = (unsigned char)(x&255);
    r.h.dh = (unsigned char)(y&255);
    int386(0x10, &r, &r);
}

float fogDensity = 0.005;
float fogColor[4]={0.25,0.5,1.0,1};

void blitSpriteObjectSprite_(const SpriteObject *sp, const class Vector *pos, double scale, unsigned int color) {
  const bool fastVersion = true;
  if (fastVersion) {
    Matrix mv_,p_,mvp_;
    glGetDoublev(GL_MODELVIEW_MATRIX,mv_.m);
    glGetDoublev(GL_PROJECTION_MATRIX,p_.m);
    mvp_ = p_ * mv_;
    Vector q = transform(*pos,mvp_);
    if (q.z < 0) return;
    double f0 = fogDensity * q.z;
    f0 = exp(-f0*f0);
    if (f0 <  0) f0 = 0;
    if (f0 > 1) f0 = 1;
    int r = fogColor[0]*(1-f0)*255.0;
    int g = fogColor[1]*(1-f0)*255.0;
    int b = fogColor[2]*(1-f0)*255.0;
    int cor = (color & 255)*(f0);
    int cog = ((color>>8) & 255)*(f0);
    int cob = ((color>>16) & 255)*(f0);
    blitSpriteObjectSpriteNoTexture(sp,pos,scale,cor|(cog<<8)|(cob<<16)|(color&0xff000000),r|(g<<8)|(b<<16));
  } else {
    blitSpriteObjectSprite(sp,pos,scale,color);
  }
}

SpriteObject tree[2];


Landscape *scape;
LandscapeRaw *raw;
LandscapeCollision *collision;
LandscapeEdit *edit;

void paintPortal() {
  const double s = glSeconds();
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_QUADS);
  const float xp[4] = {1,-1,-1,1};
  const float yp[4] = {1,1,-1,-1};
  srand(0);
  for (int j = 0; j < 100; j++) {
    const double phase = (double)(rand() & 255) / 255.0;
    const double pos = fmod(s*0.5+phase,1.0);
    double xa = (double)((rand() & 511)-256)/256.0;
    double ya = (double)((rand() & 511)-256)/256.0;
    const double l = sqrt(xa*xa+ya*ya);
    if (l != 0) {xa /= l; ya /=l;}
    const double posX = xa * pos * 2.0;
    const double posY = ya * pos * 4.0 + 3.0;
    const double sizX = ((rand() & 255)+128)/256.0*0.25;
    const double sizY = ((rand() & 255)+128)/256.0*0.25;
    glColor4f(1,0.25f,0.25f,1-pos);
    for (int i = 0; i < 4; i++) {
      glVertex3f(xp[i]*sizX+posX,yp[i]*sizY+posY,0);
    }
  }
  glEnd();
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
}

const double r = 0.002;

Vector getTexCoord(const Vector &xz) {
  return Vector(xz.x*r+0.5,xz.z*r+0.5);
}

unsigned int *texData;

void applyNormalMap(RGBAImage *colors,RGBAImage *normals,float nx,float ny,float nz, float exp, float mul, float thresh) {
  float l = sqrt(nx*nx+ny*ny+nz*nz);
  if (l != 0) {l = 1.f/l;}
  nx *= l; ny *= l; nz *= l;
  for (int y = 0; y < colors->height; y++) {
    for (int x = 0; x < colors->width; x++) {
      unsigned int normal = normals->data[x+y*normals->width];
      unsigned int color = colors->data[x+y*colors->width];
      float nx2 = ((int)(normal & 255)-128)/128.f;
      float ny2 = ((int)((normal>>16) & 255)-128)/128.f;
      float nz2 = ((int)((normal>>8) & 255)-128)/128.f;
      double d = nx2*nx + ny2*ny + nz2*nz;
      d -= thresh;
      if (d < 0) d = 0;
      int r = color & 255;
      int g = (color>>8) & 255;
      int b = (color>>16) & 255;
      float m = 1.f + pow(d,exp)*mul;
      r *= m;
      g *= m;
      b *= m;
      if (r > 255) r = 255;
      if (g > 255) g = 255;
      if (b > 255) b = 255;
      if (r < 0) r = 0;
      if (g < 0) g = 0;
      if (b < 0) b = 0;
      colors->data[x+y*colors->width] = r|(g<<8)|(b<<16)|(color & 0xff000000);
    }
  }
}

void blitGrassBlade(unsigned int texture, float w, float h, const class Vector *pos, double scale, unsigned int color, float texAdd, float time) {
  Matrix k;
  glGetDoublev(GL_MODELVIEW_MATRIX, k.m);

  const float z2 = pos->x * k.m[0*4+2] + pos->y * k.m[1*4+2] + pos->z * k.m[2*4+2] + k.m[3*4+2];
  if (z2 > 0) return; // may conflict with some GL_PROJECTION matrices

  k = k.matrix3x3();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glTranslatef(pos->x, pos->y, pos->z);
  glMultMatrixd(transpose(k).m);
  const Vector mn = Vector(-w/2.f,0)*scale;
  const Vector mx = Vector(w/2.f,h)*scale;
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0.5);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glVertex4f(0,0,0,1);
  glBegin(GL_QUADS);
  glColor4ubv((GLubyte*)&color);
  glTexCoord2f(1+texAdd,0); glVertex3f(mx.x+cos(time+1)*scale,mx.y+sin(time)*scale,0);
  glTexCoord2f(0+texAdd,0); glVertex3f(mn.x+cos(time+1.5)*scale,mx.y+sin(time+0.1)*scale,0);
  glTexCoord2f(0+texAdd,1); glVertex3f(mn.x,mn.y,0);
  glTexCoord2f(1+texAdd,1); glVertex3f(mx.x,mn.y,0);
  glEnd();
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();
}

void blitFlower(Vector *pos, float siz) {
  //glEnable(GL_POINT_SPRITE);
  //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
  float c[3] = {1,0,0.1};
  float s = glFrameBufferWidth / 4;
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, c);
  glPointParameterf(GL_POINT_SIZE_MIN, 0);
  glPointParameterf(GL_POINT_SIZE_MAX, s);
  glPointSize(glFrameBufferWidth*0.025*siz);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0.5);
  glBegin(GL_POINTS);
  glVertex3f(pos->x, pos->y, pos->z);
  glEnd();
  glDisable(GL_ALPHA_TEST);
}

void blitSun(Vector *pos) {
  float c[3] = {1,0,0};
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, c);
  glPointParameterf(GL_POINT_SIZE_MIN, glFrameBufferWidth/4);
  glPointParameterf(GL_POINT_SIZE_MAX, glFrameBufferWidth/4);
  glPointSize(glFrameBufferWidth/4);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0.5);
  glEnable(GL_BLEND);
  glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
  glBegin(GL_POINTS);
  glVertex3f(pos->x, pos->y, pos->z);
  glEnd();
  glDisable(GL_BLEND);
  glDisable(GL_ALPHA_TEST);
}

double sfmod(double a, double b) {
  if (a>=0) return fmod(a,b);
  return fmod(a+b*((int)(-a/b)+10),b);
}

Vector tryWalk(const Vector &start, const Vector &end) {

  Vector r = end;
  double hitx,hitz,nx,nz;
  const bool hit = collision->collideLine(start.x, start.z, end.x, end.z, &hitx, &hitz, &nx, &nz);

  if (hit) {
    r.x = hitx;
    r.z = hitz;
    const double step = 1.0;
    Vector normal = normalize(Vector(nx,0,nz));
    if (length(normal)>0) {
      Vector refl = normalize(reflect(normalize(Vector(end.x-start.x,0,end.z-start.z)),normal));
      double x2 = r.x+refl.x*fabs(-normal.z);
      double z2 = r.z+refl.z*fabs(normal.x);
      const bool hit2 = collision->collideLine(r.x, r.z, x2, z2, &hitx, &hitz, &nx, &nz);
      if (hit2) {
        r.x = hitx;
        r.z = hitz;
      } else {
        r.x = x2;
        r.z = z2;
      }
    }
  }

  return r;
}

void drawLandscapeText(double xp, double zp, const char *text) {
  double yp = scape->getHeight(xp,zp) + 1;
  glColor4f(1,0,0,1);
  Vector v = Vector(xp,yp,zp);
  blitFlower(&v,1);
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_LINES);
  glColor4f(1,1,1,1);
  glVertex3f(xp,yp,zp);
  glVertex3f(xp,scape->getHeight(xp,zp),zp);
  glEnd();
  glDrawText3DTTF(0, xp, yp, zp, 1.f, text, 0xffffffff, 0.5, 1.f);
}

#define OBJECTSPNG "DATA/MAPS/1/OBJECTS.PNG"

const double details = 1.0;
Vector characterPos(0,0,0);
Vector cameraPos = characterPos;

void drawBirds(const Vector &pos, double time, WAVOBJ_Mesh *bird) {
  Matrix mv_,p_,mvp_;
  glGetDoublev(GL_MODELVIEW_MATRIX,mv_.m);
  glGetDoublev(GL_PROJECTION_MATRIX,p_.m);
  mvp_ = p_ * mv_;
  glDisable(GL_TEXTURE_2D);
  glColor4f(1,1,1,1);
  const double areaSize = 20.0;
  srand(0);
  for (int i = 0; i < 10; i++) {
    Vector p(sfmod(-pos.x+(rand()&255)*0.25,areaSize*2)-areaSize,0,sfmod(-time*(1+(rand()&255)/255.0)-pos.z+(rand()&255)*0.25,areaSize*2)-areaSize);
    p += pos;
    p.y = scape->getHeightBox(p.x,p.z,3)+3+(rand()&255)/255.0;
    Vector q = transform(p,mvp_);
    if (q.z < 0) continue;
    if (fabs(q.x)>fabs(q.w)*1.2) continue;
    if (fabs(q.y)>fabs(q.w)*1.2) continue;
    glPushMatrix();
    glTranslatef(p.x,p.y,p.z);
    paintMesh(bird);
    glPopMatrix();
  }
  glEnable(GL_TEXTURE_2D);
}

#define SUNWIDTH 256
#define SUNHEIGHT 256
unsigned int sun[SUNWIDTH*SUNHEIGHT];

int main(int argc, const char **argv) {

  glFastTexturing = FASTTEXTURING;

  class GLTFA_File *girl;
  girl = loadGLTF_Character("c:/meshes/vd_grl1f.glb",GLTF_OBJECTID_MAINCHARACTER_NORMAL);

  glWatcomPrecisionTimer(GL_TRUE);
#ifdef LOGFPS
  glVesa(320,200,32);
#else

#ifdef __WATCOMC__
  //glVGA();
  glVesa(320,200,32);
#else
  glVesa(640,480,16);
#endif

#endif
  //glVesa(640,480,32);
  glRefresh();

  unsigned int font = stb_initfont(0, "DATA/FONTS/LIBESANS.TTF", 32.f);

  glAdditionalPointSpriteXStretch((double)glFrameBufferWidth/glFrameBufferHeight*9.0/16.0);

  installKeyboardHandler();

  createSpriteObjectFrameBuffer(&tree[0],TREERTTSIZE,TREERTTSIZE);
 // createSpriteObjectFrameBuffer(&tree[1],256,256);
  //Mesh *tre = loadOBJ("c:/MESHES/TREE/PINE2.OBJ");
  WAVOBJ_Mesh *tre[2];
  //tre[0] = loadOBJ("c:/MESHES/TREE2/TREE.OBJ");
  tre[0] = loadTreeOBJ("c:/MESHES/EZTREE/PINELRG/PINELRG.OBJ");
  WAVOBJ_Mesh *objs[16];
  objs[0] = loadOBJ("c:/MESHES/LAMPS/SLAMP1_B.OBJ",false,1);
  objs[1] = loadOBJ("c:/MESHES/LAMPS/FENCE_S1.OBJ",false,0.5);
  objs[2] = loadOBJ("c:/MESHES/LAMPS/SHIELD_1.OBJ",false,0.5);
  objs[4] = loadOBJ("c:/MESHES/dragon/dragon1.OBJ",false,0.75);
  objs[5] = loadOBJ("c:/MESHES/ghosts/ghost1.OBJ",false,0.25, Vector(0,0.5,0), true);
  unsigned int dragonTexture = SMPL_loadTexture2("c:/MESHES/dragon/dbody.jpg","map_Kd");
  //tre[1] = loadOBJ("c:/MESHES/TREE3/TREE.OBJ", true);
  WAVOBJ_Mesh *bird = loadOBJ("c:/MESHES/pigeon1.OBJ",false,1.0);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);
  glDepthFunc(GL_LEQUAL);

#define GRASSW 128
#define GRASSH 16
  srand(0);
  unsigned int grass_[GRASSW*GRASSH];
  {for (int x = 0; x < GRASSW; x++) {
    int h = (rand() % GRASSH)+1;
    int k2 = (rand() % 32);
    float k = (rand() & 255)/255.f*0.5 + 0.5;
    k *= ((rand() & 3)+4)/7.f;
    bool off = (rand() & 7)<3;
    for (int y = 0; y < GRASSH; y++) {
      float hhere = (float)(GRASSH-y)/h;
      unsigned int col = 0x010101*(int)(hhere>1?255:hhere*96*k+k2+128);
      grass_[x+y*GRASSW] = hhere < 1 ? col|0xff000000 : col;
      if (off) grass_[x+y*GRASSW] = col;
    }
  }}
  unsigned int gr2;
  glGenTextures(1,&gr2);
  glBindTexture(GL_TEXTURE_2D,gr2);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, GRASSW, GRASSH, 0, GL_RGBA, GL_BYTE, grass_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  Vector clearColor;

  RGBAImage pn = RGBAImage::fromFile("c:/GRAPHICS/PANORAMA/1/PANORA~4.PNG");
  {
    for (int y = 0; y < pn.height; y++) {
      double fogValue = (float)y / pn.height;
      fogValue *= 2;
      if (fogValue > 1) fogValue = 1;
      for (int x = 0; x < pn.width; x++) {
        unsigned int rgba = pn.data[x+y*pn.width];
        int r = rgba & 255;
        int g = (rgba>>8) & 255;
        int b = (rgba>>16) & 255;
        r = (fogColor[0]*255.0-r)*fogValue + r;
        g = (fogColor[1]*255.0-g)*fogValue + g;
        b = (fogColor[2]*255.0-b)*fogValue + b;
        if (x==0&&y==0) clearColor.set(r/255.f,g/255.f,b/255.f);
        pn.data[x+y*pn.width] = r | (g<<8) | (b<<16) | (rgba & 0xff000000);
      }
    }
  }
  unsigned int pnTex;
  glGenTextures(1,&pnTex);
  glBindTexture(GL_TEXTURE_2D,pnTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, pn.width, pn.height, 0, GL_RGBA, GL_BYTE, pn.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  RGBAImage pn2 = RGBAImage::fromFile("c:/GRAPHICS/PANORAMA/1/PANORA~4.PNG");
  {
    for (int y = 0; y < pn2.height/2; y++) {
      for (int x = 0; x < pn2.width; x++) {
        unsigned int rgba = pn2.data[x+(pn2.height/2-1-y)*pn2.width];
        pn2.data[x+(y+pn2.height/2)*pn2.width] =  rgba;
      }
    }
  }
  {
    float rt = 0.8;
    float gt = 0.9;
    float bt = 1.0;
    float k2 = 0.7;
    float rc = 0.7*255.0*k2;
    float gc = 0.8*255.0*k2;
    float bc = 0.9*255.0*k2;
    float k = 0.75;
    int k5 = 0;
    for (int y = 0; y < pn2.height; y++) {
      for (int x = 0; x < pn2.width; x++) {
        int ax = 0;
        int ay = 0;
        unsigned int rgba = pn2.data[((x+ax+pn2.width*10)%pn2.width)+((pn2.height-1-y+pn2.height*10+ay)%pn2.height)*pn2.width];
        int r = rgba & 255;
        int g = (rgba>>8) & 255;
        int b = (rgba>>16) & 255;
        r -= k5; if (r >= 0) r = (r * r)/(255-k5); r += k5;
        g -= k5; if (g >= 0) g = (g * g)/(255-k5); g += k5;
        b -= k5; if (b >= 0) b = (b * b)/(255-k5); b += k5;
        /*r *= rt;
        g *= gt;
        b *= bt;
        r = (r-rc)*k+rc;
        g = (g-gc)*k+gc;
        b = (b-bc)*k+bc;
        float k2 = 1.f + ((rand() & 255) / 255.f)*0.25f;
        r *= k2;
        g *= k2;
        b *= k2;
        if (r>255) r = 255;
        if (g>255) g = 255;
        if (b>255) b = 255;
        if (r<0) r = 0;
        if (g<0) g = 0;
        if (b<0) b = 0;*/
        pn2.data[x+y*pn2.width] =  r | (g<<8) | (b<<16) | (rgba & 0xff000000);
      }
    }
  }
  unsigned int pn2Tex;
  glGenTextures(1,&pn2Tex);
  glBindTexture(GL_TEXTURE_2D,pn2Tex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, pn2.width, pn2.height, 0, GL_RGBA, GL_BYTE, pn2.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


  RGBAImage pn3;
  pn3.width = 256;
  pn3.height = 256;
  pn3.data = new unsigned int[pn3.width*pn3.height];
  {
    for (int y = 0; y < pn3.height; y++) {
      for (int x = 0; x < pn3.width; x++) {
        int c = rand() & 255;
        //c = (c * (rand() & 255))/255.f;
        c = c * c * 1.5 / 255.f;
        if (c > 255) c = 255;
        unsigned int rgba = c | (c<<8) | (c<<16) | 0xff000000;
        pn3.data[x+y*pn3.width] =  rgba;
      }
    }
  }
  unsigned int pn3Tex;
  glGenTextures(1,&pn3Tex);
  glBindTexture(GL_TEXTURE_2D,pn3Tex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, pn3.width, pn3.height, 0, GL_RGBA, GL_BYTE, pn3.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#define FLWIDTH 16
#define FLHEIGHT 16
  unsigned int fl[FLWIDTH*FLHEIGHT];
  {
    for (int y = 0; y < FLHEIGHT; y++) {
      for (int x = 0; x < FLWIDTH; x++) {
        unsigned int c = 0x00;
        float rx = (float)x / (FLWIDTH-1) * 2.f - 1.f;
        float ry = (float)y / (FLHEIGHT-1) * 2.f - 1.f;
        double d = 1.f-sqrt(rx*rx+ry*ry);
        if (d > 0) {
          int o = d * 255;
          o += 128;
          if (o>255) o = 255;
          c = o * 0x010101;
          c |= 0xff000000;
        }    
        fl[x+y*FLWIDTH] = c;
      }
    }
  }

  unsigned int flTex;
  glGenTextures(1,&flTex);
  glBindTexture(GL_TEXTURE_2D,flTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, FLWIDTH, FLHEIGHT, 0, GL_RGBA, GL_BYTE, fl);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  {
    for (int y = 0; y < SUNHEIGHT; y++) {
      for (int x = 0; x < SUNWIDTH; x++) {
        unsigned int c = 0x00;
        float rx = (float)x / (SUNWIDTH-1) * 2.f - 1.f;
        float ry = (float)y / (SUNHEIGHT-1) * 2.f - 1.f;
        double d = 1.f-sqrt(rx*rx+ry*ry);
        if (d > 0) {
          int o = pow(d,0.75) * 127+128;
          c = o * 0x010101;
          c |= 0xff000000;
        }    
        sun[x+y*SUNWIDTH] = c;
      }
    }
  }

  unsigned int sunTex;
  glGenTextures(1,&sunTex);
  glBindTexture(GL_TEXTURE_2D,sunTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, SUNWIDTH, SUNHEIGHT, 0, GL_RGBA, GL_BYTE, sun);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


  RGBAImage gr = RGBAImage::fromFile("c:/GRAPHICS/GRASS/GRASS1.PNG");
  unsigned int grTex;
  glGenTextures(1,&grTex);
  glBindTexture(GL_TEXTURE_2D,grTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, gr.width, gr.height, 0, GL_RGBA, GL_BYTE, gr.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  RGBAImage bd2 = RGBAImage::fromFile("c:/GRAPHICS/GRASS/GRASS2.PNG");
  {for (int i = 0; i < bd2.width*bd2.height; i++) bd2.data[i]=(bd2.data[i]&0x00ffffff)|(((rand() % 192)+64) * 0x01000000);}
  unsigned int bd2Tex;
  glGenTextures(1,&bd2Tex);
  glBindTexture(GL_TEXTURE_2D,bd2Tex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, bd2.width, bd2.height, 0, GL_RGBA, GL_BYTE, bd2.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  RGBAImage rd = RGBAImage::fromFile("c:/GRAPHICS/TEXTURES/1/GRB4F4~1.PNG");
  RGBAImage rdn = RGBAImage::fromFile("c:/GRAPHICS/TEXTURES/1/GROUND~1.PNG");
  for (int j2 = 0; j2 < rd.width*rd.height; j2++) {
    unsigned int rgba = rd.data[j2];
    int r = rgba & 255;
    int g = (rgba>>8) & 255;
    int b = (rgba>>16) & 255;
    int grey = (r + g + b)/3;
    int t = 10;
    if (abs(r-grey)>t||abs(g-grey)>t||abs(b-grey)>t) rdn.data[j2] = 0xff808080;
  }
  applyNormalMap(&rd,&rdn,1.0,1.0,1.0,3.0,10.0,0.2);
  unsigned int rdTex;
  glGenTextures(1,&rdTex);
  glBindTexture(GL_TEXTURE_2D,rdTex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, rd.width, rd.height, 0, GL_RGBA, GL_BYTE, rd.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  RGBAImage r2 = RGBAImage::fromFile("c:/GRAPHICS/TEXTURES/1/GROUND~2.PNG");
  RGBAImage r2n = RGBAImage::fromFile("c:/GRAPHICS/TEXTURES/1/GROUND~3.PNG");
  applyNormalMap(&r2,&r2n,1.0,1.0,1.0,3.0,10.0,0.2);
  unsigned int r2Tex;
  glGenTextures(1,&r2Tex);
  glBindTexture(GL_TEXTURE_2D,r2Tex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, r2.width, r2.height, 0, GL_RGBA, GL_BYTE, r2.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  BitmapLayers *psd = new BitmapLayers();
  psd->loadPSD("c:/GRAPHICS/MAPS/MAP1.PSD");
  int psdw = psd->layers["elevation"].w;
  int psdh = psd->layers["elevation"].h;
  //glDone();printf("%d %d\n",psdw,psdh);exit(0);
  unsigned short *hMapdata = new unsigned short[psdw*psdh];
  unsigned short *heightMap = new unsigned short[psdw*psdh];

  unsigned int *e = psd->layers["elevation"].data;
  unsigned int *water = psd->layers["water"].data;
  unsigned int *grass = psd->layers["grass"].data;
  unsigned int *boden = psd->layers["boden"].data;
  unsigned int *trees = psd->layers["trees"].data;
  unsigned int *flowers = psd->layers["flowers"].data;
  unsigned int *stones = psd->layers["hecken"].data;
  unsigned int *roads = psd->layers["road"].data;
  unsigned int *cols = new unsigned int[psdw*psdh]; memset(cols,0,psdw*psdh*sizeof(unsigned int));
  unsigned int *grassTex = new unsigned int[psdw*psdh]; memset(grassTex,0,psdw*psdh*sizeof(unsigned int));

  unsigned char *roads2 = new unsigned char[psdw*psdh]; memset(roads2,0,psdw*psdh*sizeof(unsigned char));
  unsigned char *roads3 = new unsigned char[psdw*psdh]; memset(roads3,0,psdw*psdh*sizeof(unsigned char));
  unsigned char *roads4 = new unsigned char[psdw*psdh]; memset(roads4,0,psdw*psdh*sizeof(unsigned char));
  unsigned char *trees2 = new unsigned char[psdw*psdh]; memset(trees2,0,psdw*psdh*sizeof(unsigned char));
  unsigned char *grass2 = new unsigned char[psdw*psdh]; memset(grass2,0,psdw*psdh*sizeof(unsigned char));
  unsigned char *flowers2 = new unsigned char[psdw*psdh]; memset(flowers2,0,psdw*psdh*sizeof(unsigned char));
  unsigned char *stones2 = new unsigned char[psdw*psdh]; memset(stones2,0,psdw*psdh*sizeof(unsigned char));
  unsigned char *water2 = new unsigned char[psdw*psdh]; memset(water2,0,psdw*psdh*sizeof(unsigned char));
  unsigned char *boden2 = new unsigned char[psdw*psdh]; memset(boden2,0,psdw*psdh*sizeof(unsigned char));

  {for (int i = 0; i < psdw*psdh; i++) {
    int k = (roads[i]>>24) & 255;
    if (k < 0) k = 0;
    if (k > 255) k = 255;
    roads2[i] = k;
    water2[i] = (water[i]>>24) & 255;
    k -= 64;
    if (k < 0) k = 0;
    if (k > 255) k = 255;
    if ((stones[i]>>24) > 128) k = 1;
    if ((water[i]>>24) > 0) k = 1;
    roads3[i] = k;
    roads4[i] = k;
    trees2[i] = ((trees[i]>>24) & 255)>128 ? 1 : 0;
    boden2[i] = ((boden[i]>>24) & 255);
  }}

  for (int i = 0; i < psdw*psdh; i++) {
    int k = (e[i] & 255)*((e[i]>>24)&255)/255;
    hMapdata[i] = k << 8;
    hMapdata[i] = hMapdata[i]*40000/65535 + 10000;
    alpha(cols[i],grass[i]);
    alpha(cols[i],water[i]);
    //alpha(cols[i],flowers[i]);
    alpha(cols[i],trees[i],0.5);
    alpha(cols[i],stones[i]);
    alpha(cols[i],roads[i]);
    flowers2[i] = ((flowers[i]>>24) & 255) > 128 ? 1 : 0;
    stones2[i] = (stones[i]>>24) & 255;

    alpha(grassTex[i],grass[i]);
    remove(grassTex[i],roads[i],128);
    remove(water[i],roads[i],64);
    remove(water[i],stones[i],64);
    grass2[i] = 0; if ((grassTex[i]&0xffffff)==(grass[i]&0xffffff)) grass2[i] = 1;
  }

  {for (int i = 0; i < psdw*psdh; i++) {
    hMapdata[i]+=rand() & 1023;
  }}
     
  for (int y = 0; y < psdh; y++) {
    for (int x = 0; x < psdw; x++) {
      const int bx = 2; const int by = bx;
      float v = 0; float w = 0;
      for (int ky = -by+y; ky <= by+y; ky++) {
        for (int kx = -bx+x; kx <= bx+x; kx++) {
          if ((unsigned int)kx<psdw&&(unsigned int)ky<psdh) {
            v += hMapdata[kx+ky*psdw];
            w += 1.f;
          }
        }
      }
      if (w != 0) v /= w;
      heightMap[x+y*psdw] = v;
    }
  }

  scape = new Landscape(-250.0,-250.0,250.0,250.0,0.0,100.0);
  raw = new LandscapeRaw(scape);
  scape->setHeightMap(roads3,heightMap, psdw, psdh, 1, 1, 10.0, 10.0, boden2);
  int w = psdw; int h = psdh;
  //downsample(&roads2,&w,&h,3);
  scape->setRoads(roads2,w, h,128,128+16,64);
  scape->setTrees(roads2,trees2,psdw,psdh,128);
  scape->setGrass(roads4,grass2,psdw,psdh,32);
  scape->setFlowers(roads3,flowers2,psdw,psdh,5);
  scape->setStones(stones2,psdw,psdh,128,110);
  w = psdw; h = psdh; downsample(&water2,&w,&h,8); scape->setWater(water2,w,h,128,100);
  edit = new LandscapeEdit(scape, raw, &cameraPos, &details);
  edit->setObjectsFile(OBJECTSPNG,1024,1024);
  edit->refreshObjects();
   
  collision = new LandscapeCollision(-250.0,-250.0,250.0,250.0,psdw,psdh);
  collision->placeMask(water2,w,h,1.0,0.1);
  collision->placeMask(stones2,psdw,psdh,1.0,0.1);
  {
    for(int i = 0; i < scape->scape.size(); i++) {
      LandscapeElement *e = &scape->scape[i];
      switch(e->type) {
      case LANDSCAPE_TYPE_TREE: {
        collision->placeCircle(e->x,e->z,e->v2>=128?2:1);
      } break;
      case LANDSCAPE_TYPE_OBJECT: {
        double siz = 1;
        switch(e->v0-1) {
        case 4: {siz = 4;} break;
        }
        collision->placeCircle(e->x,e->z,siz);
      } break;
      }
    }
  }
  collision->boxBlur(2);

  unsigned int tex;
  glGenTextures(1,&tex);
  glBindTexture(GL_TEXTURE_2D,tex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, psdw, psdh, 0, GL_RGBA, GL_BYTE, cols);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_EXP2);
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogf(GL_FOG_DENSITY, fogDensity);


  raw->update(cameraPos, details);

  while(1) {
    glDebug(0);
    double timeAllStart = glSeconds();
    static double lastSeconds = glSeconds();
    double seconds = glSeconds();
    double td = seconds - lastSeconds;
    lastSeconds = seconds;

    static double fpsTime = 0;
    static int fpsCounter = 0;
    fpsTime+=td;
    fpsCounter++;
    static double currentFPS=0;
    if (fpsTime>2.0) {
      fpsTime -= 2.0;
      currentFPS = fpsCounter/2.0;
      fpsCounter = 0;
    }

    const int currentKey = glNextKey();
    if (currentKey == GL_VK_END) break;
    double mdx, mdy;
    glNextMouseDelta(&mdx, &mdy);
    int a = glMouseButtons();
    mouseX += mdx;
    mouseY += mdy;

    static Vector lastCameraPos = cameraPos;
    static Vector cameraPosDest = characterPos;
    static Vector cameraTarget = characterPos;
    static float characterRotationY = 0;
    static float cameraDestRotY = characterRotationY;
    static float cameraRotY = cameraDestRotY;
    const float characterRotYSpeed = 2.0;
    const float cameraRotYSpeed = 0.25;
    const float cameraAddRotSpeedY = 4.0;
    const float cameraMoveSpeed = 1.0;
    const float cameraAddMoveSpeed = 4.0;
    const float cameraQuantRotY = 1;
    const float cameraHeight = 2.f;
    const float cameraDistance = 4.f;
    Vector characterDest(0,0,0,0);
    if (keyPressed[SCANCODE_UP]) characterDest += characterPos + Vector(sin(characterRotationY),0,cos(characterRotationY))*td*10.0;
    if (keyPressed[SCANCODE_DOWN]) characterDest += characterPos - Vector(sin(characterRotationY),0,cos(characterRotationY))*td*10.0;
    if (keyPressed[SCANCODE_LEFT]) characterRotationY += td*characterRotYSpeed;
    if (keyPressed[SCANCODE_RIGHT]) characterRotationY -= td*characterRotYSpeed;
    if (length(characterDest) != 0) characterPos = tryWalk(characterPos, characterDest);

    static float walkAnimSeconds = 0;
    const float walkAnimSpeed = 1.25f;
    if (keyPressed[SCANCODE_UP]) { walkAnimSeconds += td*walkAnimSpeed; 
    } else {
      if (keyPressed[SCANCODE_DOWN]) {
        walkAnimSeconds -= td*walkAnimSpeed; 
      }
      else {
        walkAnimSeconds = 0;
      }
    }

#ifdef EDITLANDSCAPE
    if (currentKey >= 'a' && currentKey <= 'z') edit->placeObject(&characterPos,currentKey - 'a' + 1);
    if (currentKey >= '1' && currentKey <= '8') edit->rotateObjectY(&characterPos,currentKey - '1');
    if (currentKey == ' ') edit->removeObjects(&characterPos);
#endif

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0,16.0/9.0,0.1,2000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    cameraTarget = characterPos;
    cameraTarget.y = scape->getHeight(cameraTarget.x,cameraTarget.z) + cameraHeight;
    cameraDestRotY = characterRotationY;
    const float cameraAddRotSpeedY2 = fabs(cameraRotY-cameraDestRotY)*cameraAddRotSpeedY+1.0;
    if (cameraRotY<cameraDestRotY) cameraRotY+= cameraRotYSpeed*td*cameraAddRotSpeedY2;
    if (cameraRotY>cameraDestRotY) cameraRotY-= cameraRotYSpeed*td*cameraAddRotSpeedY2;
    cameraPosDest = cameraTarget - Vector(sin(cameraRotY),0,cos(cameraRotY))*cameraDistance;
    cameraPosDest.y = scape->getHeight(cameraPos.x,cameraPos.z) + cameraHeight;
    const Vector distv = cameraPosDest-cameraPos;
    const float distf = length(distv);
    const Vector distn = normalize(distv);
    float d = td * (cameraMoveSpeed+cameraAddMoveSpeed*distf);
    if (d > distf) d = distf;
    cameraPos += distn * d;
    gluLookAt(cameraPos.x,cameraPos.y,cameraPos.z, cameraTarget.x,cameraTarget.y,cameraTarget.z, 0,1,0);

    Matrix mv_,p_,mvp_;
    glGetDoublev(GL_MODELVIEW_MATRIX,mv_.m);
    glGetDoublev(GL_PROJECTION_MATRIX,p_.m);
    mvp_ = p_ * mv_;

    float dist = 1.f;float pos[4] = {1*dist,1*dist,1*dist,0}; // directional
    glLightfv(GL_LIGHT0, GL_POSITION,pos);
    float specular[4] = {1,1,1,1}; glLightfv(GL_LIGHT0, GL_SPECULAR,specular);
    float diffuse[4] = {1,1,1,1}; glLightfv(GL_LIGHT0, GL_DIFFUSE,diffuse);
    float am[4]={1.5,1.5,1.5,0}; glLightModelfv(GL_LIGHT_MODEL_AMBIENT,am);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specular);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,20.0);

    if (length(cameraPos-lastCameraPos)>8.0) {
      lastCameraPos = cameraPos;
      Matrix m;
      glGetDoublev(GL_MODELVIEW_MATRIX,m.m);
      m = transpose(inverse(m));
      raw->update(cameraPos,details);
    }

    glDebug(1);

    double timeTreePaintStart = glSeconds(); 
    for (int p = 0; p < 1; p++) {
      startSpriteObjectPainting(&tree[p], &tre[p]->minBounding, &tre[p]->maxBounding);
      glDisable(GL_TEXTURE_2D);
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      float am[4]={0.25,0.25,0.25,0}; glLightModelfv(GL_LIGHT_MODEL_AMBIENT,am);
      float sp[4]={0,0,0,0};glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,sp);
      glEnable(GL_ALPHA_TEST);
      //glAlphaFunc(GL_GREATER,0.3);
      glAlphaFunc(GL_GREATER,3.f/256.f);
      glDisable(GL_FOG);
      paintMesh(tre[p]);
      glEnable(GL_FOG);
      glDisable(GL_ALPHA_TEST);
      glDisable(GL_COLOR_MATERIAL);
      glDisable(GL_LIGHT0);
      glDisable(GL_LIGHTING);
      glEnable(GL_TEXTURE_2D);
      finishSpriteObjectPainting();
    }
    double timeTreePaintEnd = glSeconds(); 

    glDebug(2);

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,specular);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,20.0);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,am);

    glClearColor(clearColor.x,clearColor.y,clearColor.z,1);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  
    glBindTexture(GL_TEXTURE_2D,grTex);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
    glColor4f(1,1,1,1);

    int k = (int)(glSeconds()*10.0);
    srand(k & 32767);
    float xa = (float)(rand() & 255)/255.f;
    float za = (float)(rand() & 255)/255.f;

    srand(0);
    double timeLandscapePaintStart = glSeconds();
    for (int i = 0; i < raw->triangles.size(); i++) {

      //glColor3f((rand() & 255)/255.f,(rand() & 255)/255.f,(rand() & 255)/255.f);

      const int t0 = raw->types[raw->triangles[i].p[0]];
      const int t1 = raw->types[raw->triangles[i].p[1]];
      const int t2 = raw->types[raw->triangles[i].p[2]];
      const Vector &p0 = raw->vertices[raw->triangles[i].p[0]];
      const Vector &p1 = raw->vertices[raw->triangles[i].p[1]];
      const Vector &p2 = raw->vertices[raw->triangles[i].p[2]];
      const Vector &n0 = raw->parameters[raw->triangles[i].p[0]];
      const Vector &n1 = raw->parameters[raw->triangles[i].p[1]];
      const Vector &n2 = raw->parameters[raw->triangles[i].p[2]];
      float boden[3] = {0,0,0};
      if (t0 == LANDSCAPE_TYPE_HEIGHT) boden[0] = n0.y;
      if (t1 == LANDSCAPE_TYPE_HEIGHT) boden[1] = n1.y;
      if (t2 == LANDSCAPE_TYPE_HEIGHT) boden[2] = n2.y;
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D,grTex);
      float texScale = 1;
      bool stones = false;
      float stonesShade = 1;
      if (t0 == LANDSCAPE_TYPE_STONE||t1 == LANDSCAPE_TYPE_STONE||t2 == LANDSCAPE_TYPE_STONE) {
        glBindTexture(GL_TEXTURE_2D,r2Tex); 
        texScale = 10; 
        stones = true;
        stonesShade = dot(normalize(cross(p1-p0,p2-p0)),normalize(Vector(1,1,1)))*0.5+0.5;
        if (stonesShade < 0) stonesShade = 0;
      }

      bool water = false;
      glEnable(GL_FOG);
      if ((t0 == t1) && (t1 == t2)) {
       if (t0 == LANDSCAPE_TYPE_ROAD) {glBindTexture(GL_TEXTURE_2D,rdTex); texScale = 3;}
       if (t0 == LANDSCAPE_TYPE_WATER) {glBindTexture(GL_TEXTURE_2D,pn2Tex);water = true; texScale = 0.025;glDisable(GL_FOG);}
      }

      int t_[3]; t_[0] = t0; t_[1] = t1; t_[2] = t2;
  
      bool landscapeRoadWithBorder = false; 
      for (int j = 0; j < 3; j++) {
        Vector &p = raw->vertices[raw->triangles[i].p[j]];
        Vector &n = raw->parameters[raw->triangles[i].p[j]];
        Vector tn = getTexCoord(p);
        unsigned char *col = (unsigned char *)&cols[((int)(tn.x*psdw)&(psdw-1))+((int)(tn.y*psdh)&(psdh-1))*psdw];
        float ck = n.x*0.25+0.75;
        glColor4ub(col[0]*ck,col[1]*ck,col[2]*ck,col[3]);
        glTexCoord2f(tn.x*texScale*100.0,tn.y*texScale*100.0);

        if ((t0 == t1) && (t1 == t2)) {
          if (t0 == LANDSCAPE_TYPE_ROAD) {
            float c = 1.0-raw->parameters[raw->triangles[i].p[j]].x;
            bool isBorder = c > 0.95;
            c = c * 0.5 + 0.5;
            c *= 1.5;
            glColor3f(c,c*0.6,c*0.2);
            /*if (isBorder) {
              float k = sin(p.x+p.z)*0.2;
              glColor3f(0.25+k,0.5+k,0);
            }*/
            glTexCoord2f(p.x*texScale*0.1,p.z*texScale*0.1);
            if (isBorder) landscapeRoadWithBorder = true;
          }
        }
        if (stones) {
          float hd = stonesShade * n.x;
          if (t_[j] == LANDSCAPE_TYPE_STONE) {
            glColor3f(hd,hd,hd);
          } else {
            unsigned char *b = (unsigned char *)&cols[((int)(tn.x*psdw)&(psdw-1))+((int)(tn.y*psdh)&(psdh-1))*psdw];
            glColor4ub(b[0]*0.2,b[1]*0.5,b[2]*0.3,b[4]);
          }
          glTexCoord2f(p.x*texScale*0.1,p.z*texScale*0.1);
        }
        if (water) {
          Vector pw = mv_ * p;
          if (pw.w != 0) pw /= pw.w;
          float ix = pw.x;
          float iz = pw.z;
          glColor3f(1,1,1);
          glTexCoord2f((ix)*texScale,(iz)*texScale);
        }
        glVertex3dv(&p.x);
      }
      if (landscapeRoadWithBorder) {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER,0.25);
        glBindTexture(GL_TEXTURE_2D,bd2Tex);        
        const float texScale = 0.1*0.5;
        for (int j = 0; j < 3; j++) {
          const Vector &p = raw->vertices[raw->triangles[i].p[j]];
          const float c = 1.0-raw->parameters[raw->triangles[i].p[j]].x;
          float k = sin(p.x+p.z)*0.05+0.15;
          const float k2 = 0.9;
          glColor4f((0.2+k)*k2,(0.5+k)*k2,0.0,c);
          glTexCoord2f(p.x*texScale,p.z*texScale);
          glVertex3dv(&p.x);
        }
        glDisable(GL_ALPHA_TEST);
      }

      if (boden[0]!=0||boden[1]!=0||boden[2]!=0) {
        //float s = dot(normalize(cross(p1-p0,p2-p0)),normalize(Vector(1,1,1)))*0.5+0.5;
        //if (s < 0) s = 0;
        const Vector *n_[3] = {&n0,&n1,&n2};
        glBindTexture(GL_TEXTURE_2D,bd2Tex);        
        const float texScale = 0.1;
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER,0.25);
        for (int j = 0; j < 3; j++) {
          float ck = boden[j]!=0?n_[j]->x*0.25+0.75:0.5;
          Vector &p = raw->vertices[raw->triangles[i].p[j]];
          glTexCoord2f(p.x*texScale,p.z*texScale);
          glColor4f(1*ck,0.8*ck,0.5*ck,boden[j]);
          glVertex3dv(&p.x);
        }
        glDisable(GL_ALPHA_TEST);
      }

      if (water) {
        float flowSpeed = 0.5;
        float waveScale = 0.05*0.5;
        float wavePos = glSeconds() * waveScale * flowSpeed;
        glBindTexture(GL_TEXTURE_2D,pn3Tex);        
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
        {
          glColor3f(1,1,1);
          for (int j = 0; j < 3; j++) {
            Vector &p = raw->vertices[raw->triangles[i].p[j]];
            float ix = p.x*waveScale+wavePos;
            float iz = p.z*waveScale;
            glTexCoord2f(ix,iz);
            glVertex3dv(&p.x);
          }
        }
        {
          for (int j = 0; j < 3; j++) {
            Vector &p = raw->vertices[raw->triangles[i].p[j]];
            float ix = p.x*waveScale;
            float iz = p.z*waveScale+wavePos;
            glTexCoord2f(ix,iz);
            glVertex3dv(&p.x);
          }
        }
        waveScale *= 0.125;
        wavePos = glSeconds() * waveScale * 1.5 * flowSpeed;
        {
          glColor3f(1,1,1);
          for (int j = 0; j < 3; j++) {
            Vector &p = raw->vertices[raw->triangles[i].p[j]];
            float ix = p.x*waveScale+wavePos;
            float iz = p.z*waveScale;
            glTexCoord2f(ix,iz);
            glVertex3dv(&p.x);
          }
        }
        {
          float a = 0.025*0.75;
          float b = 0;
          for (int j = 0; j < 3; j++) {
            Vector &p = raw->vertices[raw->triangles[i].p[j]];
            float ix = p.x*waveScale;
            float iz = p.z*waveScale+wavePos;
            Vector q = mv_ * p; if (q.w != 0) q /= q.w; float k = -q.z * a + b;  if (k < 0) k = 0; if (k > 1) k = 1;
            glColor3f(k,k,k);
            glTexCoord2f(ix,iz);
            glVertex3dv(&p.x);
          }
        }
        glBlendFunc(GL_ONE, GL_ONE);
        glDisable(GL_TEXTURE_2D);
        {
          float a = 0.025;
          float b = 0.1;
          for (int j = 0; j < 3; j++) {            
            Vector &p = raw->vertices[raw->triangles[i].p[j]];
            Vector q = mv_ * p; if (q.w != 0) q /= q.w; float k = -q.z * a + b;  if (k < 0) k = 0; if (k > 1) k = 1;
            glColor3f(0.25*k,0.5*k,1.0*k);
            glVertex3dv(&p.x);
          }
        }
        glDisable(GL_BLEND);
      }
    }
    glEnd();
    glEnable(GL_FOG);
    double timeLandscapePaintEnd = glSeconds();

    glDebug(3);

    const float characterScale = 1.75f;

    double timeSpritePaintStart = glSeconds();
    for (int i2 = 0; i2 < raw->elements.size(); i2++) {
      const LandscapeElement *e = raw->elements[i2];
      switch(e->type) {
      case LANDSCAPE_TYPE_GRASS: {
        float k = e->v2 / 255.f;
        k += seconds;
        k *= PI * 2.f * 0.5f;
        Vector v = Vector(e->x,e->y,e->z);
        blitGrassBlade(gr2, GRASSW*2*(e->v2/255.f*0.75f+0.125f), GRASSH, &v, (e->v1/255.f*0.7+0.3)*0.05, (0x000402*(e->v0))|0xff000000,e->v2/255.f,k);
      } break;
      case LANDSCAPE_TYPE_FLOWER: {
        if (e->v0 == 0) glColor4f(1,1,1,1);
        if (e->v0 == 1) glColor4f(0,1,1,1);
        if (e->v0 == 2) glColor4f(1,1,0,1);
        if (e->v0 == 3) glColor4f(1,0,0,1);
        float k = e->v1 / 255.f;
        k += seconds;
        k *= PI * 2.f * 0.5f;
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,flTex);
        Vector v = Vector(e->x+cos(k)*0.05,e->y+sin(k)*0.05,e->z+sin(k*1.5)*0.05);
        blitFlower(&v,(e->v2/255.f*0.5+0.125)*2);
      } break;
      case LANDSCAPE_TYPE_TREE: {
        const float treeScale = 0.1*1.5*(5.0/5.0);
        Vector v = Vector(e->x,e->y,e->z);
        blitSpriteObjectSprite_(&tree[0], &v, (e->v1/255.f*0.4+0.6)*0.5*4*(1+(e->v2 & 1)*0.25)*treeScale*(e->v2/128*4.0+1), (0x010101*(e->v0/2+128))|0xff000000);
      } break;
      case LANDSCAPE_TYPE_OBJECT: {
        Vector zwo = mv_ * Vector(e->x,e->y,e->z);
        if (zwo.w != 0) zwo /= zwo.w;
        if (zwo.z < 0+1) {
          glPushMatrix();
          glTranslatef(e->x,e->y,e->z);
          glRotatef(e->v1*360.0/8.0,0,1,0);
          const float s = characterScale;
          glScalef(s,s,s);
          glDisable(GL_TEXTURE_2D);
          int meshNr = e->v0-1;
          switch(meshNr) {
          case 0:
          case 1:
          case 2: {
            paintMesh(objs[meshNr]);
          } break;
          case 3: {
            paintPortal();
          } break;
          case 4: {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
            float am[4]={0.25,0.25,0.25,0}; glLightModelfv(GL_LIGHT_MODEL_AMBIENT,am);
            float sp[4]={1,1,1,0};glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,sp);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,dragonTexture);
            glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
            paintMesh(objs[meshNr]);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
          } break;
          case 5: {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
            float am[4]={0.75,0.75,0.75,0}; glLightModelfv(GL_LIGHT_MODEL_AMBIENT,am);
            float sp[4]={1,1,1,0};glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,sp);
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,dragonTexture);
            glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
            const float s2 = 0.5+e->v1/8.0;
            glScalef(s2,s2,s2);
            glRotatef(10,1,0,0);
            paintMesh(objs[meshNr]);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
          } break;
          }
          glPopMatrix();
        }
        //drawLandscapeText(e->x,e->z,"Hallo");
      } break;
      }
    }

    glDebug(4);

    drawBirds(cameraPos, glSeconds(), bird);
    double timeSpritePaintEnd = glSeconds();

    glDebug(5);
  
    double timeApplyAnimationStart = glSeconds();
    animateGLTF_Character(girl,(sfmod(walkAnimSeconds*24.f,61.f)+2.f));
    double timeApplyAnimationEnd = glSeconds();

    double timePaintCharacterStart = glSeconds();
    glPushMatrix();
    Vector playerPos;
    playerPos = characterPos;
    playerPos.y = scape->getHeight(playerPos.x,playerPos.z)+1.3+0.25;
    const float SEEMPREVENT = 0.001;
    glTranslatef(playerPos.x+SEEMPREVENT,playerPos.y+SEEMPREVENT,playerPos.z+SEEMPREVENT);
    glRotatef(characterRotationY*360.f/PI/2.f,0,1,0);
    glScalef(characterScale,characterScale,characterScale);
    renderGLTF_Character(girl);
    glPopMatrix();
    double timePaintCharacterEnd = glSeconds();

    glDebug(6);
 
    double timeHimmelStart = glSeconds();
    const bool himmel = true;
    if (himmel) {
      glDisable(GL_FOG);
      const int skyDomeSegments = 16;
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D,pnTex);
      glBegin(GL_QUADS);
      glColor4f(1,1,1,1);
      for (int i3 = 0; i3 < skyDomeSegments; i3++) {
        const float ri0 = (float)i3 / skyDomeSegments;
        const float ri1 = (float)(i3+1) / skyDomeSegments;
        const float h = 1250.f;
        const float r = 1000.f;
        const float xp = cameraPos.x;
        const float yp = cameraPos.y;
        const float zp = cameraPos.z;
        const float y0 = h*0.5f+yp;
        const float y1 = -h*0.5f+yp;
        const float x0 = sin(ri0 * PI * 2.f) * r+xp;
        const float x1 = sin(ri1 * PI * 2.f) * r+xp;
        const float z0 = -cos(ri0 * PI * 2.f) * r+zp;
        const float z1 = -cos(ri1 * PI * 2.f) * r+zp;
        const float tx0 = ri0;
        const float tx1 = ri1;
        const float ty0 = 0;
        const float ty1 = 1;
        glTexCoord2f(tx1,ty0); glVertex3f(x1,y0,z1);
        glTexCoord2f(tx0,ty0); glVertex3f(x0,y0,z0);
        glTexCoord2f(tx0,ty1); glVertex3f(x0,y1,z0);
        glTexCoord2f(tx1,ty1); glVertex3f(x1,y1,z1);
      }
      glEnd();
      glEnable(GL_FOG);
  
      glBindTexture(GL_TEXTURE_2D,sunTex);
      glDisable(GL_FOG);
      Vector v = Vector(250+cameraPos.x,150+cameraPos.y,250+cameraPos.z);
      blitSun(&v);
    }
    double timeHimmelEnd = glSeconds();

    glDebug(7);

    double timeAllEnd = glSeconds();

    glRefresh();
#ifdef LOGFPS
    setBiosCursor(0,0);
    printf("all:%f\n",timeAllEnd-timeAllStart);
    printf("tree:%f\n",timeTreePaintEnd-timeTreePaintStart);
    printf("landscape:%f\n",timeLandscapePaintEnd-timeLandscapePaintStart);
    printf("sprites:%f\n",timeSpritePaintEnd-timeSpritePaintStart);
    printf("character:%f\n",timePaintCharacterEnd-timePaintCharacterStart);
    //printf("himmel:%f\n",timeHimmelEnd-timeHimmelStart);
    //printf("applyAnimation:%f\n",timeApplyAnimationEnd-timeApplyAnimationStart);
#endif

  }

  uninstallKeyboardHandler();

  glDone();
  return 0;
}
