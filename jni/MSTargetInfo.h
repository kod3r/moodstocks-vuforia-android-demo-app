#ifndef MS_TARGETINFO_H
#define MS_TARGETINFO_H

/**********************************************************
 * Class in charge of handling the informations needed to
 * render and place a target: pose matrix, 3D-model and
 * texture.
 **********************************************************/

class MSTexture;
class MSModel;

class MSTargetInfo {
  public:
    /* Build from homography, target dimensions, model and texture.
     * If the model is NULL, a 2D-plan fitting the recognized image will be
     * used to display the chosen texture. */
    MSTargetInfo(float *h, int *d, MSModel *m, MSTexture *t, float *scale);
    ~MSTargetInfo();
    float *getPose();
    MSTexture *getTexture();
    // change the texture currently used to render this target.
    void updateTexture(MSTexture *t);
    MSModel *getModel();
  private:
    /* members */
    float *pose;
    MSTexture *tex;
    MSModel *model;
    /* methods */
    /* Adapt a pose matrix to the fact that OpenGL expects matrix in a
     * column-major fashion, and Vuforia uses a system coordinates where
     * the Y- and Z-axis are inverted compared to the homography Moodstocks
     * SDK returns. */
    static void convert2GLMatrix(float *m);
};

#endif
