/**
 @file dialogbox.h
 @brief This is a class library for dialog boxes with messages and up to 2 buttons
 @author Carsten Meyer <info@keyboardpartner.de>
*/

#ifndef DIALOGBOX_H
#define DIALOGBOX_H

/*
// ############################################################################
//       __ ________  _____  ____  ___   ___  ___
//      / //_/ __/\ \/ / _ )/ __ \/ _ | / _ \/ _ \
//     / ,< / _/   \  / _  / /_/ / __ |/ , _/ // /
//    /_/|_/___/_  /_/____/\____/_/_|_/_/|_/____/
//      / _ \/ _ | / _ \/_  __/ |/ / __/ _ \
//     / ___/ __ |/ , _/ / / /    / _// , _/
//    /_/  /_/ |_/_/|_| /_/ /_/|_/___/_/|_|
//
// ############################################################################
*/

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts
#include "buttons.h"


#define MSG_WIDTH 236
#define MSG_HEIGHT 124
#define MSG_WIDTH_2 MSG_WIDTH/2 // halbe Breite
#define MSG_HEIGHT_2 MSG_HEIGHT/2 // halbe Höhe

#define WIDEBUTTON_W 96
#define WIDEBUTTON_W2 48  // halbe Button-Breite
#define WIDEBUTTON_H 34
#define WIDEBUTTON_H2 17  // halbe Button-Höhe

// msgType =0/16 "i" in blau, =1/17 "?" in blau, =2/18 "!" in rot
enum DialogBoxType {
	DB_INFO = 0,
	DB_REQUEST = 1,
	DB_ERROR = 2,

  DB_INFO_OK = 8,
  DB_REQUEST_OK = 9,
  DB_ERROR_OK = 10,

  DB_INFO_OKCANCEL = 16,
  DB_REQUEST_OKCANCEL = 17,
  DB_ERROR_OKCANCEL = 18
};

// Used to show a modal dialog with a message and an OK button
class DialogBox {
public:

// Constructor, initializes the dialog box. "tft" is the TFT display object, "touchProvider" is the touch event provider
DialogBox(TFT_eSPI *tft, TouchProvider *touchProvider)
	: _tft(tft), _touchProvider(touchProvider), _x1(0), _y1(0),
    _btnOK(_tft, _touchProvider), _btnCancel(_tft, _touchProvider) {}

  //   ######  ######     #    #     #
  //   #     # #     #   # #   #  #  #
  //   #     # #     #  #   #  #  #  #
  //   #     # ######  #     # #  #  #
  //   #     # #   #   ####### #  #  #
  //   #     # #    #  #     # #  #  #
  //   ######  #     # #     #  ## ##

	// Draw a dialog box with a message. Does not restore screen content.
	// message1 is the main message, message2 is an optional secondary message
  // msgType sets the icon type (and leaves space for buttons if needed)
	void draw(String message1, String message2, int msgType) {

		uint32_t my_msg_height = (msgType >= DB_INFO_OK) ? MSG_HEIGHT : MSG_HEIGHT * 3 / 5;
		uint16_t center_x = DISPLAY_W / 2; 	// 160 Pixel
		uint16_t center_y = DISPLAY_H / 2; // 120 Pixel
		uint16_t msg_center_y;
		if (msgType >= DB_INFO_OK) {
			msg_center_y = center_y - (my_msg_height / 4); // adjust for buttons
		} else {
			msg_center_y = center_y;
		}

		spkrBeep(50);
		DEBUG_PRINT(F("Dialog Box: "));
		DEBUG_PRINT(message1);
		DEBUG_PRINT(" ");
		DEBUG_PRINTLN(message2);
		uint16_t x0 = center_x - MSG_WIDTH_2;
		uint16_t y0 = center_y - my_msg_height / 2;
		msgType &= 0x07; // Mask to get the message type (0-7)
    // draw dialog/message box
		_tft->drawRect(x0, y0, MSG_WIDTH, my_msg_height, TFT_WHITE);
		_tft->drawRect(x0 + 1, y0 + 1, MSG_WIDTH - 2, my_msg_height - 2, TFT_WHITE);
		_tft->fillRect(x0 + 2, y0 + 2, MSG_WIDTH - 4, my_msg_height - 4, TFT_DIALOGGREY);

    // draw message text relative to center
		_tft->setTextFont(2);
		_tft->setTextColor(TFT_WHITE, TFT_DIALOGGREY);
		_tft->setTextDatum(MC_DATUM); // middle center text datum
		if (message2.length() == 0) {
			_tft->drawString(message1, center_x + 10, msg_center_y, 2);
		} else {
			_tft->drawString(message1, center_x + 10, msg_center_y - 8, 2);
			_tft->drawString(message2, center_x + 10, msg_center_y + 10, 2);
		}

		// display icon in the dialog box
		uint16_t msg_color;
		if (msgType == 2)
			msg_color = TFT_RED;
		else
			msg_color = TFT_BLUE;
		_tft->fillRoundRect(center_x - 104, msg_center_y - 18, 24, 36, 4, msg_color); // center_x - 120 +
		_tft->setTextColor(TFT_WHITE, msg_color);

		_tft->setFreeFont(FF22);
		center_x -= 93; // adjust center_x for icon position
    msg_center_y += 2;    // nudge down
		switch (msgType) {
		case 0:
			_tft->drawString("i", center_x, msg_center_y, 4);
			break;
		case 1:
			_tft->drawString("?", center_x, msg_center_y, 4);
			break;
		case 2:
			_tft->drawString("!", center_x, msg_center_y, 4);
			break;
		default:
			break;
		}
		_tft->setTextFont(2);
		_tft->setTextDatum(TL_DATUM); // middle center text datum
	}

