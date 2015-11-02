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

#define MAX_LEVENSHTEIN_DISTANCE 5
                      
// Phrases
#define VOICE_BED_TIME "bedtime"
#define VOICE_BED_TIME_WITH_ALARM "bedtimewithalarm"
#define VOICE_BED_TIME_ALARM "bedtimealarm"
#define VOICE_BED_TIME_WITHOUT_ALARM "bedtimewithoutalarm"
#define VOICE_BED_TIME_NO_ALARM "bedtimenoalarm"
#define VOICE_BED_TIME_PRESET_ONE "bedtimepresetone"
#define VOICE_BED_TIME_PRESET_TWO "bedtimepresetto"
#define VOICE_BED_TIME_PRESET_THREE "bedtimepresetthre"
#define VOICE_BED_TIME_ALARM_ONE "bedtimealarmone"
#define VOICE_BED_TIME_ALARM_TWO "bedtimealarmto"
#define VOICE_BED_TIME_ALARM_THREE "bedtimealarmthre"
#define VOICE_BED_TIME_WITH_PRESET_ONE "bedtimewithpresetone"
#define VOICE_BED_TIME_WITH_PRESET_TWO "bedtimewithpresetto"
#define VOICE_BED_TIME_WITH_PRESET_THREE "bedtimewithpresetthre"
#define VOICE_BED_TIME_WITH_ALARM_ONE "bedtimewithalarmone"
#define VOICE_BED_TIME_WITH_ALARM_TWO "bedtimewithalarmto"
#define VOICE_BED_TIME_WITH_ALARM_THREE "bedtimewithalarmthre"
#define VOICE_POWER_NAP "powernap"
#define VOICE_SNOOZE "snoozealarm"
#define VOICE_CANCEL "stopalarm"

// Local actions
static void reset_with_alarm_on();
static void reset_with_alarm_off();
static void reset_with_preset_one();
static void reset_with_preset_two();
static void reset_with_preset_three();

// Invoke a voice item
typedef void (*VoiceSelectAction)(void);

// Define a voice item
typedef struct {
  char *phrase;
  VoiceSelectAction action;
  bool vibe;
} VoiceDef;

// Phrases definitions
static VoiceDef voice_def[] = { 
  { VOICE_BED_TIME, reset_sleep_period, false },
  { VOICE_BED_TIME_ALARM, reset_with_alarm_on, false },
  { VOICE_BED_TIME_WITH_ALARM, reset_with_alarm_on, false },
  { VOICE_BED_TIME_WITHOUT_ALARM, reset_with_alarm_off, false },
  { VOICE_BED_TIME_NO_ALARM, reset_with_alarm_off, false },
  { VOICE_BED_TIME_PRESET_ONE, reset_with_preset_one, false },
  { VOICE_BED_TIME_PRESET_TWO, reset_with_preset_two, false },   
  { VOICE_BED_TIME_PRESET_THREE, reset_with_preset_three, false },
  { VOICE_BED_TIME_ALARM_ONE, reset_with_preset_one, false },
  { VOICE_BED_TIME_ALARM_TWO, reset_with_preset_two, false },   
  { VOICE_BED_TIME_ALARM_THREE, reset_with_preset_three, false },
  { VOICE_BED_TIME_WITH_PRESET_ONE, reset_with_preset_one, false },
  { VOICE_BED_TIME_WITH_PRESET_TWO, reset_with_preset_two, false },   
  { VOICE_BED_TIME_WITH_PRESET_THREE, reset_with_preset_three, false },
  { VOICE_BED_TIME_WITH_ALARM_ONE, reset_with_preset_one, false },
  { VOICE_BED_TIME_WITH_ALARM_TWO, reset_with_preset_two, false },   
  { VOICE_BED_TIME_WITH_ALARM_THREE, reset_with_preset_three, false },
  { VOICE_POWER_NAP, toggle_power_nap, true },
  { VOICE_SNOOZE, snooze_alarm, true},
  { VOICE_CANCEL, cancel_alarm, true} };

static bool voice_system_active = false;
static DictationSession *ds = NULL;
static char s2buff[PHRASE_BUFFER_LEN];

// Vibe back good
static const uint32_t const good_segments[] = { 80, 100, 80, 100, 80 };
VibePattern good_pat = {
  .durations = good_segments,
  .num_segments = ARRAY_LENGTH(good_segments),
};

// Vibe back bad
static const uint32_t const bad_segments[] = { 200, 100, 200, 100, 200 };
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
 * Generate a compare string, ignoring duplicate letters, spaces and switches to lowercase
 */
static void build_compare_string(char *out, char *in, size_t n) {
  // Clear buffer
  memset(out, 0, n);
  
  // Copy ignoring spaces and turning uppercase to lower. Ignores duplicate letters too.
  char *ob = out;
  char *ib = in;
  char l = '$';
  for (uint8_t i = 0; i < n; i++) {
    char a = *ib++;
    if (a == l) {
      continue;
    }
    l = a;
    if (a == ' ')
      continue;
    if (a == '\0')
      break;
    *ob++ = tolower(a);
  }
}

/*
 * Does a case, duplicate blind and space blind comparision
 */
static uint32_t fuzzy_compare (char *s1, char *s2, size_t n) {
  build_compare_string(s2buff, s2, n);
  return levenshtein(s1, s2buff);
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
  uint8_t best_i = 99;
  uint32_t best_match = 99;
  for (uint8_t i = 0; i < ARRAY_LENGTH(voice_def); i++) {
      uint32_t match = fuzzy_compare(voice_def[i].phrase, transcription, PHRASE_BUFFER_LEN);
      if (match < MAX_LEVENSHTEIN_DISTANCE) {
        if (match < best_match) {
          best_match = match;
          best_i = i;
        }
      }
  }
  
  if (best_i != 99) {
    // If we get a match then fire the action
    LOG_INFO("Invoking phrase %s", voice_def[best_i].phrase);
    if (voice_def[best_i].vibe)
      respond_with_vibe(true);
    voice_def[best_i].action();
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

#endif

