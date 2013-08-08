package com.moodstocks.vuforia;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.View;

import com.moodstocks.android.MoodstocksError;
import com.moodstocks.android.Scanner;

public class HomeScreen extends Activity implements View.OnClickListener, Scanner.SyncListener {

	public static final String TAG = "HomeScreen";
	private boolean compatible = false;
	private Scanner scanner = null;

	/* sync related variables */
	private long last_sync = 0;
	private static final long DAY = DateUtils.DAY_IN_MILLIS;

	//--------------------------------
	// Moodstocks API key/secret pair
	//--------------------------------
	private static final String API_KEY    = "ApIkEy";
	private static final String API_SECRET = "ApIsEcReT";

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		/* First of all, check that the device is compatible, aka runs Android 2.3 or over.
		 * If it's not the case, you **must** not try using the scanner as it will crash.
		 * Here we chose to inform the user with a popup and kill the app. In practice, you
		 * may want to do this verification at application startup and display the button
		 * allowing scanner access if and only if the device is compatible.
		 */
		compatible = Scanner.isCompatible();
		if (compatible) {
			setContentView(R.layout.home);
			findViewById(R.id.scan_button).setOnClickListener(this);
			try {
				this.scanner = Scanner.get();
				/* Open the scanner, necessary to perform any operation using it.
				 * This step also checks at runtime that the device is compatible.
				 * If the device is not compatible, it will throw a RuntimeException
				 * and crash the app.
				 */
				scanner.open(this, API_KEY, API_SECRET);
			} catch (MoodstocksError e) {
				/* an error occurred while opening the scanner */
				if (e.getErrorCode() == MoodstocksError.Code.CREDMISMATCH) {
					// == DO NOT USE IN PRODUCTION: THIS IS A HELP MESSAGE FOR DEVELOPERS
					String errmsg = "there is a problem with your key/secret pair: "+
							"the current pair does NOT match with the one recorded within the on-disk datastore. "+
							"This could happen if:\n"+
							" * you have first build & run the app without replacing the default"+
							" \"ApIkEy\" and \"ApIsEcReT\" pair, and later on replaced with your real key/secret,\n"+
							" * or, you have first made a typo on the key/secret pair, build & run the"+
							" app, and later on fixed the typo and re-deployed.\n"+
							"\n"+
							"To solve your problem:\n"+
							" 1) uninstall the app from your device,\n"+
							" 2) make sure to properly configure your key/secret pair within Scanner.java\n"+
							" 3) re-build & run\n";
					MoodstocksError err = new MoodstocksError(errmsg, MoodstocksError.Code.CREDMISMATCH);
					err.log();
					finish();
					// == DO NOT USE IN PRODUCTION: THIS WAS A HELP MESSAGE FOR DEVELOPERS
				}
				else {
					e.log();
				}
			}
		}
		else {
			/* device is *not* compatible. In this demo application, we chose
       * to inform the user and exit application. `compatible` flag is here
       * to avoid calling scanner methods that *will* fail and log errors.
       */
      AlertDialog.Builder builder = new AlertDialog.Builder(this);
      builder.setCancelable(false);
      builder.setTitle("Unsupported device!");
      builder.setMessage("Device must feature an ARM CPU and run Android Gingerbread or over, sorry...");
      builder.setNeutralButton("Quit", new DialogInterface.OnClickListener() {
        public void onClick(DialogInterface dialog, int id) {
          finish();
        }
      });
      builder.show();
		}
	}

	@Override
	protected void onResume() {
		/* perform a sync if:
		 * - the app is started either for the first time,
		 *   or has been killed and is started back.
		 * - the app is resumed from the background AND
		 *   has not been synced for more than one day.
		 */
		super.onResume();
		if (compatible && System.currentTimeMillis() - last_sync > DAY)
			scanner.sync(this);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		if (compatible) {
			try {
				/* you must close the scanner before exiting */
				scanner.close();
			} catch (MoodstocksError e) {
				e.log();
			}
		}
	}

	@Override
	public void onClick(View v) {
		if (v == findViewById(R.id.scan_button)) {
			// launch scanner
			startActivity(new Intent(this, Main.class));
		}
	}

	//----------------------
	// Scanner.SyncListener
	//----------------------

	/* The synchronization is performed seamlessly. Until it has ended,
	 * the user can still use the online search as a fallback.
	 */

	@Override
	public void onSyncStart() {
		// Developer logs, do not use in production
		Log.d(TAG, "[SYNC] Starting...");
	}

	@Override
	public void onSyncComplete() {
		last_sync = System.currentTimeMillis();
		// Developer logs, do not use in production
		Log.d(TAG, "[SYNC] Complete!");
	}

	@Override
	public void onSyncFailed(MoodstocksError e) {
		e.log();
	}

	@Override
	public void onSyncProgress(int total, int current) {
		// Developer logs, do not use in production
		Log.d(TAG, "[SYNC] "+current+"/"+total);
	}

}
