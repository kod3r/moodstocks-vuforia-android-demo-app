#include "MSTracker.h"
#include "MSTargetInfo.h"
#include "MSController.h"

#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>
#include <QCAR/Trackable.h>

#include <string.h>

MSTracker::MSTracker() {
  QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
  QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(trackerManager.initTracker(QCAR::Tracker::IMAGE_TRACKER));

  // Create and activate the data set:
  this->dataset = imageTracker->createDataSet();
  imageTracker->activateDataSet(this->dataset);
}

MSTracker::~MSTracker() {
  QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
  QCAR::ImageTracker* imageTracker = static_cast<QCAR::ImageTracker*>(
          trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));

  if (imageTracker != NULL && this->dataset != 0)
  {
      imageTracker->deactivateDataSet(this->dataset);
      int n = this->dataset->getNumTrackables();
      for (int i = 0; i < n; ++i) {
          QCAR::Trackable *t = this->dataset->getTrackable(i);
          if (t->getUserData()) {
              MSTargetInfo *info = static_cast<MSTargetInfo *>(t->getUserData());
              delete info;
              t->setUserData(NULL);
          }
      }
      imageTracker->destroyDataSet(this->dataset);
      this->dataset = NULL;
  }

  trackerManager.deinitTracker(QCAR::Tracker::IMAGE_TRACKER);
}

void
MSTracker::addTrackableSource(QCAR::TrackableSource *t) {
  QCAR::TrackerManager& trackerManager =
          QCAR::TrackerManager::getInstance();
  QCAR::ImageTracker* imageTracker =
          static_cast<QCAR::ImageTracker*> (trackerManager.getTracker(
                  QCAR::Tracker::IMAGE_TRACKER));
  // Deactivate current dataset
  imageTracker->deactivateDataSet(imageTracker->getActiveDataSet());

  // Replace the previously tracked target (if any) by the new one.
  if ( this->dataset->hasReachedTrackableLimit() ||
       this->dataset->getNumTrackables()) {
    delete static_cast<MSTargetInfo *>(this->dataset->getTrackable(0)->getUserData());
    this->dataset->destroy(this->dataset->getTrackable(0));
  }

  // Add new trackable source
  QCAR::Trackable *trackable = this->dataset->createTrackable(t);

  // Get its additionnal info from controller, and store it as userdata.
  MSTargetInfo *info = MSController::getNewTargetInfo();
  if (info) {
      trackable->setUserData(info);
  }

  // Reactivate current dataset
  imageTracker->activateDataSet(this->dataset);
}

bool
MSTracker::updateTexture(char *name, MSTexture *tex) {
  bool success = false;
  for (int i = 0; i < this->dataset->getNumTrackables(); ++i) {
    QCAR::Trackable *track = this->dataset->getTrackable(i);
    if (!strcmp(name, track->getName())) {
      MSTargetInfo *inf = static_cast<MSTargetInfo *>(track->getUserData());
      inf->updateTexture(tex);
      track->setUserData(inf);
      success = true;
    }
  }
  return success;
}

void
MSTracker::start() {
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0) imageTracker->start();
}

void
MSTracker::stop() {
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::Tracker* imageTracker = trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER);
    if(imageTracker != 0) imageTracker->stop();
}
