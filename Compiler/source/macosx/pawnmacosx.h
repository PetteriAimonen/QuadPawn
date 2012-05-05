/*
 *  pawnmacosx.h
 *
 *  Common include file for Mac OS X Pawn build under Xcode.
 *
 *  Copyright (c) ITB CompuPhase, 2005
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 *
 *  Created by Allen Cronce on 06-Aug-2005.
 *
 */

#define MACOS

#if     __ppc__ || __ppc64__
#   define BYTE_ORDER   BIG_ENDIAN
#elif   __i386__
#   define BYTE_ORDER   LITTLE_ENDIAN
#else
#   error  Unknown endian!
#endif

#define HAVE_STDINT_H
#define stricmp     strcasecmp
#define strnicmp    strncasecmp

#define ANSITERM
