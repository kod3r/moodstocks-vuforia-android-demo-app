package com.moodstocks.vuforia;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;

/**
 * Implementation of a SurfaceView handling OpenGL drawing.
 */
public class GLView extends GLSurfaceView
{

  private static final int DEPTH_SIZE = 16;
  private static final int STENCIL_SIZE = 0;

  public GLView(Context context) {
    super(context);
  }

  public void init(boolean translucent)
  {
    if (translucent) {
      this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
    }

    setEGLContextFactory(new ContextFactory());

    setEGLConfigChooser( translucent ?
                         new ConfigChooser(8, 8, 8, 8, DEPTH_SIZE, STENCIL_SIZE) :
                         new ConfigChooser(5, 6, 5, 0, DEPTH_SIZE, STENCIL_SIZE) );
  }


  private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
    
    private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

    public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
      int[] attributes = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
      return egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attributes);
    }

    public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
      egl.eglDestroyContext(display, context);
    }
    
  }

  private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser {
    
    protected int redSize;
    protected int greenSize;
    protected int blueSize;
    protected int alphaSize;
    protected int depthSize;
    protected int stencilSize;
    private int[] mValue = new int[1];

    public ConfigChooser(int r, int g, int b, int a, int depth, int stencil) {
      redSize = r;
      greenSize = g;
      blueSize = b;
      alphaSize = a;
      depthSize = depth;
      stencilSize = stencil;
    }

    private EGLConfig getMatchingConfig(EGL10 egl, EGLDisplay display, int[] configAttribs) {
      int[] num_config = new int[1];
      egl.eglChooseConfig(display, configAttribs, null, 0, num_config);

      int numConfigs = num_config[0];
      if (numConfigs <= 0)
        throw new IllegalArgumentException("No matching EGL configs");

      EGLConfig[] configs = new EGLConfig[numConfigs];
      egl.eglChooseConfig(display, configAttribs, configs, numConfigs, num_config);

      return chooseConfig(egl, display, configs);
    }


    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
      final int EGL_OPENGL_ES2_BIT = 0x0004;

      final int[] attributes = {
        EGL10.EGL_RED_SIZE, 4,
        EGL10.EGL_GREEN_SIZE, 4,
        EGL10.EGL_BLUE_SIZE, 4,
        EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL10.EGL_NONE
      };

      return getMatchingConfig(egl, display, attributes);
    }


    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display, EGLConfig[] configs) {
      for(EGLConfig config : configs) {
        int d = findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE, 0);
        int s = findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE, 0);

        if (d < depthSize || s < stencilSize)
          continue;

        int r = findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE, 0);
        int g = findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0);
        int b = findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0);
        int a = findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0);

        if (r == redSize &&
            g == greenSize &&
            b == blueSize &&
            a == alphaSize)
          return config;
      }

      return null;
    }


    private int findConfigAttrib(EGL10 egl, EGLDisplay display, EGLConfig config, int attribute, int defaultValue) {

      if (egl.eglGetConfigAttrib(display, config, attribute, mValue))
        return mValue[0];

      return defaultValue;
    }
  }

}
