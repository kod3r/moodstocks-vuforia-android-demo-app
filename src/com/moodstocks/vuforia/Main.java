package com.moodstocks.vuforia;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.pm.ActivityInfo;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.RelativeLayout;

import com.moodstocks.android.MoodstocksError;
import com.moodstocks.android.Result;
import com.qualcomm.QCAR.QCAR;


/** The main activity */
public class Main extends Activity implements ScannerManager.Listener, View.OnClickListener
{
  
  /** The GLView to use for display */
  private GLView mGlView;
  /** The {@link Renderer} to use for display */
  private Renderer mRenderer;
  
  /** The settings button */
  private ImageButton button;

  /** The {@link Initializer} to use */
  private Initializer initializer;
  /** The initializer parameter: request OpenGLES 2 */
  private int mQCARFlags = QCAR.GL_20;

  /** The {@link ScannerManager} to use */
  private ScannerManager scan_manager;
  /** The {@link ResultManager} to use */
  private ResultManager manager;
  
  /** Byte buffer to store the camera frames data */
  private byte[] imgData = null;
  /** Stores the camera frames width */
  private int imgWidth = -1;
  /** Stores the camera frames height */
  private int imgHeight = -1;
  /** Stores the camera frames stride, in bytes per row */
  private int imgStride = -1;

  /** Static block to load native libraries */
  static
  {
    System.loadLibrary("QCAR");
    System.loadLibrary("MoodstocksVuforia");
  }
  
  /** Native function to ask for the next available frame to be delivered via onPreviewFrame Callback */
  private native void requireNewFrame();

  /** Native function to switch <b>to</b> target building mode */
  public native void startBuildingMode();
  /** Native function to switch <b>from</b> target building mode */
  public native void stopBuildingMode();

  /**
   * Native method to try building a new tracking target when in the right mode.
   * @param name a unique identifier for this target
   * @param h the homography as returned by {@link com.moodstocks.android.Result#getHomography()}
   * @param d the reference image dimensions as returned by {@link com.moodstocks.android.Result #getDimensions()}
   * @param m the {@link Model} to display, or {@code null} to display a plane
   * @param t the {@link Texture} to display on the 3D model.
   * @param scale the {x, y, z} scaling to apply to the 3D model.
   * @return false if the current view ange does not correspond to the target being held 
   * parallel to the device, true otherwise.
   */
  public native boolean tryBuild(String name, float[] h, int[] d, Model m, Texture t, float[] scale);
  
  /**
   * Changes the texture applied to a target
   * @param name the unique identifier of the target
   * @param t the new {@link Texture} to apply
   * @return true if the target exists, false otherwise.
   */
  public native boolean updateTexture(String name, Texture t);


  protected void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
    
