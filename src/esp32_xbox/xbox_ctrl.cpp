
/** NimBLE_Client Demo:
 *
 *  Demonstrates many of the available features of the NimBLE client library.
 *
 *  Created: on March 24 2020
 *      Author: H2zero
 *
 */
#include <Arduino.h>
#include <NimBLEDevice.h>
#include "xbox_ctrl.h"
#include "XboxSeriesXControllerESP32_asukiaaa.hpp"
// #include "ble_ctrl_parser.h"

XboxSeriesXControllerESP32_asukiaaa::Core
    xboxController("c0:d6:d5:e8:34:3a");

BLEControllerNotificationParser bleParser;

String xbox_string()
{
    String str = String(xboxController.xboxNotif.btnY) + "," +
                 String(xboxController.xboxNotif.btnX) + "," +
                 String(xboxController.xboxNotif.btnB) + "," +
                 String(xboxController.xboxNotif.btnA) + "," +
                 String(xboxController.xboxNotif.btnLB) + "," +
                 String(xboxController.xboxNotif.btnRB) + "," +
                 String(xboxController.xboxNotif.btnSelect) + "," +
                 String(xboxController.xboxNotif.btnStart) + "," +
                 String(xboxController.xboxNotif.btnXbox) + "," +
                 String(xboxController.xboxNotif.btnShare) + "," +
                 String(xboxController.xboxNotif.btnLS) + "," +
                 String(xboxController.xboxNotif.btnRS) + "," +
                 String(xboxController.xboxNotif.btnDirUp) + "," +
                 String(xboxController.xboxNotif.btnDirRight) + "," +
                 String(xboxController.xboxNotif.btnDirDown) + "," +
                 String(xboxController.xboxNotif.btnDirLeft) + "," +
                 "LHori:" + String(xboxController.xboxNotif.joyLHori) + "," +
                 String(xboxController.xboxNotif.joyLVert) + "," +
                 "RHori:" + String(xboxController.xboxNotif.joyRHori) + "," +
                 String(xboxController.xboxNotif.joyRVert) + "," +
                 String(xboxController.xboxNotif.trigLT) + "," +
                 String(xboxController.xboxNotif.trigRT) + "\n";
    return str;
};

BLEControllerNotificationParser *BLECtrl::get_status(void)
{
    return &bleParser;
}

void BLECtrl::setup(const char *ble_addr)
{
    Serial.begin(115200);
    log_i("Starting NimBLE Client");
    xboxController.begin();
}

void _update_status()
{
    bleParser.btnA = xboxController.xboxNotif.btnA;
    bleParser.btnDirUp = xboxController.xboxNotif.btnDirUp;
    bleParser.btnDirDown = xboxController.xboxNotif.btnDirDown;
    bleParser.btnDirLeft = xboxController.xboxNotif.btnDirLeft;
    bleParser.btnDirRight = xboxController.xboxNotif.btnDirRight;
    bleParser.joyLVert = xboxController.xboxNotif.joyLVert;
    bleParser.joyRVert = xboxController.xboxNotif.joyRVert;
    bleParser.joyLHori = xboxController.xboxNotif.joyLHori;
    bleParser.joyRHori = xboxController.xboxNotif.joyRHori;
}


void BLECtrl::loop(void)
{
    xboxController.onLoop();
    if (xboxController.isConnected())
    {
        if (xboxController.isWaitingForFirstNotification())
        {
            log_i("waiting for first notification");
        }
        else
        {
            _update_status();
            // log_i("%s", xbox_string().c_str());
            // demoVibration();
            //   demoVibration_2();
        }
    }
    else
    {
        log_i("not connected");
        // if (xboxController.getCountFailedConnection() > 2)
        // {
        //     ESP.restart();
        // }
    }
}
