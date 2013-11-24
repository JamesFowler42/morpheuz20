#include "pebble.h"
#include "morpheuz.h"

#define SAMPLES_IN_ONE_MINUTE 24
	
static AppSync sync;
static uint8_t sync_buffer[BUFF_SIZE];

uint16_t one_minute_biggest = 0;
uint8_t sample_sets = 0;

#define ALARM_MAX 30
uint8_t alarm_count;

/*
 * Fire alarm
 */
static void fire_alarm() {
	alarm_count = 0;
	reset_tick_service(true);
}

/*
 * Do the alarm if needed
 */
void do_alarm() {
	if (alarm_count >= ALARM_MAX) {
		reset_tick_service(false);
		return;
	}
	vibes_double_pulse();
	vibes_long_pulse();
	alarm_count++;
}

/*
 * Error callback from sync
 */
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

/*
 * Success callback - actually we do nothing
 */
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  int32_t alarm_now = 0;
  switch (key) {
    case ALARM:
      alarm_now = new_tuple->value->int32;
	  if (alarm_now == 1) {
		  APP_LOG(APP_LOG_LEVEL_DEBUG, "firing alarm");
		  fire_alarm();
	  }
      break;
  }
}

/*
 * Pass our sample to javascript for storage
 */
static void send_cmd(uint16_t biggest) {

  Tuplet value = TupletInteger(BIGGEST, biggest);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

/*
 * Store our samples from each group until we have a minute's worth
 */
void store_sample(uint16_t biggest) {
	if (biggest > one_minute_biggest)
		one_minute_biggest = biggest;
	sample_sets++;
	if (sample_sets > SAMPLES_IN_ONE_MINUTE) {
		send_cmd(one_minute_biggest);
		sample_sets = 0;
		one_minute_biggest = 0;
	}
}

/*
 * Can't be bothered to play with negative numbers
 */
uint16_t scale_accel(int16_t val) {
	int16_t retval = 4000 + val;
	if (retval < 0)
		retval = 0;
	return retval;
}

/*
 * Process accelerometer data
 */
void accel_data_handler(AccelData *data, uint32_t num_samples) {

	// Average the data
	uint32_t avg_x = 0;
	uint32_t avg_y = 0;
	uint32_t avg_z = 0;
	AccelData *dx = data;
	for (uint32_t i = 0; i < num_samples; i++, dx++) {
		avg_x = avg_x + scale_accel(dx->x);
		avg_y = avg_y + scale_accel(dx->y);
		avg_z = avg_z + scale_accel(dx->z);
	}
	avg_x = avg_x / num_samples;
	avg_y = avg_y / num_samples;
	avg_z = avg_z / num_samples;

	// Work out deviations
	uint16_t biggest = 0;
	AccelData *d = data;
	for (uint32_t i = 0; i < num_samples; i++, d++) {
		uint16_t x = scale_accel(d->x) ;
		uint16_t y = scale_accel(d->y) ;
		uint16_t z = scale_accel(d->z) ;

		if (x < avg_x)
			x = avg_x - x;
		else
			x = x - avg_x;

		if (y < avg_y)
			y = avg_y - y;
		else
			y = y - avg_y;

		if (z < avg_z)
			z = avg_z - z;
		else
			z = z - avg_z;

		// Store the largest deviation in the period
		if (x > biggest) biggest = x;
		if (y > biggest) biggest = y;
		if (z> biggest) biggest = z;

	}
	store_sample(biggest);
}

/*
 * Initialise comms and accelerometer
 */
void init_morpheuz() {
	
	  alarm_count = ALARM_MAX;

	  const int inbound_size = BUFF_SIZE;
	  const int outbound_size = BUFF_SIZE;

	  app_message_open(inbound_size, outbound_size);

	  Tuplet initial_values[] = {
		  TupletInteger(BIGGEST, 0),
		  TupletInteger(ALARM, 0),
	  };

	  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
	      sync_tuple_changed_callback, sync_error_callback, NULL);

	  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
	  accel_data_service_subscribe(25, &accel_data_handler);
}

/*
 * Close down accelerometer
 */
void deinit_morpheuz() {
	accel_data_service_unsubscribe();
}
