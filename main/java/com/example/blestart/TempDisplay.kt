package com.example.blestart

import android.annotation.SuppressLint
import android.app.Notification
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGattCharacteristic
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.MenuItem
import android.view.View
import android.widget.CheckBox
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.NotificationCompat
import com.amplifyframework.AmplifyException
import com.amplifyframework.core.Amplify
import com.amplifyframework.datastore.AWSDataStorePlugin
import com.amplifyframework.datastore.DataStoreException
import com.amplifyframework.datastore.DataStoreItemChange
import com.amplifyframework.datastore.generated.model.Priority
import com.amplifyframework.datastore.generated.model.TempUser
import com.jjoe64.graphview.GraphView
import com.jjoe64.graphview.series.DataPoint
import com.jjoe64.graphview.series.LineGraphSeries
import kotlinx.android.synthetic.main.activity_temp_display.*
import org.jetbrains.anko.alert
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

private const val FEVER_TEMP = 100.6-15.0
private const val NORMAL_TEMP_MIN = 97-15
private const val NORMAL_TEMP_MAX = 99-15


@SuppressLint("SetTextI18n")
class TempDisplay : AppCompatActivity(), View.OnClickListener {


    private lateinit var device:BluetoothDevice
    private lateinit var graph :GraphView
    private var subscribeBox:CheckBox ? = null
    private lateinit var batteryTextView:TextView
    private lateinit var logTextView:TextView
    private lateinit var tempSeries: LineGraphSeries<DataPoint>
    private var alreadySubscribed:Boolean = false
    private var uartServiceUuid: UUID? = null
    private var uartReadCharUuid: UUID? = null
    private var uartChar: BluetoothGattCharacteristic? = null
    private lateinit var newData: String
    private var seriesCount: Double = 0.0
    private var notifyingCharacteristics = mutableListOf<UUID>()
    private val dateFormatter = SimpleDateFormat("MMM d, HH:mm:ss", Locale.US)
    private val graphDate = SimpleDateFormat("HH", Locale.US)
    private lateinit var connectedTextView:TextView
    private var storedTemps = mutableListOf<Int>()
    private lateinit var lowView:TextView
    private lateinit var avgView:TextView
    private lateinit var highView:TextView
    private var currentStatus:String = ""
    private var previousStatus:String = ""
    private var highFever:Boolean = false

    override fun onCreate(savedInstanceState: Bundle?) {
        ConnectionManager.registerListener(connectionEventListener)
        super.onCreate(savedInstanceState)
        device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE)
                ?: error("Missing BluetoothDevice from MainActivity!")

        setContentView(R.layout.activity_temp_display)
        supportActionBar?.apply {
            setDisplayHomeAsUpEnabled(true)
            setDisplayShowTitleEnabled(true)
            title = device.name
        }
        connectedTextView = findViewById(R.id.device_address)
        connectedTextView.text = "Connected to: ${device.address}"
        subscribeBox = findViewById(R.id.subscribe)
        subscribeBox?.setOnClickListener(this)
        batteryTextView = findViewById(R.id.battery)
        logTextView = findViewById(R.id.logView)
        graph = findViewById(R.id.graph)
        graph.visibility = View.VISIBLE;
        tempSeries = LineGraphSeries()

        lowView = findViewById(R.id.lowView)
        avgView = findViewById(R.id.avgView)
        highView = findViewById(R.id.highView)

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
        if(subscribeBox?.isChecked!! && !alreadySubscribed) {
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
    }

    /**
     * Primary form of logging data nicely with a timestamp
     */
    @SuppressLint("SetTextI18n")
    private fun log(message: String) {
        val formattedMessage = String.format("%s: %s", dateFormatter.format(Date()), message)
        runOnUiThread {
            val currentLogText = if (logTextView.text.isEmpty()) {
                "Beginning of log."
            } else {
                logTextView.text
            }
            logTextView.text = "$currentLogText\n$formattedMessage"
            log_scroll_view.post { log_scroll_view.fullScroll(View.FOCUS_DOWN) }
        }
    }

    /**
     * Function used to log new data with a date to the Log View which corresponds to
     *  either a new temperature reading or a new battery percentage
     */
    @SuppressLint("SetTextI18n")
    private fun printNewData() {
        val output = StringBuilder()
        var i = 0
        while (i < newData.length) {
            val str: String = newData.substring(i, i + 2)
            output.append(str.toInt(16).toChar())
            i += 2
        }
        newData = output.toString()
        val formattedMessage = String.format(
            "%s: %s", dateFormatter.format(Date()),
            "New Data: $output"
        )
        runOnUiThread {
            val currentLogText = if (logTextView.text.isEmpty()) {
                "Beginning of log."
            } else {
                logTextView.text
            }
            logTextView.text = "$currentLogText\n$formattedMessage"
            log_scroll_view.post { log_scroll_view.fullScroll(View.FOCUS_DOWN) }
        }
    }

