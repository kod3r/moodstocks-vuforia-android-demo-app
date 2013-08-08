package com.moodstocks.vuforia;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.res.Configuration;
import android.opengl.GLSurfaceView;

import com.qualcomm.QCAR.QCAR;

/**
 * Class handling the OpengGL display
 */
public class Renderer implements GLSurfaceView.Renderer {
  
  public boolean mIsActive = false;
  private Main parent;
  
  /**
   * COnstructor
   * @param parent the parent {@link Main} activity.
   */
  public Renderer(Main parent) {
    super();
    this.parent = parent;
  }

  /** Native function for initializing the renderer. */
  public native void initRendering();

  /** Native function for updating the way the frames are rendered.
   * <p>
   * This method allows to adapt the display to the screen size and orientation.
   * @param width the screen width
   * @param height the screen height
   * @param portrait true if in portrait mode, false if in landscape mode
   */
  public native void updateRendering(int width, int height, boolean portrait);
  
  /**
   * <i>GLSurfaceView callback</i>
   */
  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    initRendering();
    QCAR.onSurfaceCreated();
  }

  /**
   * <i>GLSurfaceView callback</i>
   */
  public void onSurfaceChanged(GL10 gl, int width, int height) {    
    // get screen orientation
    Configuration config = parent.getResources().getConfiguration(); 
    boolean portrait = true;
    switch (config.orientation) {
    case Configuration.ORIENTATION_LANDSCAPE:
      portrait = false;
      break;
    default:
      break;
    }
    updateRendering(width, height, portrait);
    QCAR.onSurfaceChanged(width, height);
  }

  /**
   * Native rendering method
   */
  private native void renderFrame();

  /**
   * Called when a frame needs to be rendered
   * @param gl the OpenGL context
   */
  public void onDrawFrame(GL10 gl) {
    if (!mIsActive)
      return;
    renderFrame();
  }
}
