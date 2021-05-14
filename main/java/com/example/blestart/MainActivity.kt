package com.example.blestart

import android.Manifest
import android.app.Activity
import android.app.AlertDialog
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.content.DialogInterface
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.Button
import androidx.annotation.RequiresApi
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.SimpleItemAnimator
import kotlinx.android.synthetic.main.activity_main.*
import org.jetbrains.anko.alert

private const val ENABLE_BLUETOOTH_REQUEST_CODE = 1
private const val LOCATION_PERMISSION_REQUEST_CODE = 2

/**
 * Main Activity Page currently functions as a scanner for
 * Bluetooth Low Energy devices. TempID featherboard can be
 * found under template name such as TempID00X.
 *
 * Credit: Punchthrough for tutorial and skeleton code
 * https://punchthrough.com/android-ble-guide/#Service-discovery-as-part-of-the-connection-flow
 * Author: Raunel Albiter (albiterri@msoe.edu)
 * Last Revised: 01/15/2021
 */
class MainActivity : AppCompatActivity() {

    /**
     * Bluetooth Properties
     *  -BluetoothAdapter (perform fundamental Bluetooth tasks)
     *  -BluetoothLEScanner (perform scan related operations for BLE devices)
     *  -ScanSettings (define parameters of scan)
     *  -ScanResultAdapter (display scan results with properties including MAC and device name)
     *  -isScanning (check whether scan is in progress)
     *  -isLocationPermissionGranted (location permission needed for Android 9.0 for scanning)
     */
    private val bluetoothAdapter: BluetoothAdapter by lazy {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    private val bleScanner by lazy {
        bluetoothAdapter.bluetoothLeScanner
    }

    @RequiresApi(Build.VERSION_CODES.M)
    private val scanSettings = ScanSettings.Builder()
        .setMatchMode(ScanSettings.MATCH_MODE_STICKY)
        .setScanMode(ScanSettings.SCAN_MODE_BALANCED)
        .build()

    private var isScanning = false
        set(value) {
            field = value
            runOnUiThread {findViewById<Button>(R.id.Scan).text =
                    if (value) "Stop Scan" else "Start Scan" }
        }

    //Returns computed value every time location is accessed (Read Only)
    private val scanResults = mutableListOf<ScanResult>()

    private val scanResultAdapter: ScanResultAdapter by lazy {
        ScanResultAdapter(scanResults) { result ->
            // User tapped on a scan result
            if (isScanning) {
                stopBleScan()
            }
            with(result.device) {
                Log.w("ScanResultAdapter", "Connecting to $address")
                //connectGatt(this@MainActivity, false, ConnectionManager.gattCallback)
                ConnectionManager.connect(this,this@MainActivity)
            }
        }
    }

    private val isLocationPermissionGranted
        get() = hasPermission(Manifest.permission.ACCESS_FINE_LOCATION)



    /**
     * Activity Function Overrides
     *  -onCreate (setup up display for scan results)
     *  -onResume (prompt user to enable bluetooth to begin scan)
     *  -onActivityResult (continue to ask user to enable bluetooth)
     *  -onRequestPermissionResult (begin scan)
     *  -onClick (button for starting scan)
     */

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        setupRecyclerView()
    }

    //Prompts user to enable bluetooth once
    override fun onResume() {
        super.onResume()
        ConnectionManager.registerListener(connectionEventListener)
        if (!bluetoothAdapter.isEnabled) {
            promptEnableBluetooth()
        }
    }

