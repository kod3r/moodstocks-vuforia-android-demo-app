#include <QCAR/TrackerManager.h>
#include <QCAR/ImageTracker.h>

#include "MSTargetManager.h"
#include "MSController.h"

MSTargetManager::MSTargetManager() :
building(false),
trackableSource(NULL)
{ }

MSTargetManager::~MSTargetManager() {
  this->stop();
}

void
MSTargetManager::start() {
  QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
  QCAR::ImageTracker* imageTracker =
    static_cast<QCAR::ImageTracker*> (trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
  if (imageTracker)
  {
    QCAR::ImageTargetBuilder* targetBuilder = imageTracker->getImageTargetBuilder();
    if (targetBuilder) {
      if (targetBuilder->getFrameQuality() == QCAR::ImageTargetBuilder::FRAME_QUALITY_NONE)
      {
        targetBuilder->startScan();
      }
    }
  }
}

void
MSTargetManager::stop() {
  QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
  QCAR::ImageTracker* imageTracker =
    static_cast<QCAR::ImageTracker*> (trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));
  if (imageTracker)
  {
    QCAR::ImageTargetBuilder* targetBuilder = imageTracker->getImageTargetBuilder();
    if (targetBuilder) {
      if (targetBuilder->getFrameQuality() != QCAR::ImageTargetBuilder::FRAME_QUALITY_NONE)
      {
        targetBuilder->stopScan();
      }
    }
  }
}

void
MSTargetManager::build(char *name) {
  QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
  QCAR::ImageTracker* imageTracker =
    static_cast<QCAR::ImageTracker*>(trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));

  if(imageTracker)
  {
    QCAR::ImageTargetBuilder* targetBuilder = imageTracker->getImageTargetBuilder();
    if(targetBuilder)
    {
      targetBuilder->build(name, MODEL_SIZE*MSController::getFrameRatio());
      this->building = true;
    }
  }
}

bool
MSTargetManager::hasNewTrackableSource( )
{
  if (this->building) {
    QCAR::TrackerManager& trackerManager = QCAR::TrackerManager::getInstance();
    QCAR::ImageTracker* imageTracker =
      static_cast<QCAR::ImageTracker*> (trackerManager.getTracker(QCAR::Tracker::IMAGE_TRACKER));

    QCAR::ImageTargetBuilder* targetBuilder = imageTracker->getImageTargetBuilder();
    QCAR::TrackableSource* newTrackableSource = targetBuilder->getTrackableSource();
    if (newTrackableSource != NULL)
    {
      trackableSource = newTrackableSource;
      building = false;
      return true;
    }
  }
  return false;
}

QCAR::TrackableSource*
MSTargetManager::getNewTrackableSource( )
{
  QCAR::TrackableSource * result = trackableSource;
  trackableSource = NULL;
  return result;
}

