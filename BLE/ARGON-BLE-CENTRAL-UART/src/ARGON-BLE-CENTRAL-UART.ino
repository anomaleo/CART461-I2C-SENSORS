/*
 * Project ARGON-BLE-CENTRAL-UART
 * Description:
 * Author:
 * Date:
 */

#include "Particle.h"

/* THESE ARE PARTICLE SPECIFIC PARAMETERS APPLIED AT CODE RUNTIME */
/* RUN ALL PARTICLE CLOUD COMMUNICATION IN SEPARATE THREAD */
SYSTEM_THREAD(ENABLED);
/* HOW TO CONNECT TO WiFi & INTERNET: AUTOMATIC, SEMI_AUTOMATIC, MANUAL */
SYSTEM_MODE(SEMI_AUTOMATIC);

// These UUIDs were defined by Nordic Semiconductor and are now the defacto standard for
// UART-like services over BLE. Many apps support the UUIDs now, like the Adafruit Bluefruit app.
const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");

const size_t UART_TX_BUF_SIZE = 20;
const size_t SCAN_RESULT_COUNT = 20;

BleScanResult scanResults[SCAN_RESULT_COUNT];

BleCharacteristic peerTxCharacteristic;
BleCharacteristic peerRxCharacteristic;
BlePeerDevice peer;


uint8_t txBuf[UART_TX_BUF_SIZE] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
size_t txLen = UART_TX_BUF_SIZE;


const unsigned long SCAN_PERIOD_MS = 2000;
unsigned long lastScan = 0;

void onDataReceived(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
    Serial.println("In data Received: ");
    for (size_t ii = 0; ii < len; ii++) {
        Serial.print((char)data[ii]);
    }
    Serial.println();

}

void setup() {
    Serial.begin();
    BLE.on();
    peerTxCharacteristic.onDataReceived(onDataReceived, &peerTxCharacteristic);
}

void loop() {
    if (BLE.connected()) {
        
        // while (Serial.available() && txLen < UART_TX_BUF_SIZE) {
        //     txBuf[txLen++] = Serial.read();
        //     Serial.write(txBuf[txLen - 1]);
        // }
        if (txLen > 0) {
            // Transmit the data to the BLE peripheral
            peerRxCharacteristic.setValue(txBuf, txLen);
            txLen = UART_TX_BUF_SIZE;
        }
    }
    else {
        if (millis() - lastScan >= SCAN_PERIOD_MS) {
            // Time to scan
            lastScan = millis();

            size_t count = BLE.scan(scanResults, SCAN_RESULT_COUNT);
            if (count > 0) {
                for (uint8_t ii = 0; ii < count; ii++) {
                    // Our serial peripheral only supports one service, so we only look for one here.
                    // In some cases, you may want to get all of the service UUIDs and scan the list
                    // looking to see if the serviceUuid is anywhere in the list.
                    BleUuid foundServiceUuid;
                    size_t svcCount = scanResults[ii].advertisingData.serviceUUID(&foundServiceUuid, 1);
                    if (svcCount > 0 && foundServiceUuid == serviceUuid) {
                        peer = BLE.connect(scanResults[ii].address);
                        if (peer.connected()) {
                            peer.getCharacteristicByUUID(peerTxCharacteristic, txUuid);
                            peer.getCharacteristicByUUID(peerRxCharacteristic, rxUuid);

                            // Could do this instead, but since the names are not as standardized, UUIDs are better
                            // peer.getCharacteristicByDescription(peerTxCharacteristic, "tx");
                        }
                        
                        break;
                    }
                }
            }
        }

    }
}