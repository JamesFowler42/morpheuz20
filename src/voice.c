/* 
 * Morpheuz Sleep Monitor
 *
 * Copyright (c) 2013-2015 James Fowler
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "pebble.h"
#include "morpheuz.h"
#include "language.h"

#ifdef VOICE_SUPPORTED

// Phrase buffer
#define PHRASE_BUFFER_LEN 30 

// Local actions
static void reset_with_alarm_on();
static void reset_with_alarm_off();
static void reset_with_preset_one();
static void reset_with_preset_two();
static void reset_with_preset_three();

// Invoke a voice item
typedef void (*VoiceSelectAction)(void);

static bool voice_system_active = false;
static DictationSession *ds = NULL;

// Vibe back good
static uint32_t const good_segments[] = { 80, 100, 80, 100, 80 };
VibePattern good_pat = {
  .durations = good_segments,
  .num_segments = ARRAY_LENGTH(good_segments),
};

// Vibe back bad
static uint32_t const bad_segments[] = { 200, 100, 200, 100, 200 };
VibePattern bad_pat = {
  .durations = bad_segments,
  .num_segments = ARRAY_LENGTH(bad_segments),
};

/*
 * Need a feedback vibe pattern
 */
static void respond_with_vibe(bool good) {
  vibes_enqueue_custom_pattern(good ? good_pat : bad_pat);
}

/*
 * Determines if the transscript contains a word or not
 */
static bool contains(char *trans, char *match, size_t n, int8_t *calc_length) {
  
  // Find first matching letter
  char *tp = trans;
  char *mp = match;
  
  char pret = ' ';
  for (uint8_t i = 0; i < n; i++) {
    char t = *tp++;
    char m = *mp++;
    // If we've reached the end of trans string, or the end of the word and we've reached the end of the match
    // Then we've matched all letters. This is a good thing
    if ( m == '\0' && (t == '\0' || t == ' ') && pret == ' ') {
      *calc_length += strlen(match) + 1;
      LOG_DEBUG("matched %s", match);
      return true;
    }
    // End of transcript - stop
    if ( t == '\0')
      break;
    // Letter mismatch is time to reset the match string. Remember the character before. Has to be a space to exit.
    if (tolower(t) != m) {
      mp = match;
      pret = *(tp - 1);
    } 
  }
  return false;
}

/*
 * Work out what the phrase means
 */
static VoiceSelectAction determine_action(char *transcription, bool *vibe) {
  VoiceSelectAction action = NULL;
  *vibe = false;
  
  // Locate key words
  int8_t calc_length = -1;
  bool b_bed = contains(transcription, "bed", PHRASE_BUFFER_LEN, &calc_length);
  bool b_time = contains(transcription, "time", PHRASE_BUFFER_LEN, &calc_length);
  bool b_bedtime = contains(transcription, "bedtime", PHRASE_BUFFER_LEN, &calc_length);
  bool b_alarm = contains(transcription, "alarm", PHRASE_BUFFER_LEN, &calc_length);
  bool b_with = contains(transcription, "with", PHRASE_BUFFER_LEN, &calc_length); // Noise word
  bool b_without = contains(transcription, "without", PHRASE_BUFFER_LEN, &calc_length);
  bool b_no = contains(transcription, "no", PHRASE_BUFFER_LEN, &calc_length);
  bool b_preset = contains(transcription, "preset", PHRASE_BUFFER_LEN, &calc_length);
  bool b_presets = contains(transcription, "presets", PHRASE_BUFFER_LEN, &calc_length);
  bool b_one = contains(transcription, "one", PHRASE_BUFFER_LEN, &calc_length);
  bool b_two = contains(transcription, "to", PHRASE_BUFFER_LEN, &calc_length);
  bool b_three = contains(transcription, "three", PHRASE_BUFFER_LEN, &calc_length);
  bool b_powernap = contains(transcription, "powernap", PHRASE_BUFFER_LEN, &calc_length);
  bool b_power = contains(transcription, "power", PHRASE_BUFFER_LEN, &calc_length);
  bool b_nap = contains(transcription, "nap", PHRASE_BUFFER_LEN, &calc_length);
  bool b_snooze = contains(transcription, "snooze", PHRASE_BUFFER_LEN, &calc_length);
  bool b_stop = contains(transcription, "stop", PHRASE_BUFFER_LEN, &calc_length);
  bool b_cancel = contains(transcription, "cancel", PHRASE_BUFFER_LEN, &calc_length);
  
  // Length cross check - make sure there are not loads more words than we recognise
  int8_t true_length = strlen(transcription);
  if (true_length != calc_length) {
    LOG_DEBUG("Calc length = %d, true length = %d", calc_length, true_length);
    return action;
  }
  
  // Handle compound words or misheard words
  b_bedtime = b_bedtime || (b_bed && b_time);
  b_powernap = b_powernap || (b_power && b_nap);
  b_preset = b_preset || b_presets;
  
  // Allocate meaning based on word appearance
  if (b_bedtime) {
    if (b_alarm) {
      if (b_without || b_no) {
        LOG_DEBUG("Invoking action for 'bedtime without|no alarm'");
        action = reset_with_alarm_off;
      } else if (b_one) {
        LOG_DEBUG("Invoking action for 'bedtime [with] alarm one'");
        action = reset_with_preset_one;
      } else if (b_two) {
        LOG_DEBUG("Invoking action for 'bedtime [with] alarm two'");
        action = reset_with_preset_two;
      } else if (b_three) {
        LOG_DEBUG("Invoking action for 'bedtime [with] alarm three'");
        action = reset_with_preset_three;
      } else {
        LOG_DEBUG("Invoking action for 'bedtime [with] alarm'");
        action = reset_with_alarm_on;
      }
    } else if (b_preset) {
      if (b_one) {
        LOG_DEBUG("Invoking action for 'bedtime [with] preset one'");
        action = reset_with_preset_one;
      } else if (b_two) {
        LOG_DEBUG("Invoking action for 'bedtime [with] preset two'");
        action = reset_with_preset_two;
      } else if (b_three) {
        LOG_DEBUG("Invoking action for 'bedtime [with] preset three'");
        action = reset_with_preset_three;
      }
    } else {
      LOG_DEBUG("Invoking action for 'bedtime'");
      action = reset_sleep_period;
    }
  } else if (b_powernap) {
     LOG_DEBUG("Invoking action for 'powernap'");
     action = toggle_power_nap;
     *vibe = true;
  } else if (b_alarm) {
    if (b_snooze) {
      LOG_DEBUG("Invoking action for 'snooze alarm'");
      action = snooze_alarm;
      *vibe = true;
    } else if (b_stop || b_cancel) {
      LOG_DEBUG("Invoking action for 'stop|cancel alarm'");
      action = cancel_alarm;
      *vibe = true;
    }
  }
  
  // Return the action
  return action;
}