  //   #     #  #####   #####
  //   ##   ## #     # #     #
  //   # # # # #       #
  //   #  #  #  #####  #  ####
  //   #     #       # #     #
  //   #     # #     # #     #
  //   #     #  #####   #####

	// Draw a dialog box with a message for a specified time
	// message1 is the main message, message2 is an optional secondary message
  // Screen content will be restored after the message box is displayed
  void message(String message1, String message2, int duration, int msgType = DB_INFO) {
    uint16_t x0 = DISPLAY_W / 2 - MSG_WIDTH_2;
    uint16_t y0 = DISPLAY_H / 2 - MSG_HEIGHT_2;
    uint16_t *screenbuf;
    screenbuf = new uint16_t[MSG_WIDTH * MSG_HEIGHT]; // Create a screen buffer
    _tft->readRect(x0, y0, MSG_WIDTH, MSG_HEIGHT, screenbuf);
    draw(message1, message2, msgType);
    delay(duration);
    // Restore screen content
    _tft->pushRect(x0, y0, MSG_WIDTH, MSG_HEIGHT, screenbuf);
    delete[] screenbuf; // Free the screen buffer memory
    delay(100);  // Wait a bit before restoring the screen
  }

  //   ######  #        #####
  //   #     # #       #     #
  //   #     # #       #
  //   #     # #       #  ####
  //   #     # #       #     #
  //   #     # #       #     #
  //   ######  #######  #####

	// Show a modal dialog with a message and an OK button
	// Returns true if OK was pressed, false if CANCEL was pressed
  // Screen content will be restored after the message box is displayed
	bool modalDlg(String message1, String message2, int msgType = DB_INFO_OK) {
		uint16_t center_x = DISPLAY_W / 2;
		uint16_t center_y = DISPLAY_H / 2; // 120 Pixel
    uint16_t x0 = center_x - MSG_WIDTH_2;
    uint16_t y0 = center_y - MSG_HEIGHT_2;

    uint16_t *screenbuf;
    screenbuf = new uint16_t[MSG_WIDTH * MSG_HEIGHT]; // Create a screen buffer
    _tft->readRect(x0, y0, MSG_WIDTH, MSG_HEIGHT, screenbuf);

		uint16_t tx, ty; // button coordinates
		ty = center_y + 34; // Button y position
		bool result = false;
    msgType |= DB_INFO_OK; // always with at least one button
		draw(message1, message2, msgType); // draw dialog box with message
		tx = center_x; // single button x position
		_tft->setTextColor(TFT_WHITE, TFT_DIALOGGREY);
		if (msgType >= DB_INFO_OKCANCEL) {
			_tft->drawCentreString(F("Press OK or CANCEL to continue"), center_x, center_y - 4, 2);
			tx = center_x - WIDEBUTTON_W2 - 10;
			_btnCancel.initCenter(tx, ty, WIDEBUTTON_W, WIDEBUTTON_H, TFT_WHITE, TFT_BTNGREY, TFT_RED, 2, FF21);
			_btnCancel.setLabel("CANCEL");
			_btnCancel.setActive(true, true);
			tx = center_x + WIDEBUTTON_W2 + 10; // Button x position
		} else {
			_tft->drawCentreString(F("Press OK to continue"), center_x, center_y - 4, 2);
		}
		_btnOK.initCenter(tx, ty, WIDEBUTTON_W, WIDEBUTTON_H, TFT_WHITE, TFT_BTNGREY, TFT_GREEN, 2, FF21);
		_btnOK.setLabel("OK");
		_btnOK.setActive(true, true);
		while (true) {
			_touchProvider->checkTouch(); // Check for touch input
			if ((msgType >= DB_INFO_OKCANCEL) && _btnCancel.checkPressed(true)) {
				spkrCancelBeep();
				break;
			}
			if (_btnOK.checkPressed(true)) {
				result = true;
				spkrOKbeep();
				break;
			}
			delay(20); // delay for debounce
		}
		_btnCancel.setEnabled(false, false);
		_btnOK.setEnabled(false, false);
		_tft->setTextFont(2);
    // Restore screen content
    _tft->pushRect(x0, y0, MSG_WIDTH, MSG_HEIGHT, screenbuf);
    delete[] screenbuf; // Free the screen buffer memory
    delay(100);  // Wait a bit before restoring the screen
		_tft->setTextColor(TFT_WHITE, TFT_BLACK);
		return result;
	}

private:
  TFT_eSPI *_tft;
  TouchProvider *_touchProvider;  // Pointer to the touch provider for touch handling
  int16_t  _x1, _y1;              // Coordinates of top-left corner of dialog box
  int16_t  _x2, _y2;              // Coordinates of bottom-right corner of dialog box
  PushButton _btnOK;
  PushButton _btnCancel;
};

#endif
