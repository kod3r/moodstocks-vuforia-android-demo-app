#include "MSRenderer.h"
#include "MSTargetInfo.h"
#include "CubeShaders.h"
#include "MSTexture.h"
#include "MSModel.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <math.h>

#include <QCAR/CameraDevice.h>
#include <QCAR/Renderer.h>
#include <QCAR/TrackableResult.h>
#include <QCAR/VideoBackgroundConfig.h>

// Contructor
MSRenderer::MSRenderer() :
shaderProgramID(0),
vertexHandle(0),
normalHandle(0),
textureCoordHandle(0),
mvpMatrixHandle(0),
texSampler2DHandle(0)
{
  // Define clear color
  glClearColor(0.0f, 0.0f, 0.0f, QCAR::requiresAlpha() ? 0.0f : 1.0f);

  // Initialize OpenGL: shaders, attributes.
  shaderProgramID = MSRenderer::createProgramFromBuffer(cubeMeshVertexShader,
                            cubeFragmentShader);
  vertexHandle = glGetAttribLocation(shaderProgramID, "vertexPosition");
  normalHandle = glGetAttribLocation(shaderProgramID, "vertexNormal");
  textureCoordHandle = glGetAttribLocation(shaderProgramID, "vertexTexCoord");
  mvpMatrixHandle = glGetUniformLocation(shaderProgramID, "modelViewProjectionMatrix");
  texSampler2DHandle  = glGetUniformLocation(shaderProgramID, "texSampler2D");
}

void
MSRenderer::renderFrame() {
  // Clear color and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Get the state from QCAR and mark the beginning of a rendering section
  QCAR::State state = QCAR::Renderer::getInstance().begin();

  // Explicitly render the Video Background
  QCAR::Renderer::getInstance().drawVideoBackground();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Did we find any trackables this frame?
  for(int tIdx = 0; tIdx < state.getNumTrackableResults(); tIdx++)
  {
  // Get the trackable:
  const QCAR::TrackableResult* trackableResult = state.getTrackableResult(tIdx);
  QCAR::Matrix44F modelViewMatrix =
    QCAR::Tool::convertPose2GLMatrix(trackableResult->getPose());
  MSRenderer::scalePoseMatrix(MSController::getFrameRatio(),
                              1,
                              1,
                              &modelViewMatrix.data[0]);

  // get the target info
  void *userData = trackableResult->getTrackable().getUserData();
  MSTargetInfo *info = static_cast<MSTargetInfo *>(userData);
  MSTexture *tex = info->getTexture();
  MSModel *model = info->getModel();
  // Bind texture to OpenGL if not done yet
  if (!tex->mHasID) {
    glGenTextures(1, &(tex->mTextureID));
    glBindTexture(GL_TEXTURE_2D, tex->mTextureID);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->mWidth,
                 tex->mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 (GLvoid*) tex->mData);
    tex->mHasID = true;
  }

  MSRenderer::multiplyMatrix(&modelViewMatrix.data[0],
                             info->getPose(),
                             &modelViewMatrix.data[0]);

  QCAR::Matrix44F modelViewProjection;

  MSRenderer::multiplyMatrix(&projectionMatrix.data[0],
                             &modelViewMatrix.data[0] ,
                             &modelViewProjection.data[0]);

  glUseProgram(shaderProgramID);

  glVertexAttribPointer(vertexHandle, 3, GL_FLOAT, GL_FALSE, 0, model->vertices);
  glVertexAttribPointer(normalHandle, 3, GL_FLOAT, GL_FALSE, 0, model->normals);
  glVertexAttribPointer(textureCoordHandle, 2, GL_FLOAT, GL_FALSE, 0, model->texCoords);

  glEnableVertexAttribArray(vertexHandle);
  glEnableVertexAttribArray(normalHandle);
  glEnableVertexAttribArray(textureCoordHandle);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex->mTextureID);
  glUniformMatrix4fv(mvpMatrixHandle, 1, GL_FALSE, (GLfloat*)modelViewProjection.data);
  glUniform1i(texSampler2DHandle, 0);
  glDrawElements(GL_TRIANGLES, 3*model->nFaces, GL_UNSIGNED_SHORT, model->faces);
  }

  glDisable(GL_DEPTH_TEST);

  glDisableVertexAttribArray(vertexHandle);
  glDisableVertexAttribArray(normalHandle);
  glDisableVertexAttribArray(textureCoordHandle);

  glDisable(GL_BLEND);

  QCAR::Renderer::getInstance().end();
}