/*
 * Once dication has happened this fires, for better or worse
 */
static void voice_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) {
  
  // If for worse then this is where we go
  if (status != DictationSessionStatusSuccess) {
    LOG_WARN("dictation failed");
    dictation_session_stop(session);
    voice_system_active = false;
    respond_with_vibe(false);
    show_notice(RESOURCE_ID_NOTICE_VOICE_FAILED);
    return;
  }
  
  // If we were lucky then we try to match the transcription to a menu item
  dictation_session_stop(session);
  
  LOG_INFO("dictation got %s", transcription);
  bool vibe;
  VoiceSelectAction action = determine_action(transcription, &vibe);
  
  if (action != NULL) {
    // If we get a match then fire the action
    if (vibe)
      respond_with_vibe(true);
    action();
  } else {
    // Feedback we didn't find anything
    respond_with_vibe(false);
    show_notice_with_message(RESOURCE_ID_VOICE_DIDNT_UNDERSTAND, transcription);
  }
  
  // Allow comms
  voice_system_active = false; 
}

/*
 * Start voice control
 */
EXTFN void voice_control() {
  
  voice_system_active = true;
  if (ds == NULL) {
    ds = dictation_session_create(PHRASE_BUFFER_LEN, voice_callback, NULL);
    #ifndef TESTING_BUILD
      dictation_session_enable_confirmation(ds, false);
    #endif
    dictation_session_enable_error_dialogs(ds, false);
  }
  if (ds != NULL) {
    dictation_session_start(ds);
  } else {
    voice_system_active = false;
    respond_with_vibe(false);
    show_notice(RESOURCE_ID_NOTICE_VOICE_UNAVAILABLE);
  }
  
}

/*
 * Close away voice control
 */
EXTFN void tidy_voice() {
    if (ds != NULL) {
      dictation_session_destroy(ds);
    }
}

/* 
 * Let the comms system know the voice system is active
 */
EXTFN bool is_voice_system_active() {
  return voice_system_active;
}

/*
 * Bed time with the alarm turned on
 */
static void reset_with_alarm_on() {
  if (!get_config_data()->smart) {
    get_config_data()->smart = true;
    trigger_config_save();
    set_smart_status();
  }
  reset_sleep_period();
}

/*
 * Bed time with the alarm turned off
 */
static void reset_with_alarm_off() {
  if (get_config_data()->smart) {
    get_config_data()->smart = false;
    trigger_config_save();
    set_smart_status();
  }
  reset_sleep_period();
}

/*
 * Bed time using preset one alarm settings
 */
static void reset_with_preset_one() {
  set_using_preset(0);
  reset_sleep_period();
}

/*
 * Bed time using preset two alarm settings
 */
static void reset_with_preset_two() {
  set_using_preset(1);
  reset_sleep_period();
}

/*
 * Bed time using preset three alarm settings
 */
static void reset_with_preset_three() {
  set_using_preset(2);
  reset_sleep_period();
}

/*
 * Turn off voice
 */
EXTFN void voice_system_inactive() {
    voice_system_active = false; 
}

#endif
