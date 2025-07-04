#ifndef _GLXTARGET_H_
#define _GLXTARGET_H_

void glPlatGrabFrameBufferToTexture(unsigned long, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
void glx_UpdateWarble();
void glx_DOFUpdate(float);
void glx_DOFGrab();
void glx_ShadowTextureGrab();
void glx_ClearZBuffer();
void glx_OffsetGrab();
void glx_ColourGrab();
void glx_ShadowGrab();
void glxPostInitTargets();
void glxInitTargets();
void glx_GetSharedLock();
void glx_UnlockSharedMemory();
void glx_LockSharedMemory();
void glx_GetSharedMemorySize();
void glx_GetSharedMemory();
// void 0x8028D520..0x8028D524 | size: 0x4;
#endif // _GLXTARGET_H_
