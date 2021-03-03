package com.example.blestart

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGattCharacteristic
import android.os.Bundle
import android.util.Log
import android.view.MenuItem
import android.view.View
import android.widget.CheckBox
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.amplifyframework.AmplifyException
import com.amplifyframework.core.Amplify
import com.amplifyframework.datastore.AWSDataStorePlugin
import com.amplifyframework.datastore.DataStoreException
import com.amplifyframework.datastore.DataStoreItemChange
import com.amplifyframework.datastore.generated.model.Priority
import com.amplifyframework.datastore.generated.model.TempUser
import kotlinx.android.synthetic.main.activity_temp_display.*
import org.jetbrains.anko.alert
import java.lang.StringBuilder
import java.text.SimpleDateFormat
import java.util.*


/**
 * TempDisplay Page currently functions as a logger for temperature
 * data received over Bluetooth depending on if the user enables
 * notifications.
 *
 * Credit: Punchthrough for tutorial and skeleton code
 * https://punchthrough.com/android-ble-guide/#Service-discovery-as-part-of-the-connection-flow
 * Author: Raunel Albiter (albiterri@msoe.edu)
 * Last Revised: 01/15/2021
 */

class TempDisplay : AppCompatActivity(), View.OnClickListener {

    private lateinit var device: BluetoothDevice
    private val dateFormatter = SimpleDateFormat("MMM d, HH:mm:ss", Locale.US)
    private var subcribeBox:CheckBox ? = null
    //private var checkBattery:CheckBox ? = null
    private var logTextView:TextView ? = null
    private var alreadySubscribed:Boolean = false
    private var uartServiceUuid: UUID? = null
    private var uartReadCharUuid: UUID? = null
    private var uartChar: BluetoothGattCharacteristic? = null
    private lateinit var newData: String

    private var notifyingCharacteristics = mutableListOf<UUID>()

    override fun onCreate(savedInstanceState: Bundle?) {
        ConnectionManager.registerListener(connectionEventListener)
        super.onCreate(savedInstanceState)
        device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                ?: error("Missing BluetoothDevice from MainActivity!")

        setContentView(R.layout.activity_temp_display)
        supportActionBar?.apply {
            setDisplayHomeAsUpEnabled(true)
            setDisplayShowTitleEnabled(true)
            title = "Readings"
        }
        subcribeBox = findViewById(R.id.subscribe)
        subcribeBox?.setOnClickListener(this)
        //checkBattery = findViewById(R.id.battery)
        //checkBattery?.setOnClickListener(this)
        logTextView = findViewById(R.id.logView)

        try {
            Amplify.addPlugin(AWSDataStorePlugin())
            Amplify.configure(applicationContext)
            Log.i("Tutorial", "Initialized Amplify")
        } catch (e: AmplifyException) {
            Log.e("Tutorial", "Could not initialize Amplify", e)
        }
    }

    override fun onDestroy() {
        ConnectionManager.unregisterListener(connectionEventListener)
        ConnectionManager.teardownConnection(device)
        super.onDestroy()
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        when (item.itemId) {
            android.R.id.home -> {
                onBackPressed()
                return true
            }
        }
        return super.onOptionsItemSelected(item)
    }

    /**
     * onClick will be the primary form of enabling notifications
     * via a checkbox. Results are printed onto the text view below
     * the checkbox. Results will include temperature and battery data
     */
    override fun onClick(v: View?) {
        val gatt = ConnectionManager.getGatt()
        if(subcribeBox?.isChecked!! && !alreadySubscribed) {
            uartServiceUuid = UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
            uartReadCharUuid = UUID.fromString("6e400003-b5a3-f393-e0a9-e50e24dcca9e")
            uartChar = gatt
                    ?.getService(uartServiceUuid)?.getCharacteristic(uartReadCharUuid)
            log("Enabling notifications on ${uartChar?.uuid}")
            uartChar?.let { ConnectionManager.enableNotifications(device, it) }
        } else {
            log("Disabling notifications on ${uartChar?.uuid}")
            uartChar?.let { ConnectionManager.disableNotifications(device, it) }
        }

//        if(checkBattery?.isChecked!!) {
//            val batteryServiceUuid = UUID.fromString("0000180f-0000-1000-8000-00805f9b34fb")
//            val batteryLevelCharUuid = UUID.fromString("00002a19-0000-1000-8000-00805f9b34fb")
//            val batteryLevelChar = gatt
//                    ?.getService(batteryServiceUuid)?.getCharacteristic(batteryLevelCharUuid)
//            if (batteryLevelChar != null) {
//                log("Enabling notifications on ${batteryLevelChar.uuid}")
//                ConnectionManager.enableNotifications(device,batteryLevelChar)
//            }
//        }
    }

