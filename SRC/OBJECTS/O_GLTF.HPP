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
#ifndef __O_GLTF_HPP__
#define __O_GLTF_HPP__

/**
* This function loads a gltf from a *.glb file to be used as a game character.
*
* @param name The file name of the *.glb to load.
* @example GLTFA_File *gltf = loadGLTF_Character("girl1.glb");
*/
class GLTFA_File *loadGLTF_Character(const char *name);

/**
* This function animates a gltf which is used as a game character. Including bone setups.
*
* @param gltf the gltf file which should be animated.
* @param frame The frame number of the animation.
* @example animateGLTF_Character(gltf, 8.0);
*/
void animateGLTF_Character(class GLTFA_File *gltf, double frame);

/**
* This function renders a gltf which is used as a game character. Including bone setups.
*
* @param gltf the gltf file which should be rendered.
* @example renderGLTF_Character(gltf);
*/
void renderGLTF_Character(class GLTFA_File *gltf);

/**
* This function logs all nodes and some more data via printf (it closes OpenGL first) and exits the program.
*
* @param gltf A GLTF object.
* @example logGLTFNodes(girl);
*/
void logGLTFNodes(class GLTFA_File *gltf);

/**
* This function shows the triangle count of the entire gltf via printf. It closes OpenGL before and exits the program.
*
* @param gltf A GLTF object.
* @example logGLTFFaceCount(girl);
*/
void logGLTFFaceCount(class GLTFA_File *gltf);

#endif //__O_GLTF_HPP__
