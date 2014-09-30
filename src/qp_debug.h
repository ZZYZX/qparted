/***************************************************************************
                          qp_debug.h  -  description
                             -------------------
    begin                : Wed Sep  4 19:21:54 UTC 2002
    copyright            : (C) 2002 by Francois Dupoux
    email                : fdupoux@partimage.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QP_DEBUG_H
#define QP_DEBUG_H

#include <stdio.h>

class QP_Debug;
extern QP_Debug g_debug;

#define showDebug(format, args...) g_debug.write(__FILE__, __FUNCTION__, __LINE__, format, ## args)

class QP_Debug
{
   public:
      QP_Debug();
      ~QP_Debug();

      int open();
      int close();
      int isOpen();
      int write(const char *fmt, ...);
      int write(const char *szFile, const char *szFunction, int nLine, const char *fmt, ...);

   private:
      FILE *m_fDebug;
};

#endif // DEBUG_H
