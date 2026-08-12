package org.example.gps;

import android.Manifest;
import android.content.Context;
import android.os.Build;
import android.provider.Settings;
import android.util.Log;
import android.content.pm.PackageManager;
import android.app.Activity;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;

import java.util.List;


/*

Build.BOARD;          // Board name
Build.BOOTLOADER;     // Bootloader version
Build.BRAND;          // Brand (e.g., Google, Samsung)
Build.DEVICE;         // Device code name
Build.DISPLAY;        // Build ID shown to users
Build.FINGERPRINT;    // Unique build fingerprint
Build.HARDWARE;       // Hardware name
Build.HOST;           // Build host
Build.ID;             // Build ID
Build.MANUFACTURER;   // Manufacturer
Build.MODEL;          // Device model
Build.ODM_SKU;        // ODM SKU (Android 12+)
Build.PRODUCT;        // Product name
Build.SKU;            // SKU (Android 12+)
Build.TAGS;           // Build tags
Build.TYPE;           // Build type (user, userdebug, eng)
Build.USER;           // Build user
Build.SOC_MANUFACTURER; // SoC manufacturer (Android 12+)
Build.SOC_MODEL;        // SoC model (Android 12+)

Build.VERSION.RELEASE;        // Android version (e.g., "14")
Build.VERSION.RELEASE_OR_CODENAME;
Build.VERSION.SDK_INT;        // API level (e.g., 34)
Build.VERSION.CODENAME;       // REL or codename
Build.VERSION.INCREMENTAL;    // Incremental build version
Build.VERSION.SECURITY_PATCH; // Security patch level

*/

public class DeviceInfo {

    public static void requestPhonePermission(Activity activity)
    {
        if (activity.checkSelfPermission(Manifest.permission.READ_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED)
        {
            activity.requestPermissions(
                    new String[]{Manifest.permission.READ_PHONE_STATE},
                    1001);
        }
    }

    public static String getBoard()          { return Build.BOARD; }
    public static String getProduct()        { return Build.PRODUCT; }
    public static String getManufacturer()   { return Build.MANUFACTURER; }
    public static String getModel()          { return Build.MODEL; }
    public static String getBrand()          { return Build.BRAND; }
    public static String getDevice()         { return Build.DEVICE; }
    public static String getHardware()       { return Build.HARDWARE; }
    public static String getFingerprint()    { return Build.FINGERPRINT; }
    public static String getBootloader()     { return Build.BOOTLOADER; }
    public static String getBuildId()        { return Build.ID; }
    public static String getDisplay()        { return Build.DISPLAY; }
    public static String getBuildType()      { return Build.TYPE; }
    public static String getTags()           { return Build.TAGS; }
    public static String getAndroidVersion() { return Build.VERSION.RELEASE; }
    public static int    getSdkVersion()     { return Build.VERSION.SDK_INT; }
    public static String getSecurityPatch()  { return Build.VERSION.SECURITY_PATCH; }

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

    // if Android < 10 ; can be get IMEI
    public static String getIMEI(Context context) {
        if (context.checkSelfPermission(Manifest.permission.READ_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED) {
            return "Permission denied";
        }

        TelephonyManager tm =
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
            return tm.getImei();
        } else {
            return tm.getDeviceId();
        }
    }

    public static int getSimSlotCount(Context context) {
        TelephonyManager tm =
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE);

        return tm.getActiveModemCount();
    }

    public static String printSimInfo(Context context)
    {
        StringBuilder sb = new StringBuilder();

        try
        {
            SubscriptionManager sm = SubscriptionManager.from(context);

            if (sm == null)
                return "SubscriptionManager == NULL";

            List<SubscriptionInfo> list = sm.getActiveSubscriptionInfoList();

            if (list == null || list.isEmpty())
                return "No active SIM";

            sb.append("SIM Count : ").append(list.size()).append("\n");

            for (SubscriptionInfo sim : list)
            {
                sb.append("Slot : ").append(sim.getSimSlotIndex()).append("\t");
                sb.append("Carrier : ").append(sim.getCarrierName()).append("\t");
                sb.append("Subscription ID : ").append(sim.getSubscriptionId()).append("\n");
            }
        }
        catch (Exception e)
        {
            return e.toString();
        }

        return sb.toString();
    }



}

