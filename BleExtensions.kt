package com.example.blestart

import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.util.Log
import java.lang.StringBuilder
import java.util.*

/** UUID of the Client Characteristic Configuration Descriptor (0x2902). */
const val CCC_DESCRIPTOR_UUID = "00002902-0000-1000-8000-00805F9B34FB"

/**
 * BleExtensions provides global functions and values that can be
 * used in order to determine attributes, characteristics, and properties of
 * the desired BLE device
 *
 * Credit: Punchthrough for tutorial and skeleton code
 * https://punchthrough.com/android-ble-guide/#Service-discovery-as-part-of-the-connection-flow
 * Author: Raunel Albiter (albiterri@msoe.edu)
 * Last Revised: 01/15/2021
 */

//BluetoothGatt
fun BluetoothGatt.printGattTable() {
    if (services.isEmpty()) {

        Log.i("printGattTable", "No service and characteristic available, call discoverServices() first?")
        return
    }
    services.forEach { service ->
        val characteristicsTable = service.characteristics.joinToString(
                separator = "\n|--",
                prefix = "|--"
        ) { it.uuid.toString() }
        Log.i("printGattTable", "\nService ${service.uuid}\nCharacteristics:\n$characteristicsTable"
        )
    }
}

fun BluetoothGatt.findCharacteristic(uuid: UUID): BluetoothGattCharacteristic? {
    services?.forEach { service ->
        service.characteristics?.firstOrNull { characteristic ->
            characteristic.uuid == uuid
        }?.let { matchingCharacteristic ->
            return matchingCharacteristic
        }
    }
    return null
}

fun BluetoothGatt.findDescriptor(uuid: UUID): BluetoothGattDescriptor? {
    services?.forEach { service ->
        service.characteristics.forEach { characteristic ->
            characteristic.descriptors?.firstOrNull { descriptor ->
                descriptor.uuid == uuid
            }?.let { matchingDescriptor ->
                return matchingDescriptor
            }
        }
    }
    return null
}

//BluetoothGattCharacteristic

fun BluetoothGattCharacteristic.isReadable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_READ)

fun BluetoothGattCharacteristic.isNotifiable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_NOTIFY)

fun BluetoothGattCharacteristic.isIndicatable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_INDICATE)

fun BluetoothGattCharacteristic.containsProperty(property: Int): Boolean =
        properties and property != 0


//BluetoothGattDescriptor

fun BluetoothGattDescriptor.isReadable(): Boolean =
        containsPermission(BluetoothGattDescriptor.PERMISSION_READ)

fun BluetoothGattDescriptor.containsPermission(permission: Int): Boolean =
        permissions and permission != 0

/**
 * Convenience extension function that returns true if this [BluetoothGattDescriptor]
 * is a Client Characteristic Configuration Descriptor.
 */
fun BluetoothGattDescriptor.isCccd() =
        uuid.toString().toUpperCase(Locale.US) == CCC_DESCRIPTOR_UUID.toUpperCase(Locale.US)

// Byte Array

fun ByteArray.toHexString(): String =
        joinToString(separator = "", prefix = "") { String.format("%02X", it) }