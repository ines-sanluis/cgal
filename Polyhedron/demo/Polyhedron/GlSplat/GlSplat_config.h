#ifndef GLSPLAT_CONFIG_H
#define GLSPLAT_CONFIG_H

#include <CGAL/export/helpers.h>

#ifdef gl_splat_EXPORTS
  #define GLSPLAT_EXPORT Q_DECL_EXPORT
#else
  #define GLSPLAT_EXPORT Q_DECL_IMPORT
#endif

#endif // GLSPLAT_CONFIG_H
