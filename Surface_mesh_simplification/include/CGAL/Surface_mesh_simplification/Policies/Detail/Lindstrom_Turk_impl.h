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
// $URL: $
// $Id: $
//
// Author(s)     : Fernando Cacciola <fernando_cacciola@ciudad.com.ar>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_LINDSTROM_TURK_IMPL_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_LINDSTROM_TURK_IMPL_H 1

CGAL_BEGIN_NAMESPACE

//
// Implementation of the Vertex Placement strategy from:
//
//  "Fast and Memory Efficient Polygonal Symplification"
//  Peter Lindstrom, Greg Turk
//

namespace Triangulated_surface_mesh { namespace Simplification 
{

template<class CD>
LindstromTurkImpl<CD>::LindstromTurkImpl( Params const&            aParams
                                        , vertex_descriptor const& aP
                                        , vertex_descriptor const& aQ
                                        , edge_descriptor const&   aP_Q
                                        , edge_descriptor const&   aQ_P
                                        , TSM&                     aSurface 
                                        )
  :
   mParams(aParams)
  ,mP(aP)
  ,mQ(aQ)
  ,mP_Q(aP_Q)
  ,mQ_P(aQ_P)
  ,mSurface(aSurface)    
{
  //
  // Each vertex constrian is an equation of the form: Ai * v = bi
  // Where 'v' is a CVector representing the vertex,
  // Ai is a (row) CVector 
  // and bi a scalar.
  //
  // The vertex is completely determined with 3 such constrian, so is the solution
  // to the folloing system:
  //
  //  A.r0(). * v = b0
  //  A1 * v = b1
  //  A2 * v = b2
  //
  // Which in matrix form is :  A * v = b
  //
  // (with A a 3x3 matrix and b a vector)
  //
  // The member variable mConstrinas contains A and b. Indidivual constrians (Ai,bi) can be added to it.
  // Once 3 such constrians have been added v is directly solved a:
  //
  //  v = b*inverse(A)
  //
  // A constrian (Ai,bi) must be alpha-compatible with the previously added constrians (see Paper); if it's not, is discarded.
  //
  
  
  
  // Volume preservation and optimization constrians are based on the normals to the triangles in the star of the collapsing egde 
  // Triangle shape optimization constrians are based on the link of the collapsing edge (the cycle of vertices around the edge)
  Triangles lTriangles;
  Link      lLink;
  
  lTriangles.reserve(16);
  lLink     .reserve(16);
  
  Extract_triangles_and_link(lTriangles,lLink);

  // If the collapsing edge is a boundary edge, the "local boundary" is cached in a Boundary object.    
  OptionalBoundary lBdry ;
  
  if ( is_undirected_edge_a_border(mP_Q) )
  {
    lBdry  = Extract_boundary();
    Add_boundary_preservation_constrians(lBdry);
  }
  
  if ( mConstrians.n < 3 )
    Add_volume_preservation_constrians(lTriangles);
    
  if ( mConstrians.n < 3 )
    Add_boundary_and_volume_optimization_constrians(lBdry,lTriangles); 
    
  if ( mConstrians.n < 3 )
    Add_shape_optimization_constrians(lLink);
    
  Optional_FT    lCost ;
  Optional_point lVertexPoint ;
  
  // It might happen that there were not enough alpha-compatible constrians.
  // In that case there is simply no good vertex placement (mResult is left absent)
  if ( mConstrians.n == 3 ) 
  {
    optional<CMatrix> OptAi = inverse_matrix(mConstrians.A);
    if ( OptAi )
    {
      CMatrix const& Ai = *OptAi ;
      
      CVector v = mConstrians.b * Ai ;
      
      lVertexPoint = Optional_point(ORIGIN + toVector(v) ) ;
      
    }
  }
    
  mResult = result_type( new Collapse_data(mP,mQ,mP_Q,mSurface,lCost,lVertexPoint) );
}


//
// Caches the "local boundary", that is, the sequence of 3 border edges: o->p, p->q, q->e 
//
template<class CD>
typename LindstromTurkImpl<CD>::OptionalBoundary LindstromTurkImpl<CD>::Extract_boundary()
{
  // Since p_q is a boundary edge, one of the previous edges (ccw or cw) is the previous boundary edge
  // Likewise, one of the next edges (ccw or cw) is the next boundary edge.
  edge_descriptor p_pt = next_edge_ccw(mP_Q,mSurface);
  edge_descriptor p_pb = next_edge_cw (mP_Q,mSurface);
  edge_descriptor q_qt = next_edge_cw (mQ_P,mSurface);
  edge_descriptor q_qb = next_edge_ccw(mQ_P,mSurface);
  
  edge_descriptor border_1 = mP_Q;
  edge_descriptor border_0 = is_undirected_edge_a_border(p_pt) ? p_pt : p_pb ;
  edge_descriptor border_2 = is_undirected_edge_a_border(q_qt) ? q_qt : q_qb ;
  
  CGAL_assertion(is_undirected_edge_a_border(border_0));
  CGAL_assertion(is_undirected_edge_a_border(border_2));

  // opposite(border0)->border1->border2 is the local boundary
  
  vertex_descriptor ov = target(border_0,mSurface);
  vertex_descriptor rv = target(border_2,mSurface);
  
  // o->p->q->r is the local boundary
  
  Point o = get_point(ov);
  Point p = get_point(mP);
  Point q = get_point(mQ);
  Point r = get_point(rv);
  
  //
  // The boundary cached contains vectors instead of points
  //
  
  CVector op  = toCVector(p - o) ;
  CVector opN = Point_cross_product(p,o);
  
  CVector pq  = toCVector(q - p) ;
  CVector pqN = Point_cross_product(q,p);
  
  CVector qr  = toCVector(r - q) ;
  CVector qrN = Point_cross_product(r,q);
  
  return OptionalBoundary(Boundary(op,opN,pq,pqN,qr,qrN)) ;
}

//
// Calculates the normal of the triangle (v0,v1,v2) (both vector and its length as (v0xv1).v2)
//
template<class CD>
typename LindstromTurkImpl<CD>::Triangle LindstromTurkImpl<CD>::Get_triangle( vertex_descriptor const& v0
                                                                            , vertex_descriptor const& v1
                                                                            , vertex_descriptor const& v2 
                                                                            )
{
  Point p0 = get_point(v0);
  Point p1 = get_point(v1);
  Point p2 = get_point(v2);
  
  CVector v01 = toCVector(p1 - p0) ;
  CVector v02 = toCVector(p2 - p0) ;
  
  CVector lNormalV = cross_product(v01,v02);
  
  FT lNormalL = Point_cross_product(p0,p1) * toCVector(p2-ORIGIN);
  
  return Triangle(lNormalV,lNormalL);
}                              

//
// If (v0,v1,v2) is a finite triangular facet of the mesh, that is, NONE of these vertices are boundary vertices,
// the triangle (properly oriented) is added to rTriangles.
// The triangle is encoded as its normal, calculated using the actual facet orientation [(v0,v1,v2) or (v0,v2,v1)]
//
template<class CD>
void LindstromTurkImpl<CD>::Extract_triangle( vertex_descriptor const& v0
                                            , vertex_descriptor const& v1
                                            , vertex_descriptor const& v2 
                                            , edge_descriptor   const& e02
                                            , Triangles&               rTriangles
                                            )
{
  // The 3 vertices are obtained by circulating ccw around v0, that is, e02 = next_ccw(e01).
  // Since these vertices are NOT obtained by circulating the face, the actual triangle orientation is unspecified.
  
  // If target(next_edge(v0))==v1 then the triangle is oriented v0->v2->v1; otherwise is oriented v0->v1->v2 ;
  if ( target(next_edge(e02,mSurface),mSurface) == v1 ) 
  {
    // The triangle is oriented v0->v2->v1.
    // In this case e02 is an edge of the facet.
    // If this facet edge is a border edge then this triangle is not in the mesh .
    if ( !is_border(e02) )
      rTriangles.push_back(Get_triangle(v0,v2,v1) ) ;
  }
  else
  {
    // The triangle is oriented v0->v1->v2.
    // In this case, e20 and not e02, is an edge of the facet.
    // If this facet edge is a border edge then this triangle is not in the mesh .
    if ( !is_border(opposite_edge(e02,mSurface)) )
      rTriangles.push_back(Get_triangle(v0,v1,v2) ) ;
  }
}

//
// Extract all triangles (its normals) and vertices (the link) around the collpasing edge p_q
//
template<class CD>
void LindstromTurkImpl<CD>::Extract_triangles_and_link( Triangles& rTriangles, Link& rLink )
{
  // 
  // Extract around mP CCW
  //  
  vertex_descriptor v0 = mP;
  vertex_descriptor v1 = mQ;
  
  edge_descriptor e02 = mP_Q;
  
  do
  {
    e02 = next_edge_ccw(e02,mSurface);
    
    vertex_descriptor v2 = target(e02,mSurface);
  
    if ( v2 != mQ )
      rLink.push_back(v2) ;
      
    Extract_triangle(v0,v1,v2,e02,rTriangles);
    
    v1 = v2 ;
  }
  while ( e02 != mP_Q ) ;
  
  // 
  // Extract around mQ CCW
  //  
  vertex_descriptor vt = target(next_edge_cw(mQ_P,mSurface),mSurface); // This was added to the link while circulating mP
  
  v0 = mQ;
  
  e02 = next_edge_ccw(mQ_P,mSurface);
  
  v1 = target(e02,mSurface); // This was added to the link while circulating around mP
  
  e02 = next_edge_ccw(e02,mSurface);
  
  do
  {
    vertex_descriptor v2 = target(e02,mSurface);

    if ( v2 != vt )  
      rLink.push_back(v2) ;
    
    Extract_triangle(v0,v1,v2,e02,rTriangles);
    
    v1 = v2 ;
     
    e02 = next_edge_cw(e02,mSurface);
    
  }
  while ( e02 != mQ_P ) ;
}

template<class CD>
void LindstromTurkImpl<CD>::Add_boundary_preservation_constrians( OptionalBoundary const& aBdry )
{
  CVector e1 = aBdry->op  + aBdry->pq  + aBdry->qr ;
  CVector e3 = aBdry->opN + aBdry->pqN + aBdry->qrN ;

  CMatrix H = LT_product(e1);
  
  CVector c = cross_product(e1,e3);
  
  mConstrians.Add_from_gradient(H,c);
}

template<class CD>
void LindstromTurkImpl<CD>::Add_volume_preservation_constrians( Triangles const& aTriangles )
{
  CVector lSumV = NULL_VECTOR ;
  FT      lSumL(0) ;
  
  for( typename Triangles::const_iterator it = aTriangles.begin(), eit = aTriangles.end() ; it != eit ; ++it )
  {
    lSumV = lSumV + it->NormalV ;
    lSumL = lSumL + it->NormalL ;  
  }   
  
  mConstrians.Add_if_alpha_compatible(lSumV,lSumL);   

}

template<class CD>
void LindstromTurkImpl<CD>::Add_boundary_and_volume_optimization_constrians( OptionalBoundary const& aBdry, Triangles const& aTriangles )
{
  CMatrix H = NULL_MATRIX ;
  CVector c = NULL_VECTOR ;

  //
  // Volume optimization  
  //
  for( typename Triangles::const_iterator it = aTriangles.begin(), eit = aTriangles.end() ; it != eit ; ++it )
  {
    Triangle const& lTri = *it ;
    
    H += direct_product(lTri.NormalV,lTri.NormalV) ;
    
    c = c + ( lTri.NormalL * lTri.NormalV ) ;
  }   
  
  
  if ( aBdry )
  {
    //
    // Boundary optimization
    //
    CMatrix Hb = LT_product(aBdry->op) + LT_product(aBdry->pq) + LT_product(aBdry->qr) ;
    
    CVector cb =  cross_product(aBdry->op,aBdry->opN) + cross_product(aBdry->pq,aBdry->pqN) + cross_product(aBdry->qr,aBdry->qrN);
    
    //
    // Weighted average
    //
    FT lBoundaryWeight = ( FT(9) * mParams.BoundaryWeight * squared_distance ( get_point(mP), get_point(mQ) ) ) / FT(10) ;
    
    H *= mParams.VolumeWeight ;
    c = c * mParams.VolumeWeight ;
    
    H += lBoundaryWeight * Hb ;
    c = c + ( lBoundaryWeight * cb ) ;
    
  }
  
  mConstrians.Add_from_gradient(H,c);
}

template<class CD>
void LindstromTurkImpl<CD>::Add_shape_optimization_constrians( Link const& aLink )
{
  FT s(aLink.size());
  
  CMatrix H (s,0,0
            ,0,s,0
            ,0,0,s
            );
           
  CVector c = NULL_VECTOR ;
  
  for( typename Link::const_iterator it = aLink.begin(), eit = aLink.end() ; it != eit ; ++it )
    c = c + toCVector(ORIGIN - get_point(*it)) ;  
           
  mConstrians.Add_from_gradient(H,c);
}

template<class Vector>
Vector normalized_vector ( Vector const& c )
{
}

template<class CD>
void LindstromTurkImpl<CD>::Constrians::Add_if_alpha_compatible( CVector const& Ai, FT const& bi )
{
  double slai = to_double(Ai*Ai) ;
  if ( slai > 0.0 )
  {
    bool lAddIt = true ;
    
    if ( n == 1 )
    {
      FT d01 = A.r0() * Ai  ;
      
      double sla0 = to_double(A.r0() * A.r0()) ;
      double sd01 = to_double(d01 * d01) ;
      
      if ( sd01 > ( sla0 * slai * squared_cos_alpha() ) )
        lAddIt = false ;
    }
    else if ( n == 2 )
    {
      CVector N = cross_product(A.r0(),A.r1());
      
      FT dc012 = N * Ai ;
      
      double slc01  = to_double(N * N) ;
      double sdc012 = to_double(dc012 * dc012);       
      
      if ( sdc012 <= slc01 * slai * squared_sin_alpha() )
        lAddIt = false ;
    }
    
    if ( lAddIt )
    {
      switch ( n )
      {
        case 0 :
          A.r0() = Ai ;
          b = CVector(bi,b.y(),b.z());
          break ;
        case 1 :
          A.r1() = Ai ;
          b = CVector(b.x(),bi,b.z());
          break ;
        case 2 :
          A.r2() = Ai ;
          b = CVector(b.x(),b.y(),bi);
          break ;
      }
      ++ n ;
    }
  }
  
}

template<class V>
int index_of_max_component ( V const& v )
{
  typedef typename Kernel_traits<V>::Kernel::FT FT ;
  
  int i = 0 ;
  FT max = v.x();
  if ( max < v.y() )
  {
    max = v.y();
    i = 1 ;
  }  
  if ( max < v.z() )
  {
    max = v.z();
    i = 2 ;
  }  
  return i ;
}

template<class CD>
void LindstromTurkImpl<CD>::Constrians::Add_from_gradient ( CMatrix const& H, CVector const& c )
{
  CGAL_precondition(n >= 0 && n<=2 );
  
  switch(n)
  {
    case 0 :
    
      Add_if_alpha_compatible(H.r0(),-c.x());
      Add_if_alpha_compatible(H.r1(),-c.y());
      Add_if_alpha_compatible(H.r2(),-c.z());
      
      break;  
      
    case 1 :
      {
        CVector const& A0 = A.r0();
        
        CVector A02( A0.x()*A0.x()
                   , A0.y()*A0.y()
                   , A0.z()*A0.z()
                   );
           
        CVector Q0 ;      
        switch ( index_of_max_component(A02) ) 
        {
          case 0: Q0 = CVector(- A0.z()/A0.x(),0              ,1              ); break;
          case 1: Q0 = CVector(0              ,- A0.z()/A0.y(),1              ); break;
          case 2: Q0 = CVector(1              ,0              ,- A0.x()/A0.z()); break;
        }
        
        CVector Q1 = cross_product(A0,Q0);
    
        CVector A1 = H * Q0 ;
        CVector A2 = H * Q1 ;
        FT b1 = - ( Q0 * c ) ;
        FT b2 = - ( Q1 * c ) ;
        
        Add_if_alpha_compatible(A1,b1);
        Add_if_alpha_compatible(A2,b2);
        
      }
      break ;
    
    case 2:
      {
    
        CVector Q = cross_product(A.r0(),A.r1());
        
        CVector A2 = H * Q ;
        
        FT b2 = - ( Q * c ) ;
        
        Add_if_alpha_compatible(A2,b2);
        
      }
      break ;
      
  }
}

} } // namespace Triangulated_surface_mesh::Simplification

CGAL_END_NAMESPACE

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_LINDSTROMTURK_IMPL_H //
// EOF //
 

