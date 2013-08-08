#include <jni.h>
#include <string.h>

#include <QCAR/QCAR.h>
#include <QCAR/CameraDevice.h>

#include "MSTexture.h"
#include "MSController.h"
#include "MSTargetManager.h"
#include "MSTargetInfo.h"
#include "MSRenderer.h"
#include "MSTracker.h"
#include "MSCallback.h"

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Main_initNative(JNIEnv *env, jobject obj)
{
  MSController::init(env, obj);
  QCAR::registerCallback(MSController::getCallback());
}

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Main_deInitNative(JNIEnv *env, jobject obj)
{
  MSController::deInit();
}

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Renderer_renderFrame(JNIEnv *, jobject)
{
  MSController::getRenderer()->renderFrame();
}

JNIEXPORT bool JNICALL
Java_com_moodstocks_vuforia_Main_startCameraNative(JNIEnv *, jobject)
{
  // Initialize the camera:
  if (!QCAR::CameraDevice::getInstance().init()) return false;
  // Set video mode
  if (!QCAR::CameraDevice::getInstance().selectVideoMode(CAM_QUALITY)) return false;
  // Check that the frame resolution fits the Moodstocks SDK requirements, i.e.
  // its largest dimensions is >= 480 pixels.
  QCAR::VideoMode mode = QCAR::CameraDevice::getInstance().getVideoMode(CAM_QUALITY);
  if (!(mode.mWidth >= 480 || mode.mHeight >= 480)) return false;
  // Start the camera:
  if (!QCAR::CameraDevice::getInstance().start()) return false;
  // Start the tracker:
  MSController::getTracker()->start();
  return true;
}

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Main_stopCameraNative(JNIEnv *, jobject)
{
  // Stop the tracker:
  MSController::getTracker()->stop();
  // Stop the camera
  QCAR::CameraDevice::getInstance().stop();
  QCAR::CameraDevice::getInstance().deinit();
}

JNIEXPORT jboolean JNICALL
Java_com_moodstocks_vuforia_Main_setFocusMode(JNIEnv*, jobject, jint mode)
{
  int qcarFocusMode;

  switch ((int)mode)
  {
    case 0:
    qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_NORMAL;
    break;

    case 1:
    qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_CONTINUOUSAUTO;
    break;

    case 2:
    qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_INFINITY;
    break;

    case 3:
    qcarFocusMode = QCAR::CameraDevice::FOCUS_MODE_MACRO;
    break;

    default:
    return JNI_FALSE;
  }

  return QCAR::CameraDevice::getInstance().setFocusMode(qcarFocusMode) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Renderer_initRendering(JNIEnv*, jobject obj)
{
  MSController::initRenderer();
}

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Renderer_updateRendering(JNIEnv*, jobject, jint width, jint height, jboolean portrait)
{
  // Update screen dimensions
  MSController::setScreenSize(width, height);
  MSController::setPortrait(portrait);
  // Update the renderer
  MSController::getRenderer()->updateState();
}

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Main_startBuildingMode(JNIEnv*, jobject)
{
  MSController::getTracker()->stop();
  MSController::getTargetManager()->start();
}

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Main_stopBuildingMode(JNIEnv*, jobject)
{
  MSController::getTargetManager()->stop();
  MSController::getTracker()->start();
}


JNIEXPORT jboolean JNICALL
Java_com_moodstocks_vuforia_Main_tryBuild(JNIEnv *env,
                                          jobject,
                                          jstring jname,
                                          jfloatArray jhomog,
                                          jintArray jdims,
                                          jobject jmodel,
                                          jobject jtex,
                                          jfloatArray jscale)
{
  if (jhomog != NULL && jdims != NULL) {
    float homog[9];
    env->GetFloatArrayRegion(jhomog, 0, 9, homog);
    if (MSController::validateHomography(homog, HOMOG_TOLERANCY)) {
      int dims[2];
      env->GetIntArrayRegion(jdims, 0, 2, dims);
      float scale[3];
      env->GetFloatArrayRegion(jscale, 0, 3, scale);
      MSTexture *tex = MSTexture::create(env, jtex);
      MSController::storeNewTargetInfo(homog, dims, NULL, tex, scale);
    }
    else {
      return JNI_FALSE;
    }
  }
  int s_name = env->GetStringLength(jname);
  char name[s_name];
  const jchar *tmp = env->GetStringChars(jname, NULL);
  memcpy(name, tmp, s_name);
  env->ReleaseStringChars(jname, tmp);
  MSController::getTargetManager()->build(name);
  return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_com_moodstocks_vuforia_Main_updateTexture(JNIEnv *env, jobject, jstring jname, jobject jtex) {
  // get name
  int s_name = env->GetStringLength(jname);
  char name[s_name];
  const jchar *tmp = env->GetStringChars(jname, NULL);
  memcpy(name, tmp, s_name);
  env->ReleaseStringChars(jname, tmp);
  // build texture
  MSTexture *tex = MSTexture::create(env, jtex);
  return MSController::getTracker()->updateTexture(name, tex);
}

JNIEXPORT void JNICALL
Java_com_moodstocks_vuforia_Main_requireNewFrame(JNIEnv *, jobject) {
  MSController::getCallback()->askFrame();
}

#ifdef __cplusplus
}
#endif