    private fun updateStats(temperature: String) {
        storedTemps.add(temperature.toInt())
        val lowTemp = storedTemps.min()?.toInt()
        Log.i("Update Stats", "MIN: $lowTemp")
        val highTemp = storedTemps.max()?.toInt()
        Log.i("Update Stats", "HIGH: $highTemp")
        val avgTemp = storedTemps.average().toInt()
        Log.i("Update Stats", "AVG: $avgTemp")

        lowView.text = "LOW: ${lowTemp.toString()}" + "\u2109"
        avgView.text = "AVG: ${avgTemp.toString()}" + "\u2109"
        highView.text = "HIGH: ${highTemp.toString()}" + "\u2109"

        if(avgTemp >= FEVER_TEMP) {
            highFever = true
            currentStatus = "Feeling Feverish? Please refer to CDC guidelines below for more info. " +
                    "Speak to supervisor when possible."
        } else if(avgTemp < NORMAL_TEMP_MIN || avgTemp > NORMAL_TEMP_MAX) {
            highFever = false
            currentStatus = "Please see TempID tech support. Device may need recalibration."
        } else {
            highFever = false
            currentStatus = "All good"
        }
    }

    private fun sendNotification() {
        if(currentStatus != "All good" && previousStatus.compareTo(currentStatus) != 0) {
            previousStatus = currentStatus
            val someIntent = Intent(
                Intent.ACTION_VIEW,
                Uri.parse("https://www.cdc.gov/quarantine/air/reporting-deaths-illness/definitions-symptoms-reportable-illnesses.html")
            )
            val contentIntent = PendingIntent.getActivity(this, 0, someIntent, 0)
            val builder = NotificationCompat.Builder(this, "notify")
            builder.setContentIntent(contentIntent)
            builder.setSmallIcon(R.drawable.ic_launcher_foreground)
            builder.setContentTitle("Temperature Status")
            builder.setContentText(currentStatus)
            builder.priority = Notification.PRIORITY_DEFAULT
            if (highFever) {
                builder.addAction(R.drawable.ic_launcher_background, "CDC Guidelines", contentIntent)
            }
            val notifManager = getSystemService(NOTIFICATION_SERVICE) as NotificationManager
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                val channelId = "Your_channel_id"
                val channel = NotificationChannel(
                    channelId,
                    "Channel human readable title",
                    NotificationManager.IMPORTANCE_HIGH
                )
                notifManager.createNotificationChannel(channel)
                builder.setChannelId(channelId)
            }
            notifManager.notify(0, builder.build())
        }
    }

    /**
     * Function used to add a new data point to the graph which corresponds
     *  to a new temperature reading from sensor
     * Sample Code: https://medium.com/@iamtjah/how-to-create-a-simple-graph-in-android-6c484324a4c1
     * @param temperature Temperature reading in degrees Fahrenheit/Celsius
     */
    private fun graphNewData(temperature: String) {
        val dataPoint = DataPoint(seriesCount, temperature.toDouble())
        tempSeries.appendData(dataPoint, false, 12, true)
        graph.addSeries(tempSeries)
        seriesCount += 1
    }

    /**
     * Function used to update the battery percentage in a text view
     *  based on the voltage readings from Adafruit Featherboard.
     *  @param batteryLevel Battery level in percent
     */
    private fun updateBattery(batteryLevel: String) {
        batteryTextView.text = "$batteryLevel %"
    }

    /**
     * Event listener for new data that is available from
     * the Featherboard or status changes such as connection/disconnection.
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
                newData = characteristic.value.toHexString()
                printNewData()
                val reading = newData.split(" ").toTypedArray()
                if(reading[0] == "T") {
                    graphNewData(reading[1])
                    updateStats(reading[1])
                    sendNotification()
                    val item = TempUser.builder()
                            .name("Raunel")
                            .priority(Priority.HEALTHY)
                            .description("Temperature is ${reading[1]} F")
                            .build()

                    Amplify.DataStore.save(
                        item,
                        { success: DataStoreItemChange<TempUser> ->
                            Log.i("Tutorial", "Saved item: " + success.item().name)
                        },
                        { error: DataStoreException? ->
                            Log.e("Tutorial", "Could not save item to DataStore", error)
                        }
                    )
                } else {
                    updateBattery(reading[1])
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