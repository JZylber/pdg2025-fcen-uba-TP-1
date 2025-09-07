//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2025-08-04 22:12:21 gtaubin>
//------------------------------------------------------------------------
//
// LoaderStl.cpp
//
// Written by: <Your Name>
//
// Software developed for the course
// Digital Geometry Processing
// Copyright (c) 2025, Gabriel Taubin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Brown University nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>
#include "TokenizerFile.hpp"
#include "LoaderStl.hpp"
#include "StrException.hpp"

#include "wrl/Shape.hpp"
#include "wrl/Appearance.hpp"
#include "wrl/Material.hpp"
#include "wrl/IndexedFaceSet.hpp"

// reference
// https://en.wikipedia.org/wiki/STL_(file_format)

const char *LoaderStl::_ext = "stl";

bool LoaderStl::loadIndexedFaceSet(Tokenizer &tkn, IndexedFaceSet &ifs)
{
  bool success = false;
  // get references to the coordIndex, coord, and normal arrays
  vector<int> &coordIndex = ifs.getCoordIndex();
  vector<float> &coord = ifs.getCoord();
  vector<float> &normal = ifs.getNormal();
  // First token should be "facet"
  if (tkn.equals("facet"))
  {
    // Second token should be "normal"
    if (tkn.expecting("normal"))
    {
      // Third, fourth, and fifth tokens should be the components of the face normal
      Vec3f *faceNormal = new Vec3f();
      bool gotNormal = tkn.getVec3f(*faceNormal);
      if (gotNormal)
      {
        normal.push_back(faceNormal->x);
        normal.push_back(faceNormal->y);
        normal.push_back(faceNormal->z);
      }
      else
      {
        throw new StrException("expecting 3d vector for normal, found " + tkn);
      }

      if (tkn.expecting("outer") && tkn.expecting("loop"))
      {
        while (tkn.get() && !tkn.equals("endloop"))
        {
          if (tkn.equals("vertex"))
          {
            Vec3f *vertex = new Vec3f();
            bool gotVector = tkn.getVec3f(*vertex);
            if (gotVector)
            {
              coord.push_back(vertex->x);
              coord.push_back(vertex->y);
              coord.push_back(vertex->z);
              coordIndex.push_back((int)(coord.size() / 3 - 1));
            }
            else
            {
              throw new StrException("expecting 3d vector for vertex, found " + tkn);
            }
          }
          else
            throw new StrException("expecting vertex, found " + tkn);
        }
        coordIndex.push_back(-1); // end of face
      }
      else
        throw new StrException("expecting outer loop, found " + tkn);
    }
    else
      throw new StrException("expecting normal, found " + tkn);
  }
  else
    throw new StrException("expecting facet, found " + tkn);
  // Last token should be "endfacet"
  if (tkn.expecting("endfacet"))
    success = true;
  else
  {
    throw new StrException("expecting endfacet, found " + tkn);
  }
  return success;
}

bool LoaderStl::load(const char *filename, SceneGraph &wrl)
{
  bool success = false;

  // clear the scene graph
  wrl.clear();
  wrl.setUrl("");

  FILE *fp = (FILE *)0;
  try
  {

    // open the file
    if (filename == (char *)0)
      throw new StrException("filename==null");
    fp = fopen(filename, "r");
    if (fp == (FILE *)0)
      throw new StrException("fp==(FILE*)0");

    // use the io/Tokenizer class to parse the input ascii file

    TokenizerFile tkn(fp);
    // first token should be "solid"
    if (tkn.expecting("solid"))
    {
      tkn.get();
      string stlName = tkn; // second token should be the solid name
      // create the scene graph structure :
      // 1) the SceneGraph should have a single Shape node a child
      Shape *shape = new Shape();
      shape->setName(stlName);
      wrl.addChild(shape);
      // 2) the Shape node should have an Appearance node in its appearance field
      Appearance *appearance = new Appearance();
      shape->setAppearance(appearance);
      // 3) the Appearance node should have a Material node in its material field
      Material *material = new Material();
      appearance->setMaterial(material);
      // 4) the Shape node should have an IndexedFaceSet node in its geometry node
      IndexedFaceSet *ifs = new IndexedFaceSet();
      shape->setGeometry(ifs);

      // from the IndexedFaceSet
      // 5) get references to the coordIndex, coord, and normal arrays (on private method)
      // 6) set the normalPerVertex variable to false (i.e., normals per face)
      ifs->setNormalPerVertex(false);

      // the file should contain a list of triangles in the following format

      // facet normal ni nj nk
      //   outer loop
      //     vertex v1x v1y v1z
      //     vertex v2x v2y v2z
      //     vertex v3x v3y v3z
      //   endloop
      // endfacet

      // - run an infinite loop to parse all the faces
      while (tkn.get() && !tkn.equals("endsolid"))
      {
        bool faceParsed = loadIndexedFaceSet(tkn, *ifs);
        // - write a private method to parse each face within the loop
        // - the method should return true if successful, and false if not
        // - if your method returns tru
        //     update the normal, coord, and coordIndex variables
        // - if your method returns false
        //     throw an StrException explaining why the method failed
        if (!faceParsed)
        {
          throw new StrException("error when loading face");
        }
      }
    }
    // close the file (this statement may not be reached)
    fclose(fp);
    success = true;
  }
  catch (StrException *e)
  {

    if (fp != (FILE *)0)
      fclose(fp);
    fprintf(stderr, "ERROR | %s\n", e->what());
    delete e;
  }

  return success;
}
