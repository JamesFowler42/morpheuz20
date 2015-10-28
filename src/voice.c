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

#ifdef VOICE_SUPPORTED

// Phrase buffer
#define PHRASE_BUFFER_LEN 30 
                      
// Phrases
#define VOICE_BED_TIME "bedtime"
#define VOICE_BED_TIME_ALARM "bedtimealarm"
#define VOICE_BED_TIME_NO_ALARM "bedtimenoalarm"
#define VOICE_BED_TIME_PRESET_ONE "bedtimepresetone"
#define VOICE_BED_TIME_PRESET_TWO "bedtimepresettwo"
#define VOICE_BED_TIME_PRESET_THREE "bedtimepresetthre"
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
} VoiceDef;

// Phrases definitions
static VoiceDef voice_def[] = { 
  { VOICE_BED_TIME, reset_sleep_period },
  { VOICE_BED_TIME_ALARM, reset_with_alarm_on },
  { VOICE_BED_TIME_NO_ALARM, reset_with_alarm_off },
  { VOICE_BED_TIME_PRESET_ONE, reset_with_preset_one },
  { VOICE_BED_TIME_PRESET_TWO, reset_with_preset_two },   
  { VOICE_BED_TIME_PRESET_THREE, reset_with_preset_three },
  { VOICE_POWER_NAP, toggle_power_nap },
  { VOICE_SNOOZE, snooze_alarm},
  { VOICE_CANCEL, cancel_alarm} };

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
    if (('A' <= a) && (a <= 'Z')) 
      a = 'a' + (a - 'A');
    *ob++ = a;
  }
}

/*
 * Does a case, duplicate blind and space blind comparision
 */
static int strncmp_ignore_spaces_and_case (char *s1, char *s2, size_t n) {
  if (n == 0)
    return 0;
  
  build_compare_string(s2buff, s2, n);
  
  return strncmp(s1, s2buff, n);
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
    show_notice(RESOURCE_ID_NOTICE_SORRY_DIDNT_UNDERSTAND);
    return;
  }
  
  // If we were lucky then we try to match the transcription to a menu item
  dictation_session_stop(session);
  
  LOG_INFO("dictation got %s", transcription);
  bool found = false;
  for (uint8_t i = 0; i < ARRAY_LENGTH(voice_def); i++) {
      if (strncmp_ignore_spaces_and_case(voice_def[i].phrase, transcription, PHRASE_BUFFER_LEN) == 0) {
        
        // If we get a match then fire the action
        LOG_INFO("Invoking phrase %s", voice_def[i].phrase);
        respond_with_vibe(true);
        voice_def[i].action();
        found = true;
        break;
      }
  }
  
  // Feedback if we didn't find anything
  if (!found) {
    respond_with_vibe(false);
    show_notice(RESOURCE_ID_NOTICE_SORRY_DIDNT_UNDERSTAND);
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
    dictation_session_enable_confirmation(ds, false);
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

