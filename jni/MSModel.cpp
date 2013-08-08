#include "MSModel.h"

#include <stdlib.h>
#include <string.h>

#include "Plane.h"

MSModel::MSModel() :
nVertex(0),
nFaces(0),
vertices(NULL),
normals(NULL),
texCoords(NULL),
faces(NULL)
{}

MSModel::~MSModel() {
  if (vertices)  free(vertices);
  if (normals)   free(normals);
  if (texCoords) free(texCoords);
  if (faces)     free(faces);
}

MSModel *
MSModel::create(JNIEnv *env, jobject obj) {
  // TODO, as well as Java part.
  return NULL;
}

MSModel *
MSModel::getPlane() {
  MSModel *m = new MSModel();
  m->nVertex = NUM_PLANE_OBJECT_VERTEX;
  m->nFaces  = NUM_PLANE_OBJECT_FACES;
  m->vertices  = (float *)malloc(3*m->nVertex*sizeof(float));
  memcpy(m->vertices, planeVertices, 3*m->nVertex*sizeof(float));
  m->normals   = (float *)malloc(3*m->nVertex*sizeof(float));
  memcpy(m->normals, planeNormals, 3*m->nVertex*sizeof(float));
  m->texCoords = (float *)malloc(2*m->nVertex*sizeof(float));
  memcpy(m->texCoords, planeTexCoords, 2*m->nVertex*sizeof(float));
  m->faces     = (float *)malloc(3*m->nFaces*sizeof(float));
  memcpy(m->faces, planeIndices, 3*m->nFaces*sizeof(float));
  return m;
}
