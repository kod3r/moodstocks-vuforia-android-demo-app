#ifndef MS_RENDERER_H
#define MS_RENDERER_H

#include "MSController.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <QCAR/Tool.h>

/**********************************************
 * Class in charge of rendering and placing
 * the 3D model for the target currently being
 * tracked.
 **********************************************/

class MSRenderer {

  public:
    MSRenderer();
    void renderFrame();
    // Called whenever the application state (orientation, screen) changes.
    void updateState();

  private:
    unsigned int shaderProgramID;
    GLint vertexHandle;
    GLint normalHandle;
    GLint textureCoordHandle;
    GLint mvpMatrixHandle;
    GLint texSampler2DHandle;
    QCAR::Matrix44F projectionMatrix;
    // configuration methods
    void setProjectionMatrix();
    void configureVideoBackground();
    // Utility methods:
    static void scalePoseMatrix(float x, float y, float z, float* nMatrix = NULL);
    static void multiplyMatrix(float *matrixA, float *matrixB, float *matrixC);
    static unsigned int initShader(unsigned int shaderType, const char* source);
    static unsigned int createProgramFromBuffer(const char* vertexShaderBuffer,
                                                const char* fragmentShaderBuffer);
};

#endif
