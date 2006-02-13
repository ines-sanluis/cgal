// ======================================================================
//
// Copyright (c) 2005-2006 by Le-Jeng Shiue.  All Rights Reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source$
// $Revision$ $Date$
// $Name$
//
// Author(s): Le-Jeng Shiue <Andy.Shiue@gmail.com>
//                          <1998-2005 SurfLab, CISE, University of Florida>
//                          <2005-     RapidMind Inc.>
//
// ======================================================================

#ifndef _POLYHEDRON_SUBDIVISION_H_01292002
#define _POLYHEDRON_SUBDIVISION_H_01292002

#include <CGAL/basic.h>

#include <vector>

#include <CGAL/circulator.h>

#include <CGAL/Subdivision_surfaces_impl_3.h>
#include <CGAL/Subdivision_surfaces_masks_3.h>

CGAL_BEGIN_NAMESPACE

// ======================================================================
namespace Subdivision_method_3 {
  //
  template <class Polyhedron>
  void CatmullClark_subdivision(Polyhedron& p, int step = 1) {
    PQQ(p, CatmullClark_mask_3<Polyhedron>(), step);
  }
  //
  template <class Polyhedron>
  void Loop_subdivision(Polyhedron& p, int step = 1) {
    PTQ(p, Loop_mask_3<Polyhedron>() , step);
  }
  //
  template <class Polyhedron>
  void DooSabin_subdivision(Polyhedron& p, int step = 1) {
    DQQ(p, DooSabin_mask_3<Polyhedron>(), step);
  }
  //
  template <class Polyhedron>
  void Sqrt3_subdivision(Polyhedron& p, int step = 1) {
    Sqrt3(p, Sqrt3_mask_3<Polyhedron>(), step);
  }

  // 
  template <class Polyhedron, template <typename> class Mask>
  void PQQ(Polyhedron& p, Mask<Polyhedron> mask, int step = 1) {
    for (int i = 0; i < step; i++) Private::PQQ_1step(p, mask);
  }
  // 
  template <class Polyhedron, template <typename> class Mask>
  void PTQ(Polyhedron& p, Mask<Polyhedron> mask, int step = 1) {
    for (int i = 0; i < step; i++) Private::PTQ_1step(p, mask);
  }
  // 
  template <class Polyhedron, template <typename> class Mask>
  void DQQ(Polyhedron& p, Mask<Polyhedron> mask, int step = 1) {
    for (int i = 0; i < step; ++i) Private::DQQ_1step(p, mask);
  }
  // 
  template <class Polyhedron, template <typename> class Mask>
  void Sqrt3(Polyhedron& p, Mask<Polyhedron> mask, int step = 1) {
    for (int i = 0; i < step; i++) Private::Sqrt3_1step(p, mask);
  }  
}

CGAL_END_NAMESPACE

#endif //_POLYHEDRON_SUBDIVISION_H_01292002
