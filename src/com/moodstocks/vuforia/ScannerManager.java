package com.moodstocks.vuforia;

import java.lang.ref.WeakReference;

import com.moodstocks.android.Image;
import com.moodstocks.android.MoodstocksError;
import com.moodstocks.android.core.OrientationListener;
import com.moodstocks.android.Result;
import com.moodstocks.android.Scanner;
import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;

/**
 * This class implements the same logic as 
 * {@link com.moodstocks.android.ScannerSession}, but
 * does not manage the camera itself, as we expect the 
 * Vuforia SDK to provide us the frames.
 */
public class ScannerManager extends Handler {
  public static final String TAG = "ScannerManager";
  private Scanner scanner = null;
  private WeakReference<Listener> listener;
  private WorkerThread worker;

  private boolean running = false;
  
  private int img_w;
  private int img_h;
  private int img_stride;

  /** default options: cache image recognition only. */
  protected int options = Result.Type.IMAGE;
  /** default geometrical extras: None */
  protected int extras = Result.Extra.NONE;

  /**
   * If true, the camera frames are re-oriented according to the current device orientation, e.g if the
   * device is in landscape mode, the frame is rotated by 90 degrees to make sure the scanner input
   * image reflects what the user sees on screen.
   * <p>
   * If false (default), the device orientation is ignored, and thus the frame is *always* handled with
   * portrait orientation.
   */
  public boolean useDeviceOrientation = false;

  /**
   * Interface that must be implemented to receive callbacks from the ScannerSession.
   */
  public static interface Listener {
    /**
     * Notifies the listener that a scan has ended.
     * @param result the {@link com.moodstocks.android.Result} if any, null otherwise.
     */
    public void onScanComplete(Result result);
    /**
     * Notified the listener that a scan has failed.
     * @param error the {@link com;moodstocks.android.MoodstocksError} that caused the 
     * scan to fail.
     */
    public void onScanFailed(MoodstocksError error);
  }

  /**
   * Constructor.
   * @param parent the caller {@link Activity}
   * @param listener the {@link Listener} to notify
   * @throws MoodstocksError if any error occurred.
   */
  public ScannerManager(Activity parent, Listener listener) 
      throws MoodstocksError {
    this.listener = new WeakReference<Listener>(listener);
    this.scanner = Scanner.get();
    this.worker = new WorkerThread(this);
    OrientationListener.init(parent);
    OrientationListener.get().enable();
    worker.start();
  }

  /**
   * Sets the operations you want the scanner to perform.
   * @param options the list of bitwise-OR separated {@link Result.Type} flags
   *                specifying the operation to perform among image recognition
   *                and various formats of barcode decoding.
   */
  public void setOptions(int options) {
    this.options = options;
  }

  /**
   * Sets the extra geometrical information to compute when a result is found.
   * @param extras the list of bitwise-OR separated {@link Result.Extra} flags
   *               specifying which geometrical information to compute.
   */
  public void setExtras(int extras) {
    this.extras = extras;
  }

  /**
   * Starts or restarts scanning the camera frames.
   * @return false if the session was already running.
   */
  public boolean resume() {
    if (!running) {
      worker.reset();
      running = true;
      return true;
    }
    return false;
  }

  /**
   * Stops scanning the camera frames.
   * @return false if the session was already paused.
   */
  public boolean pause() {
    if (running) {
      running = false;
      return true;
    }
    return false;
  }

  /**
   * Closes the ScannerManager
   * <p>
   * Must be called before exiting the enclosing {@link Activity}.
   */
  public void close() {
    pause();
    OrientationListener.get().disable();
    finishWorker(500L);
  }

  /**
   * Closes the worker thread.
   * @param t the time in milliseconds allowed for the thread to end.
   */
  private void finishWorker(long t) {
    worker.getHandler().obtainMessage(MsgCode.QUIT).sendToTarget();
    try {
      worker.join(t);
    } catch (InterruptedException e) {

    }
  }
  
  /**
   * Used to receive the camera frames provided by the Vuforia SDK
   * @param data the frame data
   * @param w the frame width
   * @param h the frame height
   * @param stride the frame stride, in bytes per row
   */
  public void onPreviewFrame(byte[] data, int w, int h, int stride) {
    if (running) {
      this.img_w = w;
      this.img_h = h;
      this.img_stride = stride;
      worker.getHandler().obtainMessage(MsgCode.SCAN, data).sendToTarget();
    }
  }
  
