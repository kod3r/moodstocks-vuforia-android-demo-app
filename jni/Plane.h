#ifndef MS_PLANE_H
#define MS_PLANE_H

#define NUM_PLANE_OBJECT_VERTEX 4
#define NUM_PLANE_OBJECT_FACES  2

static const float planeVertices[NUM_PLANE_OBJECT_VERTEX * 3] =
{
    -1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, 1.0, 0.0,
};
static const float planeTexCoords[NUM_PLANE_OBJECT_VERTEX * 2] =
{
    0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0
};
static const float planeNormals[NUM_PLANE_OBJECT_VERTEX * 3] =
{
    0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0
};
static const unsigned short planeIndices[NUM_PLANE_OBJECT_FACES * 3] =
{
    0, 1, 2, 0, 2, 3
};


#endif
