package org.example.gps;

import android.content.Context;
import android.location.LocationManager;
import android.location.OnNmeaMessageListener;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;

public class NmeaListener {

    private LocationManager locationManager;
    private OnNmeaMessageListener listener;
    private Handler mainHandler;

    public NmeaListener(Context context) {
        locationManager =
            (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);

        // ✅ Attach to Android main looper
        mainHandler = new Handler(Looper.getMainLooper());
    }

    public void start() {
        if (locationManager == null)
            return;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            listener = (nmea, timestamp) -> {
                nativeOnNmea(nmea);
            };

            // ✅ Run on UI thread with Looper
            mainHandler.post(() -> {
                locationManager.addNmeaListener(listener);
            });
        }
    }

    public void stop() {
        if (locationManager != null && listener != null) {
            mainHandler.post(() -> {
                locationManager.removeNmeaListener(listener);
            });
        }
    }

    private static native void nativeOnNmea(String nmea);
}
