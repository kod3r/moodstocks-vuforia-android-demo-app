#ifndef MS_CONTROLLER_H
#define MS_CONTROLLER_H

#include <stdlib.h>
#include <jni.h>

/************************************************************
 * This class centralizes the state of the application, as
 * well as its sub-components and some constants.
 ************************************************************/

#define MODEL_SIZE      2.0
#define CAM_QUALITY     QCAR::CameraDevice::MODE_OPTIMIZE_QUALITY
#define HOMOG_TOLERANCY 0.1

class MSRenderer;
class MSTracker;
class MSTexture;
class MSTargetInfo;
class MSTargetManager;
class MSCallback;
class MSModel;

class MSController {

  public:
  // Initialize the non-rendering-related components
  static void init(JNIEnv *env, jobject obj);
  // Initializes the renderer. Must be called from GL thread.
  static void initRenderer();
  // De-initializes all the components
  static void deInit();
  // Obtain the current JNI Environment.
  static JNIEnv *getJNIEnv();
  // Accessors to the sub-components
  static MSRenderer *getRenderer();
  static MSTracker *getTracker();
  static MSTargetManager *getTargetManager();
  static MSCallback *getCallback();
  // Getters and setters for screen dimensions and device orientation
  static void setScreenSize(int w, int h);
  static void getScreenSize(int *w, int *h);
  static void setPortrait(bool p);
  static bool isPortrait();
  // Getter and setter for the camera frames ratio (=width/height).
  static void setFrameRatio(float r);
  static float getFrameRatio();
  // Compute and store the target information of the latest found target
  static void storeNewTargetInfo(float *h, int *d, MSModel *m, MSTexture *tex,
                                 float *scale);
  // Retrieve them.
  static MSTargetInfo *getNewTargetInfo();
  // Utility function to check that the homography fits Vuforia requirements
  static bool validateHomography(float *h, float thres);

  private:
  static MSRenderer *ms_Renderer;
  static MSTracker *ms_Tracker;
  static MSTargetManager *ms_TargetManager;
  static MSCallback *ms_Callback;
  static int screenWidth;
  static int screenHeight;
  static bool portrait;
  static float frameRatio;
  static MSTargetInfo *info;
  static JavaVM *jvm;


};
#endif
