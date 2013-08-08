package com.moodstocks.vuforia;

import android.os.AsyncTask;

import com.moodstocks.android.Result;

/**
 * Class handling the results as the Moodstocks SDK provides them.
 */
public class ResultManager {
  private Result _expected; // a result that we've just found, and thanks to which we started building mode.
  private Texture _default; // default texture
  private Result _tracked;  // result currently being tracked, if any. Allows not to track the same result several times.
  private Main parent;
  private int _mode = Mode.ID;
  
  /**
   * The two possible display modes
   */
  public static final class Mode {
    /**
     * Mode in which the ID of the recognized image is displayed 
     * on a semi-transparent background, on top of the tracked target.
     */
    public static final int ID = 0;
    /**
     * Mode in which only the borders of the tracked target are displayed. 
     */
    public static final int BORDERS = 1;
  }
  
  /**
   * Constructor
   * @param parent the parent {@link Main} object
   */
  public ResultManager(Main parent) {
    this._expected = null;
    this._default = Texture.transparentTexture();
    this._tracked = null;
    this.parent = parent;
  }
  
  /**
   * Handles a new {@link com.moodstocks.android.Result}
   * <p>
   * Switching to Build mode and creating a new target cannot be done
   * at the same time, as the native part of the code is scheduled by the
   * camera frames. The idea here is that if we find a new result, we 
   * switch to build mode, then if the next frame still matches this new
   * result, the target is built.
   * @param result the result 
   */
  public void newResult(Result result) {
    // first case: result is null
    if (result == null) {
      // if we were currently expecting a result, reset it and exit building mode.
      if (_expected != null)  {
        _expected = null;
        parent.stopBuildingMode();
      }
      return;
    }
    // second case: result is non-null
    // is it the result currently being tracked? If yes, ignore.
    if (result.equals(_tracked))
      return;
    // non null result while we were expecting one.
    if (_expected != null) {
      // it's the second time we meet this result, so we're already in building mode
      if (result.equals(_expected)) {
        // try building:
        float homog[] = new float[9];
        result.getHomography().getValues(homog);
        // Reminder: for now we can't display 3D-models, we choose to display the texture on a simple plane.
        // Scale this plane so it fits the whole reference image:
        int dims[] = result.getDimensions();
        float scale[] = {1.0f, 1.0f, 1.0f};
        if (dims[0] > dims[1]) scale[0] *= ((float)dims[0])/dims[1];
        if (dims[0] < dims[1]) scale[1] *= ((float)dims[1])/dims[0];
        if (parent.tryBuild(result.getValue(), homog, dims, null, _default, scale)) {
          new TextureUpdater().execute(result);
          _tracked = _expected;
          _expected = null;
        }
      }
      // it's a new result: replace previous expected result.
      else {
        _expected = result;
        parent.startBuildingMode();    
      }
    }
    // it's a new result: store it as expected result.
    else {
      _expected = result;
      parent.startBuildingMode();
    }
  }
  
  /**
   * Switches between the different display modes
   * @param m the {@link Mode} to use.
   */
  public void setMode(int m) {
    if (m != _mode) {
      if (_tracked != null) {
        new TextureUpdater().execute(_tracked);
      }
      _mode = m;
    }
  }
  
  /**
   * Get the current display mode
   * @return the {@link Mode}
   */
  public int getMode() {
    return _mode;
  }
  
  /**
   * Chooses the string to display if in {@link Mode#ID} mode.
   * <p>
   * Feel free to modify this function in order to display the
   * text you want for each result. Here we simply take the ID as-is.
   * Useful for example if your ID is base64-encoded.   
   * @param r the result from which to extract string to display
   * @return the string to display
   */
  public static String stringToDisplay(Result r) {
    return r.getValue();
  }
  
  /**
   * Asynchronously creates the {@link Texture} to display, according to 
   * the current {@link Mode}.
   */
  private class TextureUpdater extends AsyncTask<Result, Void, Texture> {
    
    private Result r = null;
    
    @Override
    protected Texture doInBackground(Result...rs) {
      r = rs[0];
      if (_mode == Mode.ID)
        return Texture.textureFromText(stringToDisplay(r), r.getDimensions());
      else
        return Texture.borders(r.getDimensions());
    }
    
    @Override
    protected void onPostExecute(Texture t) {
      parent.updateTexture(r.getValue(), t);
    }
    
  }

}