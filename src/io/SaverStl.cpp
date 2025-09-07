//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2025-08-04 22:14:44 gtaubin>
//------------------------------------------------------------------------
//
// SaverStl.cpp
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

#include "SaverStl.hpp"

#include "wrl/Shape.hpp"
#include "wrl/Appearance.hpp"
#include "wrl/Material.hpp"
#include "wrl/IndexedFaceSet.hpp"

#include "core/Faces.hpp"

const char *SaverStl::_ext = "stl";

//////////////////////////////////////////////////////////////////////
bool SaverStl::save(const char *filename, SceneGraph &wrl) const
{
  bool success = false;
  if (filename != (char *)0)
  {

    // Check these conditions

    // 1) the SceneGraph should have a single child
    // 2) the child should be a Shape node
    // 3) the geometry of the Shape node should be an IndexedFaceSet node
    if (wrl.getNumberOfChildren() != 1)
      return false;
    Node *node = wrl[0];
    if (!node->isShape())
      return false;
    Shape *shape = (Shape *)node;
    node = shape->getGeometry();
    if (!node->isIndexedFaceSet())
      return false;
    IndexedFaceSet *ifs = (IndexedFaceSet *)node;
    if (!ifs->isTriangleMesh())
      return false;
    if (!(ifs->getNumberOfNormal() == ifs->getNumberOfFaces()))
      return false;

    // - construct an instance of the Faces class from the IndexedFaceSet
    // - remember to delete it when you are done with it (if necessary)
    //   before returning

    // 4) the IndexedFaceSet should be a triangle mesh
    // 5) the IndexedFaceSet should have normals per face PENDING

    // if (all the conditions are satisfied) {
    Faces *faces = new Faces(ifs->getNumberOfCoord(), ifs->getCoordIndex());
    vector<float> &coord = ifs->getCoord();
    vector<float> &normal = ifs->getNormal();
    FILE *fp = fopen(filename, "w");
    if (fp != (FILE *)0)
    {

      // if set, use ifs->getName()
      // otherwise use filename,
      // but first remove directory and extension
      std::string name = ifs->getName();
      if (name.empty())
      {
        std::string cleanFilename(filename);
        size_t pos = cleanFilename.find_last_of("/\\");
        size_t dot = cleanFilename.find_last_of(".");
        name = cleanFilename.substr(pos + 1, dot - pos - 1);
      }
      fprintf(fp, "solid %s\n", name.c_str());
      for (size_t nFace = 0; nFace < faces->getNumberOfFaces(); nFace++)
      {
        vector<float> faceNormal = {normal[nFace * 3], normal[nFace * 3 + 1], normal[nFace * 3 + 2]};
        fprintf(fp, " facet normal %f %f %f\n", faceNormal[0], faceNormal[1], faceNormal[2]);
        fprintf(fp, "  outer loop\n");
        // 3 vertexes since its a triangle mesh
        for (size_t nCorner = 0; nCorner < 3; nCorner++)
        {
          int iV = faces->getFaceVertex(nFace, nCorner);
          vector<float> vertex = {coord[iV * 3], coord[iV * 3 + 1], coord[iV * 3 + 2]};
          fprintf(fp, "   vertex %f %f %f\n", vertex[0], vertex[1], vertex[2]);
        }
        fprintf(fp, "  endloop\n");
        fprintf(fp, " endfacet\n");
      }
      fprintf(fp, "endsolid %s\n", name.c_str());
      fclose(fp);
      success = true;
    }
    // } endif (all the conditions are satisfied)
  }
  return success;
}
