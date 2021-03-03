package com.example.blestart

import android.bluetooth.BluetoothDevice
import android.content.Context
import java.util.*

/**
 * BleOperationType provides abstract classes/functions that need to
 * be implemented in order to perform operations such as connecting,
 * disconnecting, reading data, and adjusting MTU.
 *
 * Credit: Punchthrough for tutorial and skeleton code
 * https://punchthrough.com/android-ble-guide/#Service-discovery-as-part-of-the-connection-flow
 * Author: Raunel Albiter (albiterri@msoe.edu)
 * Last Revised: 01/15/2021
 */

sealed class BleOperationType {
    abstract val device: BluetoothDevice
}

/** Connect to [device] and perform service discovery */
data class Connect(override val device: BluetoothDevice, val context: Context) : BleOperationType()

/** Disconnect from [device] and release all connection resources */
data class Disconnect(override val device: BluetoothDevice) : BleOperationType()

/** Read the value of a characteristic represented by [characteristicUuid] */
data class CharacteristicRead(
        override val device: BluetoothDevice,
        val characteristicUuid: UUID
) : BleOperationType()

/** Write [payload] as the value of a descriptor represented by [descriptorUuid] */
data class DescriptorWrite(
        override val device: BluetoothDevice,
        val descriptorUuid: UUID,
        val payload: ByteArray
) : BleOperationType() {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (javaClass != other?.javaClass) return false

        other as DescriptorWrite

        if (device != other.device) return false
        if (descriptorUuid != other.descriptorUuid) return false
        if (!payload.contentEquals(other.payload)) return false

        return true
    }

    override fun hashCode(): Int {
        var result = device.hashCode()
        result = 31 * result + descriptorUuid.hashCode()
        result = 31 * result + payload.contentHashCode()
        return result
    }
}

/** Read the value of a descriptor represented by [descriptorUuid] */
data class DescriptorRead(
        override val device: BluetoothDevice,
        val descriptorUuid: UUID
) : BleOperationType()

/** Enable notifications/indications on a characteristic represented by [characteristicUuid] */
data class EnableNotifications(
        override val device: BluetoothDevice,
        val characteristicUuid: UUID
) : BleOperationType()

/** Disable notifications/indications on a characteristic represented by [characteristicUuid] */
data class DisableNotifications(
        override val device: BluetoothDevice,
        val characteristicUuid: UUID
) : BleOperationType()

/** Request for an MTU of [mtu] */
data class MtuRequest(
        override val device: BluetoothDevice,
        val mtu: Int
) : BleOperationType()