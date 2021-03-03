package com.example.blestart

import android.annotation.SuppressLint
import android.bluetooth.le.ScanResult
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import kotlinx.android.synthetic.main.row_scan_result.view.*
import org.jetbrains.anko.layoutInflater

/**
 * Adapter class for displaying the results of Bluetooth scan
 * in a formatted style so that most necessary data is present
 * such as device name, MAC, and RSSI (signal strength).
 *
 * Credit: Punchthrough for tutorial and skeleton code
 * https://punchthrough.com/android-ble-guide/#Service-discovery-as-part-of-the-connection-flow
 */
class ScanResultAdapter(
        private val items: List<ScanResult>,
        private val onClickListener: ((device: ScanResult) -> Unit)
) : RecyclerView.Adapter<ScanResultAdapter.ViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = parent.context.layoutInflater.inflate(
                R.layout.row_scan_result,
                parent,
                false
        )
        return ViewHolder(view, onClickListener)
    }

    override fun getItemCount() = items.size

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val item = items[position]
        holder.bind(item)
    }

    class ViewHolder(
            private val view: View,
            private val onClickListener: ((device: ScanResult) -> Unit)
    ) : RecyclerView.ViewHolder(view) {

        @SuppressLint("SetTextI18n")
        fun bind(result: ScanResult) {
            view.device_name.text = result.device.name ?: "Unnamed"
            view.mac_address.text = result.device.address
            view.signal_strength.text = "${result.rssi} dBm"
            view.setOnClickListener { onClickListener.invoke(result) }
        }
    }
}