    // Allow the 4 rotations
    setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_FULL_SENSOR);

    // Keep the screen on while in this Activity
    getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                         WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

    // Initialize the application
    initializer = new Initializer();
    initializer.init();

    // Initialize Moodstocks components
    try {
      scan_manager = new ScannerManager(this, this);
    } catch (MoodstocksError e) {
      e.log();
    }
    manager = new ResultManager(this);
  }

  protected void onResume()
  {
    super.onResume();

    // QCAR-specific resume operation
    QCAR.onResume();

    // Start Moodstocks Scanner
    scan_manager.setExtras(Result.Extra.HOMOGRAPHY | Result.Extra.DIMENSIONS);
    scan_manager.resume();
  }


  protected void onPause()
  {
    super.onPause();

    stopCamera();

    // QCAR-specific pause operation
    QCAR.onPause();

    // Pause Moodstocks Scanner
    scan_manager.pause();
  }


  protected void onDestroy()
  {
    super.onDestroy();

    // Cancel potentially running initializer
    initializer.stopInit();
    
    // De-initialize the app
    initializer.deInit();

    // Close Moodstocks Scanner
    scan_manager.close();
  }
  
  /******************************
   * Camera and focus management
   ******************************/
  
  /** Store the current state of the camera */
  private boolean cameraRunning = false;
  
  /** The different possible camera focus modes */
  @SuppressWarnings("unused")
  private static final class FocusMode {
    /** Default focus mode */
    private static final int NORMAL = 0;
    /** Continuous autofocus mode */
    private static final int CONTINUOUS_AUTO = 1;
    /** Focus set to infinity */
    private static final int INFINITY = 2;
    /** Focus mode for close-up focus */
    private static final int MACRO = 3;
  }
  
  /** Native method to start the camera */
  private native boolean startCameraNative();
  /** Native method to stop the camera */
  private native void stopCameraNative();
  
  /** Native method to set the focus mode among {@link FocusMode} */
  private native boolean setFocusMode(int mode);
  
  /** Initialize the preview and start the camera */
  private boolean startCamera() {
    if (!cameraRunning) {
      if (!startCameraNative()) {
        return false;
      }
      if (!setFocusMode(FocusMode.CONTINUOUS_AUTO)) {
        setFocusMode(FocusMode.NORMAL);
      }
      if (mGlView != null)
      {
        mGlView.setVisibility(View.VISIBLE);
        mGlView.onResume();
      }
      cameraRunning = true;
    }
    return true;
  }
  
  /** Deinitialize the preview and stop the camera */
  private void stopCamera() {
    if (cameraRunning) {
      stopCameraNative();
      if (mGlView != null)
      {
        mGlView.setVisibility(View.INVISIBLE);
        mGlView.onPause();
      }
      cameraRunning = false;
    }
  }
  
  /** Called from native code when a new camera frame is available */
  private void onPreviewFrame() {
    scan_manager.onPreviewFrame(imgData, imgWidth, imgHeight, imgStride);
  }
  
  /** {@link ScannerManager.Listener} implementation */
  @Override
  public void onScanComplete(Result result) {
    manager.newResult(result);
    requireNewFrame();
  }
  
  /** {@link ScannerManager.Listener} implementation */
  @Override
  public void onScanFailed(MoodstocksError e) {
    e.log();
    requireNewFrame();
  }
  
  
  /** Inner class to manage the initialization of all components asynchronously */
  private class Initializer extends AsyncTask<Void, Integer, Boolean>
  {
    private int mProgressValue = -1;
    private Object mShutdownLock = null;
    
    public Initializer() {
      this.mShutdownLock = new Object();
    }
    
    public void init() {
      this.execute();
    }
    
    public void stopInit() {
      if (this.getStatus() != Status.FINISHED) {
        this.cancel(true);
      }
    }
    
    public void deInit() {
      synchronized (mShutdownLock) {
        deInitNative();
        QCAR.deinit();
      }
    }
    
    private void postInit() {
      initNative();
      initGL();
      initUI();
    }

    protected Boolean doInBackground(Void... params)
    {
      // Prevent the onDestroy() method to overlap with initialization:
      synchronized (mShutdownLock)
      {
        QCAR.setInitParameters(Main.this, mQCARFlags);

        do
        {
          // Must be called repeatedly to advance step-by-step.
          // Return value is either -1 if an error occurred, or a percentage
          // of progression.
          mProgressValue = QCAR.init();

          // Check that: the task has not been cancelled, that no error occurred,
          // and that the initialization is not finished.
        } while (!isCancelled() && mProgressValue >= 0 && mProgressValue < 100);

        return (mProgressValue > 0);
      }
    }

    protected void onPostExecute(Boolean success)
    {
      String logMessage = null;
      if (!success) {
        logMessage = (mProgressValue == QCAR.INIT_DEVICE_NOT_SUPPORTED) ?
                     "Failed to initialize QCAR because this device is not supported." :
                     "Failed to initialize QCAR.";
      }
      else {
        // Run post-init tasks.
        postInit();
        // start camera
        if (startCamera()) {
          // require the first frame
          requireNewFrame();
        }
        else {
          logMessage = "The camera has a too low resolution to be supported.";
        }
      }
      if (logMessage != null) {
        // Display error, and exit.
        AlertDialog dialogError = new AlertDialog.Builder(Main.this).create();
        dialogError.setButton(DialogInterface.BUTTON_POSITIVE, "Close",
            new DialogInterface.OnClickListener() {
              public void onClick(DialogInterface dialog, int which)
              {
                System.exit(1);
              }
            }
        );
        dialogError.setMessage(logMessage);
        dialogError.show();
      }
    }
  }
  
  /** Initializes the native part of the code */
  public native void initNative();
  /** Deinitializes the native part of the code */
  public native void deInitNative();
  
  /** 
   * Method to initialize the {@link Renderer} and the GLView.
   * Must be called from GL thread!
   */
  private void initGL() {
    mGlView = new GLView(this);
    mGlView.init(QCAR.requiresAlpha());

    mRenderer = new Renderer(this);
    mGlView.setRenderer(mRenderer);
    
    mRenderer.mIsActive = true;
    
    addContentView(mGlView, new LayoutParams(LayoutParams.MATCH_PARENT,
                                             LayoutParams.MATCH_PARENT));
    
  }
  
  /** Initializes the UI */
  private void initUI() {
    LayoutInflater inflater = LayoutInflater.from(this);
    RelativeLayout layout = (RelativeLayout) inflater.inflate(R.layout.main, null); 
    addContentView(layout, new LayoutParams(LayoutParams.MATCH_PARENT,
                                            LayoutParams.MATCH_PARENT));
    this.button = (ImageButton) findViewById(R.id.settings_btn);
    this.button.setOnClickListener(this);
  }
  
  /** Settings button callback */
  @Override
  public void onClick(View v) {
    if (v.getId() == R.id.settings_btn) {
      AlertDialog.Builder builder = new AlertDialog.Builder(this);
      builder.setTitle("Settings");
      builder.setSingleChoiceItems(new String[] {"Display ID", "Display borders"}, manager.getMode(),
          new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
              manager.setMode(which);
              dialog.dismiss();
            }
          }
      );
      builder.show();
    }
    
  }
}
