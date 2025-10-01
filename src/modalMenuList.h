#ifndef MODALMENU_H
#define MODALMENU_H

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
//          by cm@keyboardpartner.de
//
// ############################################################################
*/

// Modal List Select Box for menus, directories, WIFI networks etc.
// If ENCODER_ENABLED is set, it uses the rotary encoder (param encdelta) for navigation
// encdelta should be NULL when ENCODER_ENABLED is not set

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "touchProvider.h" // Common touch provider for all widgets
#include "guiObject.h" // Common GUI object for all widgets
#include "Free_Fonts.h" // Include large fonts
#include "buttons.h"

// TODO: make these constants settable
#define LISTBOX_X (DISPLAY_W/8) // List Box X position, centered
#define LISTBOX_W (DISPLAY_W - LISTBOX_X*2)

#define LISTBOX_PADDING 10 // 10 Pixel line padding on each side
#define LISTBOX_MAXLINES 8 // Max number of lines in the list box
#define LISTFIELD_H 20
#define LISTFIELD_W (LISTBOX_W - LISTBOX_PADDING*2) // 10 Pixel padding on each side
#define LISTFIELD_LEFT (LISTBOX_X + LISTBOX_PADDING) // linke X-Koordinate
#define LISTFIELD_TOP_OFFS (LISTFIELD_H + 8) // obere Y-Koordinate Listenfelder

#define SMALLBUTTON_W 50
#define SMALLBUTTON_H 18

// ##############################################################################
//
//  ##     ## ######## ##    ## ##     ##
//  ###   ### ##       ###   ## ##     ##
//  #### #### ##       ####  ## ##     ##
//  ## ### ## ######   ## ## ## ##     ##
//  ##     ## ##       ##  #### ##     ##
//  ##     ## ##       ##   ### ##     ##
//  ##     ## ######## ##    ##  #######
//
// ##############################################################################

// Modal menu list, may be used for file selection or menu purposes


class ModalMenu {
public:

// Constructor, initializes the modal menu. "tft" is the TFT display object, "touchProvider" is the touch event provider
ModalMenu(TFT_eSPI *tft, TouchProvider *touchProvider)
	: _tft(tft), _touchProvider(touchProvider), _x1(0), _y1(0),
    btnCancel(_tft, _touchProvider) {}

