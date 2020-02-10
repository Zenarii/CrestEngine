#ifndef CRESTWin32OPENGLH
#define CRESTWin32OPENGLH

#define OpenGLProc(name, type) PFNGL##type##PROC gl##name;
#include "OpenGLProcedures.h"

#endif
