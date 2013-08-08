#include "MSCallback.h"
#include "MSController.h"
#include "MSTargetManager.h"
#include "MSTracker.h"

#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>
#include <QCAR/Frame.h>
#include <QCAR/State.h>
#include <QCAR/Image.h>

#include <assert.h>

MSCallback::MSCallback(JNIEnv *env, jobject obj) :
needFrame (false)
{
  jclass cls = env->GetObjectClass(obj);
  this->activity = env->NewWeakGlobalRef(obj);
  this->onPreviewFrameID = env->GetMethodID(cls, "onPreviewFrame", "()V");
  this->dataID = env->GetFieldID(cls, "imgData", "[B");
  this->widthID = env->GetFieldID(cls, "imgWidth", "I");
  this->heightID = env->GetFieldID(cls, "imgHeight", "I");
  this->strideID = env->GetFieldID(cls, "imgStride", "I");
}

MSCallback::~MSCallback() {
  JNIEnv *env = MSController::getJNIEnv();
  env->DeleteWeakGlobalRef(this->activity);
}

void
MSCallback::QCAR_onUpdate(QCAR::State& state) {
  // Check if there's a new target. If so, add it to the tracker.
  MSTargetManager *targetManager = MSController::getTargetManager();
  if (targetManager->hasNewTrackableSource()) {
    // exit Target Building Mode
    MSController::getTargetManager()->stop();
    MSController::getTracker()->start();
    // add target.
    MSController::getTracker()->addTrackableSource(targetManager->getNewTrackableSource());
  }
  // Pass a frame to Moodstocks SDK if asked for, and if not currently tracking.
  else if (needFrame && !state.getNumTrackableResults()) {
    needFrame = false;
    sendImage(state);
  }
}

void
MSCallback::sendImage(QCAR::State& state) {
  // Access frame
  QCAR::Frame f = state.getFrame();
  const QCAR::Image *img = f.getImage(0);

  // Get its dimensions
  int w = img->getWidth();
  int h = img->getHeight();
  int stride = img->getStride();

  JNIEnv *env = MSController::getJNIEnv();
  jobject local = env->NewLocalRef(activity);
  // Set Java byte array and dimensions if not already done.
  if (!env->IsSameObject(local, NULL)) {
    jobject jobj = env->GetObjectField(local, dataID);
    if (env->IsSameObject(jobj, NULL)) {
      jobject jarray = env->NewByteArray(h*stride);
      env->SetObjectField(local, dataID, jarray);
      jobj = env->GetObjectField(local, dataID);
      env->SetIntField(local, widthID, w);
      env->SetIntField(local, heightID, h);
      env->SetIntField(local, strideID, stride);
    }
    // Set image data
    jbyteArray jdata = reinterpret_cast<jbyteArray>(jobj);
    env->SetByteArrayRegion(jdata, 0, h*stride, (const jbyte*)img->getPixels());
    // inform the Java code that a new preview frame is available.
    env->CallVoidMethod(local, onPreviewFrameID);
  }
  env->DeleteLocalRef(local);
}

void
MSCallback::askFrame() {
  if (!needFrame)
    needFrame = true;
}
