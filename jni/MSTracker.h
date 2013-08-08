#ifndef MS_TRACKER_H
#define MS_TRACKER_H

#include <QCAR/TrackableSource.h>
#include <QCAR/Dataset.h>

class MSTexture;

class MSTracker {

  public:
    MSTracker();
    ~MSTracker();
    // adds a new target to track.
    void addTrackableSource(QCAR::TrackableSource *t);
    // updates the texture used for the target with given name.
    bool updateTexture(char *name, MSTexture *tex);
    void start();
    void stop();

  private:
    QCAR::DataSet *dataset;

};
#endif
