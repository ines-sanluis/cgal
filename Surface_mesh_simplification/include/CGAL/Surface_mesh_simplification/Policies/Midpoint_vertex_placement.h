// Copyright (c) 2005, 2006 Fernando Luis Cacciola Carballal. All rights reserved.
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
// $URL$
// $Id$
//
// Author(s)     : Fernando Cacciola <fernando_cacciola@ciudad.com.ar>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_MIDPOINT_VERTEX_PLACEMENT_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_MIDPOINT_VERTEX_PLACEMENT_H 1

#include <CGAL/Surface_mesh_simplification/TSMS_common.h>

CGAL_BEGIN_NAMESPACE

namespace Triangulated_surface_mesh { namespace Simplification 
{


//*******************************************************************************************************************
//                                -= new vertex placement functor =-
//
// Constructs the point of location of the new vertex that replaces a collapsing vertex-pair.
// The functor can return an empty optional if the point cannot be placed in a way that satisfies the deeesired constriants.
//
//*******************************************************************************************************************

//
// Mid-point placement
//
template<class Collapse_data_>
class Midpoint_vertex_placement
{
public:
    
  typedef Collapse_data_ Collapse_data ;
    
  typedef typename Collapse_data::Point_3 Point_3 ;
  
  typedef optional<Point_3> result_type ;
  
public:
    
  result_type operator()( Collapse_data const& data ) const
  {
    return result_type(midpoint(data.p()->point(), data.q()->point()));
  }
};


} } // namespace Triangulated_surface_mesh::Simplification

CGAL_END_NAMESPACE

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_MIDPOINT_VERTEX_PLACEMENT_H //
// EOF //
 
