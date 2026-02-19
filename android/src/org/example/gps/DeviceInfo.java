package org.example.gps;

import android.content.Context;
import android.os.Build;
import android.provider.Settings;

public class DeviceInfo {

    public static String getBoard() {
        return Build.BOARD;
    }

    public static String getProduct() {
        return Build.PRODUCT;
    }
    // Manufacturer (e.g. samsung, google)
    public static String getManufacturer() {
        return Build.MANUFACTURER;
    }

    // Model (e.g. SM-A536E, Pixel 7)
    public static String getModel() {
        return Build.MODEL;
    }

    // Android ID (SAFE replacement for serial)
    public static String getAndroidId(Context context) {
        try {
            return Settings.Secure.getString(
                    context.getContentResolver(),
                    Settings.Secure.ANDROID_ID
            );
        } catch (Exception e) {
            return "UNKNOWN";
        }
    }

    // Combined device ID (recommended)
    public static String getDeviceId(Context context) {
        return getManufacturer() + "_" +
               getModel() + "_" +
               getAndroidId(context);
    }
}