    /**
     * Primary form of logging data nicely with a timestamp
     */
    @SuppressLint("SetTextI18n")
    private fun log(message: String) {
        val formattedMessage = String.format("%s: %s", dateFormatter.format(Date()), message)
        runOnUiThread {
            val currentLogText = if (logView.text.isEmpty()) {
                "Beginning of log."
            } else {
                logView.text
            }
            logView.text = "$currentLogText\n$formattedMessage"
            //log_scroll_view.post { log_scroll_view.fullScroll(View.FOCUS_DOWN) }
        }
    }

    @SuppressLint("SetTextI18n")
    private fun printNewData() {
        val output = StringBuilder()
        var i = 0
        while (i < newData.length) {
            Log.i("TempDisplay","Never reached?")
            val str: String = newData.substring(i, i + 2)
            output.append(str.toInt(16).toChar())
            i += 2
        }
        newData = output.toString()
        val formattedMessage = String.format("%s: %s", dateFormatter.format(Date()),
                "New Data: $output")
        runOnUiThread {
            val currentLogText = if (logView.text.isEmpty()) {
                "Beginning of log."
            } else {
                logView.text
            }
            logView.text = "$currentLogText\n$formattedMessage"
            //log_scroll_view.post { log_scroll_view.fullScroll(View.FOCUS_DOWN) }
        }
    }

    private fun hexToAscii(hexStr: String): String {
        val output = StringBuilder()
        var i = 0
        while (i < hexStr.length) {
            Log.i("TempDisplay","Never reached?")
            val str: String = hexStr.substring(i, i + 2)
            output.append(str.toInt(16).toChar())
            i += 2
        }
        return output.toString()
    }

    /**
     * Event listener for new data that is available from
     * the Featherboard or status changes such as connection/disconnection.
     * TODO: Here is where the data will be sent to database
     */
    private val connectionEventListener by lazy {
        ConnectionEventListener().apply {
            onDisconnect = {
                runOnUiThread {
                    alert {
                        title = "Disconnected"
                        message = "Disconnected from device."
                        positiveButton("OK") { onBackPressed() }
                    }.show()
                }
            }

            onCharacteristicRead = { _, characteristic ->
                log("Read from ${characteristic.uuid}: ${characteristic.value.toHexString()}")
            }

            onMtuChanged = { _, mtu ->
                log("MTU updated to $mtu")
            }

            onCharacteristicChanged = { _, characteristic ->
                //log("Value changed on ${characteristic.uuid}: ${characteristic.value.toHexString()}")
                newData = characteristic.value.toHexString()
                printNewData()
                val reading = newData.split(" ").toTypedArray()
                if(reading[0] == "T") {
                    val item = TempUser.builder()
                            .name("Raunel")
                            .priority(Priority.HEALTHY)
                            .description("Temperature is ${reading[1]} F")
                            .build()

                    Amplify.DataStore.save(
                            item,
                            { success: DataStoreItemChange<TempUser> ->
                                Log.i("Tutorial", "Saved item: " + success.item().name) },
                            { error: DataStoreException? ->
                                Log.e("Tutorial", "Could not save item to DataStore", error) }
                    )
                }
            }

            onNotificationsEnabled = { _, characteristic ->
                log("Enabled notifications on ${characteristic.uuid}")
                notifyingCharacteristics.add(characteristic.uuid)
            }

            onNotificationsDisabled = { _, characteristic ->
                log("Disabled notifications on ${characteristic.uuid}")
                notifyingCharacteristics.remove(characteristic.uuid)
            }
        }
    }


}