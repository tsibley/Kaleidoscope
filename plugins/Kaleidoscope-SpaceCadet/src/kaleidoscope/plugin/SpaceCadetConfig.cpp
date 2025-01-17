/* -*- mode: c++ -*-
 * Kaleidoscope-SpaceCadet -- Space Cadet Shift Extended
 * Copyright (C) 2016, 2017, 2018  Keyboard.io, Inc, Ben Gemperline
 * Copyright (C) 2019-2021  Keyboard.io, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SpaceCadet.h"
#include "kaleidoscope/plugin/SpaceCadet.h"

#include <Arduino.h>                       // for F, __FlashStringHelper
#include <Kaleidoscope-FocusSerial.h>      // for Focus, FocusSerial
#include <Kaleidoscope-EEPROM-Settings.h>  // for EEPROMSettings
#include <stdint.h>                        // for uint16_t, int8_t, uint8_t

#include "kaleidoscope/Runtime.h"               // for Runtime, Runtime_
#include "kaleidoscope/event_handler_result.h"  // for EventHandlerResult, EventHandlerResult::OK

namespace kaleidoscope {
namespace plugin {

EventHandlerResult SpaceCadetConfig::onSetup() {
  settings_base_ = ::EEPROMSettings.requestSlice(sizeof(SpaceCadet::settings_));

  // If our slice is uninitialized, then return early.
  if (Runtime.storage().isSliceUninitialized(settings_base_, sizeof(SpaceCadet::settings_)))
    return EventHandlerResult::OK;

  Runtime.storage().get(settings_base_, ::SpaceCadet.settings_);

  return EventHandlerResult::OK;
}

void SpaceCadetConfig::disableSpaceCadetIfUnconfigured() {
  if (Runtime.storage().isSliceUninitialized(settings_base_, sizeof(SpaceCadet::settings_)))
    ::SpaceCadet.disable();
}

EventHandlerResult SpaceCadetConfig::onFocusEvent(const char *command) {
  const char *cmd_mode    = PSTR("spacecadet.mode");
  const char *cmd_timeout = PSTR("spacecadet.timeout");

  // Note: These two calls are intentionally separate, because we want the side
  // effects. If we were to use `||`, only one of them would run.
  bool help_handled = ::Focus.handleHelp(command, cmd_mode);
  help_handled |= ::Focus.handleHelp(command, cmd_timeout);

  if (help_handled)
    return EventHandlerResult::OK;

  if (strcmp_P(command, cmd_mode) == 0) {
    if (::Focus.isEOL()) {
      ::Focus.send(::SpaceCadet.settings_.mode);
    } else {
      uint8_t mode;
      ::Focus.read(mode);
      switch (mode) {
      case SpaceCadet::Mode::ON:
        ::SpaceCadet.settings_.mode = SpaceCadet::Mode::ON;
        break;
      case SpaceCadet::Mode::NO_DELAY:
        ::SpaceCadet.settings_.mode = SpaceCadet::Mode::NO_DELAY;
        break;
      case SpaceCadet::Mode::OFF:
      default:
        ::SpaceCadet.settings_.mode = SpaceCadet::Mode::OFF;
        break;
      }

      Runtime.storage().put(settings_base_, ::SpaceCadet.settings_);
      Runtime.storage().commit();
    }
  } else if (strcmp_P(command, cmd_timeout) == 0) {
    if (::Focus.isEOL()) {
      ::Focus.send(::SpaceCadet.settings_.timeout);
    } else {
      ::Focus.read(::SpaceCadet.settings_.timeout);

      Runtime.storage().put(settings_base_, ::SpaceCadet.settings_);
      Runtime.storage().commit();
    }
  } else {
    return EventHandlerResult::OK;
  }

  return EventHandlerResult::EVENT_CONSUMED;
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::SpaceCadetConfig SpaceCadetConfig;
