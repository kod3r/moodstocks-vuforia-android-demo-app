#include "MSController.h"
#include "MSTargetInfo.h"
#include "MSRenderer.h"
#include "MSTracker.h"
#include "MSTargetManager.h"
#include "MSCallback.h"

#include <math.h>

// Initialize static variables
MSRenderer *MSController::ms_Renderer = NULL;
MSTracker *MSController::ms_Tracker = NULL;
MSTargetManager *MSController::ms_TargetManager = NULL;
MSCallback *MSController::ms_Callback = NULL;

JavaVM *MSController::jvm = NULL;

int MSController::screenWidth = 0;
int MSController::screenHeight = 0;
bool MSController::portrait = false;
float MSController::frameRatio = -1;
MSTargetInfo *MSController::info = NULL;

void
MSController::init(JNIEnv *env, jobject obj) {
  /* Order is important! The target manager depends on
   * the tracker */
  MSController::ms_Tracker  = new MSTracker();
  MSController::ms_TargetManager = new MSTargetManager();
  MSController::ms_Callback = new MSCallback(env, obj);
  env->GetJavaVM(&MSController::jvm);
}

void
MSController::initRenderer() {
  MSController::ms_Renderer = new MSRenderer();
}

void
MSController::deInit() {
  delete MSController::ms_Renderer;
  MSController::ms_Renderer = NULL;
  delete MSController::ms_Callback;
  MSController::ms_Callback = NULL;
  /* Order is important here too, for the same reason as in `init()` */
  delete MSController::ms_TargetManager;
  MSController::ms_TargetManager = NULL;
  delete MSController::ms_Tracker;
  MSController::ms_Tracker = NULL;
}

JNIEnv *
MSController::getJNIEnv() {
  if (MSController::jvm == NULL) return NULL;
  JNIEnv* env;
  if (MSController::jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
  {
    return NULL;
  }
  return env;
}

MSRenderer *
MSController::getRenderer() {
  return MSController::ms_Renderer;
}

MSTracker *
MSController::getTracker() {
  return MSController::ms_Tracker;
}

MSTargetManager *
MSController::getTargetManager() {
  return MSController::ms_TargetManager;
}

MSCallback *
MSController::getCallback() {
  return MSController::ms_Callback;
}

void
MSController::setScreenSize(int w, int h) {
  MSController::screenWidth  = w;
  MSController::screenHeight = h;
}

void
MSController::getScreenSize(int *w, int *h) {
  *w = MSController::screenWidth;
  *h = MSController::screenHeight;
}

void
MSController::setPortrait(bool p) {
  MSController::portrait = p;
}

bool
MSController::isPortrait() {
  return MSController::portrait;
}

void
MSController::setFrameRatio(float r) {
  MSController::frameRatio = r;
}

float
MSController::getFrameRatio() {
  return MSController::frameRatio;
}

void
MSController::storeNewTargetInfo(float *h, int *d, MSModel *m, MSTexture *tex,
                                 float *scale) {
  MSController::info = new MSTargetInfo(h, d, m, tex, scale);
}

MSTargetInfo *
MSController::getNewTargetInfo() {
  return MSController::info;
}

static const float
MS_CORNERS[8] = {-1, -1, -1, 1, 1, 1, 1, -1};

bool
MSController::validateHomography(float *h, float thres) {
  /* basically checks that the projection of the 4 corners of the [-1,1]*[-1,1]
   * square form something "not too far" from a rectangle */
  float proj[8] = {0};
  // compute projections
  for (int i = 0; i < 4; ++i) {
    float x = MS_CORNERS[2*i];
    float y = MS_CORNERS[2*i+1];
    float d     =  h[6]*x+h[7]*y+1;
    proj[2*i]   = (h[0]*x+h[1]*y+h[2])/d;
    proj[2*i+1] = (h[3]*x+h[4]*y+h[5])/d;
  }
  // compute quadrilateron edges sizes.
  float dist[4] = {0};
  for (int i = 0; i < 4; ++i) {
    dist[i] = (proj[2*i]-proj[2*((i+1)%4)])*(proj[2*i]-proj[2*((i+1)%4)]) +
          (proj[2*i+1]-proj[2*((i+1)%4)+1])*(proj[2*i+1]-proj[2*((i+1)%4)+1]);
    dist[i] = sqrt(dist[i]);
  }
  // check that opposite edges have a about the same length.
  float r1 = fabs(1 - dist[0]/dist[2]);
  float r2 = fabs(1 - dist[1]/dist[3]);
  if ( r1 > thres || r2 > thres ) {
    return false;
  }
  return true;
}
