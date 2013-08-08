#include "MSTexture.h"
#include <string.h>

MSTexture::MSTexture() :
mWidth(0),
mHeight(0),
mChannelCount(0),
mData(0),
mTextureID(0),
mHasID(false)
{}


MSTexture::~MSTexture()
{
  if (mData) delete [] mData;
}


MSTexture*
MSTexture::create(JNIEnv* env, jobject textureObject)
{

  MSTexture* newTexture = new MSTexture();

  // Handle to the Texture class:
  jclass textureClass = env->GetObjectClass(textureObject);

  // Get field IDs:
  jfieldID widthID = env->GetFieldID(textureClass, "mWidth", "I");
  jfieldID heightID = env->GetFieldID(textureClass, "mHeight", "I");
  jfieldID dataID = env->GetFieldID(textureClass, "mData", "[B");

  if (!(widthID && heightID && dataID))
  {
    delete newTexture;
    return 0;
  }
  newTexture->mWidth = env->GetIntField(textureObject, widthID);
  newTexture->mHeight = env->GetIntField(textureObject, heightID);
  newTexture->mChannelCount = 4;

  // Get data:
  jobject obj = env->GetObjectField(textureObject, dataID);
  jbyteArray pixelBuffer = reinterpret_cast<jbyteArray>(obj);

  jboolean isCopy;
  jbyte* pixels = env->GetByteArrayElements(pixelBuffer, &isCopy);
  if (pixels == NULL)
  {
    env->ReleaseByteArrayElements(pixelBuffer, pixels, 0);
    delete newTexture;
    return 0;
  }

  newTexture->mData = new unsigned char[newTexture->mWidth * newTexture->mHeight * newTexture->mChannelCount];

  int rowSize = newTexture->mWidth * newTexture->mChannelCount;
  for (int r = 0; r < newTexture->mHeight; ++r)
  {
    memcpy(newTexture->mData + rowSize * r, pixels + rowSize * (newTexture->mHeight - 1 - r), newTexture->mWidth * 4);
  }

  // Release:
  env->ReleaseByteArrayElements(pixelBuffer, pixels, 0);

  return newTexture;
}

