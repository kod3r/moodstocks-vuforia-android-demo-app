#ifndef MS_MODEL_H
#define MS_MODEL_H

#include <jni.h>

class MSModel {

  public:
    /* methods */
    MSModel();
    ~MSModel();
    // create from Java. NOT IMPLEMENTED YET.
    static MSModel *create(JNIEnv *env, jobject obj);
    // get a simple square going from -1 to 1 in x and y.
    static MSModel *getPlane();

    /* members */
    int nVertex;
    int nFaces;
    float *vertices;
    float *normals;
    float *texCoords;
    float *faces;
};

#endif