    //Continues to prompt user to enable bluetooth until it is enabled
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        when (requestCode) {
            ENABLE_BLUETOOTH_REQUEST_CODE -> {
                if (resultCode != Activity.RESULT_OK) {
                    //Crude Example but can also make new info alert to enable
                    //Bluetooth
                    promptEnableBluetooth()
                }
            }
        }
    }

    @RequiresApi(Build.VERSION_CODES.M)
    override fun onRequestPermissionsResult(
            requestCode: Int,
            permissions: Array<out String>,
            grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        when (requestCode) {
            LOCATION_PERMISSION_REQUEST_CODE -> {
                if (grantResults.firstOrNull() == PackageManager.PERMISSION_DENIED) {
                    requestLocationPermission()
                } else {
                    startBleScan()
                }
            }
        }
    }

    @RequiresApi(Build.VERSION_CODES.M)
    fun onClick(view: View) {
        if (isScanning) {
            stopBleScan()
        } else {
            startBleScan()
        }
    }



    /**
     * Private Function
     *  -promptEnableBluetooth (enable bluetooth prompt)
     *  -startBleScan (clear list, start scan, currently scanning)
     *  -stopBleScan (stop scan, currently not scanning)
     *  -requestLocationPermission (alert to enable location, cannot force enable like Bluetooth)
     *  -setupRecyclerView (list options for scan results)
     */

    //If bluetooth is not enabled a request is displayed to user
    private fun promptEnableBluetooth() {
        if (!bluetoothAdapter.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            startActivityForResult(enableBtIntent, ENABLE_BLUETOOTH_REQUEST_CODE)
        }
    }

    @RequiresApi(Build.VERSION_CODES.M)
    private fun startBleScan() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && !isLocationPermissionGranted) {
            requestLocationPermission()
        } else {
            //Refresh the list
            scanResults.clear()
            scanResultAdapter.notifyDataSetChanged()
            bleScanner.startScan(null, scanSettings, scanCallback)
            isScanning = true
        }
    }

    private fun stopBleScan() {
        bleScanner.stopScan(scanCallback)
        isScanning = false
    }

    //Displayed Alert when the scan begins in order to get permission to use location
    private fun requestLocationPermission() {
        val locationAlert = AlertDialog.Builder(this)
        locationAlert.setTitle("Location permission required")
        locationAlert.setMessage("Starting from Android M (6.0), the system requires apps to be granted " +
                "location access in order to scan for BLE devices.")
        locationAlert.setCancelable(false)
        locationAlert.setPositiveButton(android.R.string.ok) { _: DialogInterface, _: Int ->
            requestPermission(
                    Manifest.permission.ACCESS_FINE_LOCATION,
                    LOCATION_PERMISSION_REQUEST_CODE
            )
        }
        //Will continue to ask for location permission until granted
        if (isLocationPermissionGranted) {
            return
        }
        runOnUiThread {
            locationAlert.show()
        }
    }

    private fun setupRecyclerView() {
        scan_results_recycler_view.apply {
            adapter = scanResultAdapter
            layoutManager = LinearLayoutManager(this@MainActivity,
                RecyclerView.VERTICAL, false)
            isNestedScrollingEnabled = false
        }

        val animator = scan_results_recycler_view.itemAnimator
        if (animator is SimpleItemAnimator) {
            animator.supportsChangeAnimations = false
        }
    }



    /**
     * Callback Bodies
     *  -scanCallback (adds scan results to list and displays according to adapter class)
     *  -connectionEventListener (checks for connect and disconnect events, display message)
     */

    private val scanCallback = object : ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val indexQuery = scanResults.indexOfFirst { it.device.address == result.device.address }
            if (indexQuery != -1) { // A scan result already exists with the same address
                scanResults[indexQuery] = result
                scanResultAdapter.notifyItemChanged(indexQuery)
            } else {
                with(result.device) {
                    Log.i("ScanCallBack", "Found BLE device! Name: ${name ?: "Unnamed"},"
                            + "Address: $address")
                }
                scanResults.add(result)
                scanResultAdapter.notifyItemInserted(scanResults.size - 1)
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e("ScanCallBack", "onScanFailed: code $errorCode")
        }
    }

    private val connectionEventListener by lazy {
        ConnectionEventListener().apply {
            onConnectionSetupComplete = { gatt ->
                Intent(this@MainActivity, TempDisplay::class.java).also {
                    it.putExtra(BluetoothDevice.EXTRA_DEVICE, gatt.device)
                    startActivity(it)
                }
                ConnectionManager.unregisterListener(this)
            }
            onDisconnect = {
                runOnUiThread {
                    alert {
                        title = "Disconnected"
                        message = "Disconnected or unable to connect to device."
                        positiveButton("OK") {}
                    }.show()
                }
            }
        }
    }



    /**
     * Extension Functions
     *  -hasPermission (checks whether activity has location permission)
     *  -requestPermission (requests permission for activity)
     */

    private fun Context.hasPermission(permissionType: String): Boolean {
        return ContextCompat.checkSelfPermission(this, permissionType) ==
                PackageManager.PERMISSION_GRANTED
    }

    private fun Activity.requestPermission(permission: String, requestCode: Int) {
        ActivityCompat.requestPermissions(this, arrayOf(permission), requestCode)
    }

}