void
MSRenderer::updateState() {
  setProjectionMatrix();
  configureVideoBackground();
}

void
MSRenderer::setProjectionMatrix() {
  // Cache the projection matrix:
  const QCAR::CameraCalibration& cameraCalibration =
  QCAR::CameraDevice::getInstance().getCameraCalibration();
  projectionMatrix = QCAR::Tool::getProjectionGL(cameraCalibration, 0.04f, 50.0f);
}

void
MSRenderer::configureVideoBackground() {
  // Get the default video mode:
  QCAR::CameraDevice& cameraDevice = QCAR::CameraDevice::getInstance();
  QCAR::VideoMode videoMode = cameraDevice.getVideoMode(CAM_QUALITY);

  // Configure the video background
  QCAR::VideoBackgroundConfig config;
  config.mEnabled = true;
  config.mSynchronous = true;
  config.mPosition.data[0] = 0.0f;
  config.mPosition.data[1] = 0.0f;

  int screenWidth, screenHeight;
  MSController::getScreenSize(&screenWidth, &screenHeight);

  if (MSController::isPortrait())
  {
    config.mSize.data[0] = videoMode.mHeight * (screenHeight / (float)videoMode.mWidth);
    config.mSize.data[1] = screenHeight;

    if (config.mSize.data[0] < screenWidth)
    {
      config.mSize.data[0] = screenWidth;
      config.mSize.data[1] = screenWidth * (videoMode.mWidth  / (float)videoMode.mHeight);
    }
  }
  else
  {
    config.mSize.data[0] = screenWidth;
    config.mSize.data[1] = videoMode.mHeight * (screenWidth
        / (float)videoMode.mWidth);

    if (config.mSize.data[1] < screenHeight)
    {
      config.mSize.data[0] = screenHeight * (videoMode.mWidth / (float)videoMode.mHeight);
      config.mSize.data[1] = screenHeight;
    }
  }

  MSController::setFrameRatio((float)videoMode.mWidth/videoMode.mHeight);

  // Set the config:
  QCAR::Renderer::getInstance().setVideoBackgroundConfig(config);

}

void
MSRenderer::scalePoseMatrix(float x, float y, float z, float* matrix)
{
  // Sanity check
  if (!matrix)
    return;

  // matrix * scale_matrix
  matrix[0]  *= x;
  matrix[1]  *= x;
  matrix[2]  *= x;
  matrix[3]  *= x;

  matrix[4]  *= y;
  matrix[5]  *= y;
  matrix[6]  *= y;
  matrix[7]  *= y;

  matrix[8]  *= z;
  matrix[9]  *= z;
  matrix[10] *= z;
  matrix[11] *= z;
}


void
MSRenderer::multiplyMatrix(float *matrixA, float *matrixB, float *matrixC)
{
  int i, j, k;
  float aTmp[16];

  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 4; j++)
    {
      aTmp[j * 4 + i] = 0.0;

      for (k = 0; k < 4; k++)
        aTmp[j * 4 + i] += matrixA[k * 4 + i] * matrixB[j * 4 + k];
    }
  }

  for (i = 0; i < 16; i++)
    matrixC[i] = aTmp[i];
}

unsigned int
MSRenderer::initShader(unsigned int shaderType, const char* source)
{
  GLuint shader = glCreateShader((GLenum)shaderType);
  if (shader)
  {
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
      GLint infoLen = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen)
      {
        char* buf = (char*) malloc(infoLen);
        if (buf)
        {
          glGetShaderInfoLog(shader, infoLen, NULL, buf);
          free(buf);
        }
        glDeleteShader(shader);
        shader = 0;
      }
    }
  }
  return shader;
}


unsigned int
MSRenderer::createProgramFromBuffer(const char* vertexShaderBuffer,
                   const char* fragmentShaderBuffer)
{
  GLuint vertexShader = initShader(GL_VERTEX_SHADER, vertexShaderBuffer);
  if (!vertexShader)
    return 0;

  GLuint fragmentShader = initShader(GL_FRAGMENT_SHADER,
                    fragmentShaderBuffer);
  if (!fragmentShader)
    return 0;

  GLuint program = glCreateProgram();
  if (program)
  {
    glAttachShader(program, vertexShader);

    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus != GL_TRUE)
    {
      GLint bufLength = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
      if (bufLength)
      {
        char* buf = (char*) malloc(bufLength);
        if (buf)
        {
          glGetProgramInfoLog(program, bufLength, NULL, buf);
          free(buf);
        }
      }
      glDeleteProgram(program);
      program = 0;
    }
  }
  return program;
}
