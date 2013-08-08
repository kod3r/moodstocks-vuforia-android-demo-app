#ifndef MS_TARGETMANAGER_H
#define MS_TARGETMANAGER_H

#include <QCAR/ImageTargetBuilder.h>
#include <jni.h>

/*******************************************
 * Class managing the target-building mode.
 *******************************************/

class MSTargetManager
{
  public:
    MSTargetManager();
    ~MSTargetManager();
    void start();
    void stop();
    void build(char *name);
    /* due to the asynchronous target building, MSTargetManager
     * must be checked at each frame to see if a new target
     * is available */
    bool hasNewTrackableSource();
    QCAR::TrackableSource* getNewTrackableSource();


  private:
    bool building;
    QCAR::TrackableSource* trackableSource;

};

#endif