  /**
   * <i>Internal message passing utility</i>
   */
  @Override
  public void handleMessage(Message msg) {
    Listener l = listener.get();

    switch(msg.what) {

      case MsgCode.SUCCESS:
        if (l != null)
          l.onScanComplete((Result)msg.obj);
        break;

      case MsgCode.FAILED:
        if (l != null)
          l.onScanFailed((MoodstocksError)msg.obj);
        break;

      default:
        break;

    }

  }
  
  /**
   * <i>Background thread processing the camera frames.</i>
   */
  private class WorkerThread extends Thread {

    private Handler handler;
    private ScannerManager manager;
    // locking values:
    private Result _result = null;
    private int _losts = 0;

    private WorkerThread(ScannerManager manager) {
      super();
      this.manager = manager;
    }

    @Override
    public void run() {
      Looper.prepare();
      handler = new WorkerHandler(this);
      Looper.loop();
    }

    private Handler getHandler() {
      return handler;
    }

    private void reset() {
      _result = null;
      _losts = 0;
    }

    private void quit() {
      Looper.myLooper().quit();
    }

    private void scan(byte[] data) {
      Result result = null;
      MoodstocksError error = null;
      int ori = OrientationListener.Orientation.UP;
      try {
        if (useDeviceOrientation)
          ori = OrientationListener.get().getOrientation();
        result = scan(new Image(data, img_w, img_h, img_stride, ori));
      } catch (MoodstocksError e) {
        error = e;
      }
      if (error != null) {
        manager.obtainMessage(MsgCode.FAILED, error).sendToTarget();
      }
      else {
        manager.obtainMessage(MsgCode.SUCCESS, result).sendToTarget();
      }
    }

    /* Performs a search in the local cache, as well as
     * barcode decoding, according to the options previously set.
     */
    private Result scan(Image qry)
        throws MoodstocksError {

      qry.retain();
      Result result = null;

      //----------
      // LOCKING
      //----------
      try {
        Result rlock = null;
        boolean lock = false;
        if (_result != null && _losts < 2) {
          int found = 0;
          switch (_result.getType()) {
            case Result.Type.IMAGE:
              rlock = scanner.match(qry, _result, extras);
              found = 1;
              break;
            case Result.Type.QRCODE:
              rlock = scanner.decode(qry, Result.Type.QRCODE, extras);
              found = 1;
              break;
            case Result.Type.DATAMATRIX:
              rlock = scanner.decode(qry, Result.Type.DATAMATRIX, extras);
              found = 1;
              break;
            default:
              break;
          }

          if (found == 1) {
            if (rlock != null) {
              found = rlock.getValue().equals(_result.getValue()) ? 1 : -1;
            }
            else {
              found = -1;
            }
          }


          if (found == 1) {
            lock = true;
            _losts = 0;
          }
          else if (found == -1) {
            _losts++;
            lock = (_losts >= 2) ? false : true;
          }
        }
        if (lock) {
          result = rlock;
        }
      } catch (MoodstocksError e) {
        e.log();
      }

      //---------------
      // IMAGE SEARCH
      //---------------
      try {
        if (result == null && ((options & Result.Type.IMAGE) != 0)) {
          result = scanner.search(qry, extras);
          if (result != null) {
            _losts = 0;
          }
        }
      } catch (MoodstocksError e) {
        if (e.getErrorCode() != MoodstocksError.Code.EMPTY)
          throw e;
      }


      //-------------------
      // BARCODE DECODING
      //-------------------
      if (result == null &&
         ( (options & (Result.Type.QRCODE|Result.Type.EAN13|
                       Result.Type.EAN8|Result.Type.DATAMATRIX) ) != 0)) {
        result = scanner.decode(qry, options, extras);
        if (result != null) {
          _losts = 0;
        }
      }

      //----------------
      // Locking update
      //---------------
      _result = result;

      qry.release();
      return result;
    }

  }

  /**
   * <i>Internal message passing utility</i>
   */
  private static class WorkerHandler extends Handler {

    private final WeakReference<WorkerThread> worker;

    private WorkerHandler(WorkerThread worker) {
      super();
      this.worker = new WeakReference<WorkerThread>(worker);
    }

    @Override
    public void handleMessage(Message msg) {

      WorkerThread w = worker.get();

      if (w != null) {
        switch(msg.what) {

          case MsgCode.SCAN:
            w.scan((byte[])msg.obj);
            break;

          case MsgCode.QUIT:
            w.quit();
            break;

          default:
            break;

        }
      }
    }
  }

  /**
   * <i>Internal message passing codes</i>
   */
  protected static final class MsgCode {
    public static final int SCAN = 0;
    public static final int QUIT = 2;
    public static final int SUCCESS = 3;
    public static final int FAILED = 4;
  }

}
