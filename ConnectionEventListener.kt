package com.example.blestart

import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor

/**
 * ConnectionEventListener is used to detect changes in status
 * of Bluetooth connection and characteristic changes in data.
 *
 * Credit: Punchthrough for tutorial and skeleton code
 * https://punchthrough.com/android-ble-guide/#Service-discovery-as-part-of-the-connection-flow
 * Author: Raunel Albiter (albiterri@msoe.edu)
 * Last Revised: 01/15/2021
 */

/** A listener containing callback methods to be registered with [ConnectionManager].*/
class ConnectionEventListener {
    var onConnectionSetupComplete: ((BluetoothGatt) -> Unit)? = null
    var onDisconnect: ((BluetoothDevice) -> Unit)? = null
    var onDescriptorRead: ((BluetoothDevice, BluetoothGattDescriptor) -> Unit)? = null
    var onDescriptorWrite: ((BluetoothDevice, BluetoothGattDescriptor) -> Unit)? = null
    var onCharacteristicChanged: ((BluetoothDevice, BluetoothGattCharacteristic) -> Unit)? = null
    var onCharacteristicRead: ((BluetoothDevice, BluetoothGattCharacteristic) -> Unit)? = null
    var onNotificationsEnabled: ((BluetoothDevice, BluetoothGattCharacteristic) -> Unit)? = null
    var onNotificationsDisabled: ((BluetoothDevice, BluetoothGattCharacteristic) -> Unit)? = null
    var onMtuChanged: ((BluetoothDevice, Int) -> Unit)? = null

}