#ifndef _GLVIEW_H_
#define _GLVIEW_H_

#include "NL/nlMath.h"
#include "NL/gl/gl.h"

class glModel;
class glModelPacket;

void glViewSetEnable(eGLView, bool);
void glViewGetEnable(eGLView);
void glViewSetFilterSource(eGLView, eGLTarget);
void glViewGetFilter(eGLView);
void glViewSetFilter(eGLView, eGLFilter);
void glViewSetTarget(eGLView, eGLTarget);
void glViewProjectPoint(eGLView, const nlVector3&, nlVector3&);
void glViewGetProjectionMatrix(eGLView, nlMatrix4&);
void glViewGetViewMatrix(eGLView, nlMatrix4&);
void glViewSetProjectionMatrix(eGLView, unsigned long);
void glViewGetProjectionMatrix(eGLView);
void glViewSetViewMatrix(eGLView, unsigned long);
void glViewGetViewMatrix(eGLView);
void glViewSetSortMode(eGLView, eGLViewSort);
void glViewSetDepthClear(eGLView, bool);
void glViewGetDepthClear(eGLView);
void gl_ViewStartup();
void gl_ViewIterate(eGLView, void (*)(eGLView, unsigned long, const glModelPacket*));
void glViewCompact();
void gl_ViewReset();
void glViewAttachPacket(eGLView, const glModelPacket*);
void glViewAttachModel(eGLView, const glModel*);
void glViewAttachModel(eGLView, unsigned long, const glModel*);
void gl_ViewGetRenderList(eGLView);

#endif // _GLVIEW_H_
