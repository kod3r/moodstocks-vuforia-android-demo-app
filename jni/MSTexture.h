#ifndef MS_TEXTURE_H
#define MS_TEXTURE_H

#include <jni.h>

class MSTexture
{
  public:
    MSTexture();
    ~MSTexture();
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    // Create a texture from a Java texture object
    static MSTexture* create(JNIEnv* env, jobject textureObject);
    unsigned int mWidth;
    unsigned int mHeight;
    unsigned int mChannelCount;
    unsigned char* mData;
    unsigned int mTextureID;
    // true if the ID has already been set, false otherwise.
    bool mHasID;
};


#endif
