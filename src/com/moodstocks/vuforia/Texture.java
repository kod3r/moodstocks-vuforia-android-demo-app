package com.moodstocks.vuforia;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Bitmap.Config;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.text.Layout;
import android.text.StaticLayout;
import android.text.TextPaint;

/**
 * Class representing a Texture to apply on a 3D model.
 */
public class Texture
{   
    /**
     * These fields are actually called from JNI, this is why
     * we suppress the "unused" warnings
     */
    @SuppressWarnings("unused")
    private int mWidth;      /// The width of the texture.
    @SuppressWarnings("unused")
    private int mHeight;     /// The height of the texture.
    @SuppressWarnings("unused")
    private int mChannels;   /// The number of channels.
    @SuppressWarnings("unused")
    private byte[] mData;    /// The pixel data.
    
    /**
     * Create a new Texture made of a semi-transparent background and a text.
     * @param s the text to write on the texture
     * @param d the final expected dimensions of the texture
     * @return the texture
     */
    public static Texture textureFromText(String s, int[] d)
    {
        Bitmap bmp = bitmapFromText(s, d);
        return textureFromBitmap(bmp);

    }
    
    /**
     * Creates a fully transparent texture, used as transient texture.
     * @return
     */
    public static Texture transparentTexture() {
      Bitmap bmp = Bitmap.createBitmap(256, 256, Bitmap.Config.ARGB_8888);
      bmp.eraseColor(Color.TRANSPARENT);
      return textureFromBitmap(bmp);
    }
    
    /**
     * Creates a transparent texture with green borders
     * @param dims the expected dimensions of the texure
     * @return the texture
     */
    public static Texture borders(int[] dims) {
      Bitmap tmp = Bitmap.createBitmap(dims[0], dims[1], Bitmap.Config.ARGB_8888);
      tmp.eraseColor(Color.TRANSPARENT);
      
      Paint p = new Paint();
      p.setColor(Color.GREEN);
      p.setStrokeWidth(10);
      p.setAntiAlias(true);
      p.setStyle(Style.STROKE);
      
      Canvas c = new Canvas(tmp);
      c.drawRect(5, 5, dims[0]-5, dims[1]-5, p);
      
      Bitmap bmp = Bitmap.createScaledBitmap(tmp, 512, 512, true);
      return textureFromBitmap(bmp);
    }
    
    /**
     * 
     * @param bmp
     * @return
     */
    private static Texture textureFromBitmap(Bitmap bmp) {
      int w = bmp.getWidth();
      int h = bmp.getHeight();
      int[] data = new int[w*h];
      bmp.getPixels(data, 0, w, 0, 0, w, h);

      // Convert:
      byte[] dataBytes = new byte[4*w*h];
      for (int p = 0; p < w*h; ++p)
      {
          int c = data[p];
          dataBytes[p * 4]        = (byte)Color.red(c);    // R
          dataBytes[p * 4 + 1]    = (byte)Color.green(c);  // G
          dataBytes[p * 4 + 2]    = (byte)Color.blue(c);   // B
          dataBytes[p * 4 + 3]    = (byte)Color.alpha(c);  // A
      }

      Texture texture = new Texture();
      texture.mWidth      = w;
      texture.mHeight     = h;
      texture.mChannels   = 4;
      texture.mData       = dataBytes;

      return texture;
    }

    private static Bitmap bitmapFromText(String s, int[] d) {
      int bw = 512;
      int bh = 512;
      if (d[0] > d[1]) {
        bw = (int)(512*(float)d[0]/d[1]);
      }
      else {
        bh = (int)(512*(float)d[1]/d[0]);
      }

      Bitmap tmp = Bitmap.createBitmap(bw, bh, Config.ARGB_8888);
      Canvas c = new Canvas(tmp);

      // paint background half-transparent green
      c.drawColor(Color.parseColor("#AA00FF00"));

      // prepare textPaint
      TextPaint p = new TextPaint();
      p.setAntiAlias(true);
      p.setColor(Color.BLACK);
      p.setStyle(Style.FILL);
      p.setTextSize(60);
      p.setShadowLayer(4.0f, 2.0f, 2.0f, Color.DKGRAY);

      // Draw
      StaticLayout l = new StaticLayout(s, p, bw, Layout.Alignment.ALIGN_CENTER, 1.0f, 0.0f, false);
      int h = l.getHeight();
      c.translate(0, bh/2-h/2);
      l.draw(c);

      Bitmap bmp = Bitmap.createScaledBitmap(tmp, 512, 512, true);
      return bmp;
    }
}
