#ifndef MS_CALLBACK_H
#define MS_CALLBACK_H

#include <QCAR/UpdateCallback.h>
#include <jni.h>

/************************************************************
 * This class receives a callback each time the Vuforia SDK
 * has finished processing a frame, with the results of this
 * processing.
 * We use this callback to send the frames to the Moodstocks
 * SDK when needed, and to analyze the results found by the
 * Vuforia SDK.
 ************************************************************/

class MSCallback : public QCAR::UpdateCallback
{

  public:
    // Constructor
    MSCallback(JNIEnv *env, jobject obj);
    // Destructor
    virtual ~MSCallback();
    // Vuforia callback
    virtual void QCAR_onUpdate(QCAR::State& state);
    // Function to inform the callback that the next frame
    // should be sent to the caller activity.
    void askFrame();

  private:
    /* members */
    // the caller activity
    jobject activity;
    // its needed IDs
    jmethodID onPreviewFrameID;
    jfieldID dataID;
    jfieldID widthID;
    jfieldID heightID;
    jfieldID strideID;
    // set to true to send frame to caller activity
    bool needFrame;

    /* methods */
    // Sends a frame to the caller activity.
    void sendImage(QCAR::State& state);
};

#endif
