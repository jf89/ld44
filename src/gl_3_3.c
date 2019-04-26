#include "gl_3_3.h"

#define GL_FUNC(return_type, name, ...) _##name *name;
GL_3_3_FUNCTIONS
#undef GL_FUNC

