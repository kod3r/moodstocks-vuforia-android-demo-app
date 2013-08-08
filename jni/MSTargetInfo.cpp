#include "MSTargetInfo.h"
#include "MSController.h"
#include "MSTexture.h"
#include "MSModel.h"

#include <math.h>
#include <string.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <QCAR/QCAR.h>

MSTargetInfo::MSTargetInfo(float *h, int *d, MSModel *m, MSTexture *t,
                           float *scale) {
  pose = (float *)calloc(16,sizeof(float));

  // set to 3D:
  for (int i = 0; i < 2; ++i) {
    pose[4*i] = h[3*i];
    pose[4*i+1] = h[3*i+1];
    pose[4*i+3] = h[3*i+2];
  }
  pose[12] = h[6];
  pose[13] = h[7];
  pose[15] = h[8];

  // compensate the recognized image
  // dimensions to get an anisotropic matrix.
  float r = ((float)d[0])/d[1];
  if (r < 1) {
    for (int i = 0; i < 4; ++i) {
      pose[4*i+1] *= r;
    }
  }
  else {
    for (int i = 0; i < 4; ++i) {
      pose[4*i] /= r;
    }
  }

  // Get Z-axis scale: project points (0,1), (0,-1), (1,0) and (-1, 0),
  // and take mean of distances to projection of (0,0)
  float tmp = 0;
  for (int i = -1; i < 2; i += 2) {
    float xa = (i*h[0]+h[2])/(i*h[6]+1)-h[2];
    float ya = (i*h[3]+h[5])/(i*h[6]+1)-h[5];
    float xb = (i*h[1]+h[2])/(i*h[7]+1)-h[2];
    float yb = (i*h[4]+h[5])/(i*h[7]+1)-h[5];
    tmp += sqrt(xa*xa+ya*ya);
    tmp += sqrt(xb*xb+yb*yb);
  }
  pose[10] = tmp / 4.0;

  // rescale if non-null.
  if (scale) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 4; ++j) {
        pose[4*j+i] *= scale[i];
      }
    }
  }

  if (m) {
    // store model
    model = m;
  }
  else {
    // use a plane
    model = MSModel::getPlane();
  }

  // convert to GL format:
  convert2GLMatrix(pose);

  // store texture
  tex = t;
}

MSTargetInfo::~MSTargetInfo() {
  if (pose) free(pose);
  if (tex) delete tex;
  if (model) delete model;
}

float *
MSTargetInfo::getPose() {
  return pose;
}

MSTexture *
MSTargetInfo::getTexture() {
  return tex;
}

void
MSTargetInfo::updateTexture(MSTexture *t) {
  delete tex;
  tex = t;
}

MSModel *
MSTargetInfo::getModel() {
  return model;
}

void
MSTargetInfo::convert2GLMatrix(float *m) {
  float r[16];
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      r[4*i+j] = m[4*j+i];
      if ((i == 1 || i == 2) ^ (j == 1 || j == 2)) {
        r[4*i+j] *= -1;
      }
    }
  }
  memcpy(m, r, 16*sizeof(float));
}