	// Draw a modal menu with a message and selectable entries
	// message1 is the main message, message2 is an optional secondary message
	// This function displays a list of entries in a modal dialog and allows the user to select an entry
	// The entries are passed in the array parameter, and the number of entries is specified by entry_count
	int select(menuArr_t array, int entry_count, String message1) {
		// bietet vorbefüllte Liste zur Auswahl an, z.B. Directory
		// liefert Eintrag zurück oder -1, wenn CANCEL gewählt wurde
		//setSetupBtnsActive(false, (instrState == state_menu));
		strcpy(array[entry_count], "CANCEL"); // immer letzter Eintrag
		int selected_line = 0;
		int start_line = 0;
		int linecount = entry_count + 1; // inkl. CANCEL
		if (linecount > LISTBOX_MAXLINES)
			linecount = LISTBOX_MAXLINES;
		int selected_item = 0;

		tft.setTextFont(2);
		tft.setTextColor(TFT_WHITE, TFT_MEDGREY);
		tft.setTextDatum(TL_DATUM); // top left text datum
		int listbox_height = LISTFIELD_H * (linecount + 2); // total height of the list box
    int listbox_y = DISPLAY_H / 2 - listbox_height / 2; // center the list box vertically
		int listfield_top = listbox_y + LISTFIELD_TOP_OFFS; // top position of the list box
		int listfield_bottom = listfield_top + (linecount * LISTFIELD_H); // bottom position of the list box
    tft.fillRect(LISTBOX_X + 1, listbox_y, LISTBOX_W - 2, listbox_height - 2, TFT_MEDGREY);
		tft.drawRect(LISTBOX_X, listbox_y, LISTBOX_W, listbox_height, TFT_WHITE);
		tft.drawString(message1, LISTFIELD_LEFT + 2, listbox_y + 6, 2);
		bool cancelled = false;
		btnCancel.init(LISTFIELD_LEFT + LISTFIELD_W - SMALLBUTTON_W, listbox_y + 5, SMALLBUTTON_W, SMALLBUTTON_H, TFT_WHITE, TFT_RED, TFT_BLACK, 1, 1);
		btnCancel.setLabel("CANCEL");
		btnCancel.setActive(true, true);

		spkrBeep(50);
		_drawModalListEntries(array, start_line, selected_line, linecount, listfield_top);
		int last_selected_line = selected_line;
		while (true) {
			if (btnCancel.checkPressed(true)) { // Check if CANCEL button is pressed
				cancelled = true;
				break;
			}
			#ifdef ENCODER_ENABLED
        int enc_delta = _touchProvider->getEncDelta(true);
				if (enc_delta != 0) {
					spkrTick();
					if (((selected_line + start_line) < entry_count) || (enc_delta < 0))
						selected_line += enc_delta;
					if (selected_line < 0) {
						selected_line = 0;
						if (start_line > 0) {
							start_line--;
							// Repaint when scrolling up
							_drawModalListEntries(array, start_line, selected_line, linecount, listfield_top);
						}
					} else if (selected_line >= LISTBOX_MAXLINES) {
						// End of displayed list range reached?
						selected_line = LISTBOX_MAXLINES - 1;
						start_line++;
						// Repaint when scrolling down
						_drawModalListEntries(array, start_line, selected_line, linecount, listfield_top);
					} else {
						// Repaint just the changed lines
						_drawModalListLine(array[start_line + last_selected_line], false, last_selected_line, listfield_top);
						_drawModalListLine(array[start_line + selected_line], true, selected_line, listfield_top);
					}
					last_selected_line = selected_line;
				}
				if (!digitalRead(ENCBTN_PIN))
					break;
			#endif // ENCODER_ENABLED
			if (_touchProvider->checkWithin(LISTFIELD_LEFT, listfield_top, LISTFIELD_LEFT + LISTFIELD_W, listfield_bottom)) {
				selected_line = (_touchProvider->ty - listfield_top) / LISTFIELD_H;
				if (selected_line < linecount)
					break;
			}
			delay(20);
		}
		if (cancelled)
		  spkrCancelBeep();
		else
			spkrClick(); // Play a click sound when an item is selected
		selected_item = selected_line + start_line;
		_drawModalListEntries(array, start_line, selected_line, linecount, listfield_top);
		delay(200);
		tft.fillRect(LISTBOX_X, listbox_y, LISTBOX_W, listbox_height, TFT_MEDGREY);
		#ifdef DEBUG
			DEBUG_PRINT("Item Selected: ");
			if (selected_item == entry_count || cancelled)
				DEBUG_PRINTLN("CANCEL");
			else
				DEBUG_PRINTLN(selected_item);
		#endif
		_touchProvider->waitReleased();
		if (selected_item == entry_count || cancelled) // CANCEL item or cancelled
			return -1; // CANCEL item
		else
			return selected_item;
	}

// ##############################################################################

private:
  TFT_eSPI *_tft;
  TouchProvider *_touchProvider;  // Pointer to the touch provider for touch handling
  int16_t  _x1, _y1;              // Coordinates of top-left corner of dialog box
  int16_t  _x2, _y2;              // Coordinates of bottom-right corner of dialog box
  PushButton btnCancel;
	volatile int _encdelta; // Current state of the button

	// Draws a single line in the modal list
	void _drawModalListLine(char *text, bool is_active, int line, int top) {
		uint16_t line_color = TFT_WHITE;
		top += line * LISTFIELD_H;
		if ((strcmp(text, "CANCEL")) == 0)
			line_color = TFT_RED; // CANCEL line
		if (is_active) {
			tft.fillRect(LISTFIELD_LEFT + 1, top + 1, LISTFIELD_W - 2, LISTFIELD_H - 1, line_color);
			tft.setTextColor(TFT_BLACK, line_color);
		} else {
			tft.fillRect(LISTFIELD_LEFT + 1, top + 1, LISTFIELD_W - 2, LISTFIELD_H - 1, TFT_BLACK);
			tft.setTextColor(line_color, TFT_BLACK);
		}
		tft.drawString(text, LISTFIELD_LEFT + 5, top + 2, 2);
		tft.drawRect(LISTFIELD_LEFT, top, LISTFIELD_W, LISTFIELD_H + 1, TFT_WHITE);
	}

	// Draws a list of entries in a modal dialog
	// The entries are drawn in a list format with a specified start entry and active line
	// startentry is the index of the first entry to display, active_line is the index of the currently selected line
	// count is the number of lines to display
	// The function fills the background of each line with a color based on whether it is active or not
	void _drawModalListEntries(menuArr_t array, int startentry, int active_line, int count, int top) {
		tft.setTextDatum(TL_DATUM); // top left text datum
		for (int i = 0; i < count; i++) {
			_drawModalListLine(array[startentry + i], (i == active_line), i, top);
		}
	}

};

#endif
