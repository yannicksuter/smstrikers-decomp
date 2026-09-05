#include "NL/gl/glDraw3.h"

#include "NL/gl/gl.h"
#include "NL/gl/glState.h"
#include "NL/gl/glView.h"
#include "NL/gl/glStruct.h"
#include "NL/gl/glMatrix.h"

#include "NL/gl/glModel.h"
#include "NL/gl/glUserData.h"

// #include "Game/GL/GLMeshWriter.h"
#include "Game/GL/gluMeshWriter.h"

static unsigned long _defaultProgram = glGetProgram("3d unlit");

/**
 * Offset/Address/Size: 0x4B4 | 0x801D8300 | size: 0x6C
 */
bool glAttachQuad3(eGLView view, unsigned long count, glQuad3* quads, bool useDefaultProgram)
{
    int i = 0;
    glQuad3* quad = quads;
    for (; i < count; i++, quad++)
    {
        glViewAttachModel(view, 0, quad->GetModel(useDefaultProgram));
    }
    return true;
}

/**
 * Offset/Address/Size: 0x2F8 | 0x801D8144 | size: 0x1BC
 */
const glModel* glQuad3::GetModel(bool useDefaultProgram) const
{
    glModel* newModel;
    unsigned long prevProgram;
    eGLStream sStreams[3] = { (eGLStream)0, (eGLStream)2, (eGLStream)3 };
    static int stripmap[4] = { 3, 0, 2, 1 };

    newModel = 0;

    GLMeshWriter writer;
    gl_GetCurrentStateBundle();

    if (useDefaultProgram)
        prevProgram = glSetCurrentProgram(_defaultProgram);

    do
    {
        if (glHasQuads())
        {
            if (!writer.Begin(4, (eGLPrimitive)3, 3, sStreams, false))
            {
                break;
            }

            for (int i = 0; i < 4; ++i)
            {
                writer.Colour(m_colour[i]);
                writer.Texcoord(m_uv[i]);
                writer.Vertex(m_pos[i]);
            }
        }
        else
        {
            if (!writer.Begin(4, (eGLPrimitive)1, 3, sStreams, false))
            {
                break;
            }

            int srcIndex;
            for (int i = 0; i < 4; ++i)
            {
                srcIndex = stripmap[i];
                writer.Colour(m_colour[srcIndex]);
                writer.Texcoord(m_uv[srcIndex]);
                writer.Vertex(m_pos[srcIndex]);
            }
        }

        if (writer.End())
            newModel = writer.GetModel();
    } while (false);

    if (useDefaultProgram)
        glSetCurrentProgram(prevProgram);

    return newModel;
}

/**
 * Offset/Address/Size: 0x2A8 | 0x801D80F4 | size: 0x50
 */
bool glQuad3::Attach(eGLView view, int stream, bool useDefaultProgram)
{
    glViewAttachModel(view, (unsigned long)stream, GetModel(useDefaultProgram));
    return true;
}

/**
 * Offset/Address/Size: 0x5C | 0x801D7EA8 | size: 0x24C
 */
void glQuad3::SetupRotatedRectangle(float w, float h, const nlMatrix4& mtx, bool flipU, bool flipV)
{
    nlVector3 corner;

    float half = 0.5f;
    float negHalfH;
    float halfW;
    float negHalfW;
    float halfH;

    negHalfW = -w * half;
    negHalfH = -h * half;
    nlVec3Set(corner, negHalfW, negHalfH, 0.0f);

    nlMultPosVectorMatrix(corner, corner, mtx);
    m_pos[0] = corner;

    halfW = w * half;
    nlVec3Set(corner, halfW, negHalfH, 0.0f);

    nlMultPosVectorMatrix(corner, corner, mtx);
    m_pos[1] = corner;

    halfH = h * half;
    nlVec3Set(corner, halfW, halfH, 0.0f);

    nlMultPosVectorMatrix(corner, corner, mtx);
    m_pos[2] = corner;

    nlVec3Set(corner, negHalfW, halfH, 0.0f);
    nlMultPosVectorMatrix(corner, corner, mtx);
    m_pos[3] = corner;

    f32 u0;
    f32 u1;
    f32 v0;
    f32 v1;

    if (flipU != 0)
    {
        u0 = 1.0f;
    }
    else
    {
        u0 = 0.0f;
    }

    if (flipU != 0)
    {
        u1 = 0.0f;
    }
    else
    {
        u1 = 1.0f;
    }

    if (flipV != 0)
    {
        v0 = 1.0f;
    }
    else
    {
        v0 = 0.0f;
    }

    if (flipV != 0)
    {
        v1 = 0.0f;
    }
    else
    {
        v1 = 1.0f;
    }

    m_uv[0].x = u0;
    m_uv[0].y = v0;
    m_uv[1].x = u1;
    m_uv[1].y = v0;
    m_uv[2].x = u1;
    m_uv[2].y = v1;
    m_uv[3].x = u0;
    m_uv[3].y = v1;

    m_colour[0].c[0] = 0xFF;
    m_colour[0].c[1] = 0xFF;
    m_colour[0].c[2] = 0xFF;
    m_colour[0].c[3] = 0xFF;
    m_colour[1].c[0] = 0xFF;
    m_colour[1].c[1] = 0xFF;
    m_colour[1].c[2] = 0xFF;
    m_colour[1].c[3] = 0xFF;
    m_colour[2].c[0] = 0xFF;
    m_colour[2].c[1] = 0xFF;
    m_colour[2].c[2] = 0xFF;
    m_colour[2].c[3] = 0xFF;
    m_colour[3].c[0] = 0xFF;
    m_colour[3].c[1] = 0xFF;
    m_colour[3].c[2] = 0xFF;
    m_colour[3].c[3] = 0xFF;
}

/**
 * Offset/Address/Size: 0x44 | 0x801D7E90 | size: 0x18
 */
void glQuad3::SetColour(const nlColour& c)
{
    const unsigned long packed = *(const unsigned long*)(&c);
    *(unsigned long*)(&m_colour[0]) = packed;
    *(unsigned long*)(&m_colour[1]) = packed;
    *(unsigned long*)(&m_colour[2]) = packed;
    *(unsigned long*)(&m_colour[3]) = packed;
}

/**
 * Offset/Address/Size: 0x0 | 0x801D7E4C | size: 0x44
 */
void glQuad3::SetColour(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    unsigned char* dst = (unsigned char*)(&m_colour[0]);
    dst[0x0] = r;
    dst[0x1] = g;
    dst[0x2] = b;
    dst[0x3] = a;
    dst[0x4] = r;
    dst[0x5] = g;
    dst[0x6] = b;
    dst[0x7] = a;
    dst[0x8] = r;
    dst[0x9] = g;
    dst[0xA] = b;
    dst[0xB] = a;
    dst[0xC] = r;
    dst[0xD] = g;
    dst[0xE] = b;
    dst[0xF] = a;
}
