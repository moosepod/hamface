#include <pebble.h>

/* Main file for hamface Pebble watch face.
  
   Displays local and utc time/date.
   
   Weather code adapted from https://github.com/pebble-examples/pebblekit-js-weather/

   by Matt Christensen (mchristensen@moosepod.com)
*/  

  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_temp_layer;
static TextLayer *s_utctime_layer;

static TextLayer *s_band_layer;
static TextLayer *s_day_band_layer;
static TextLayer *s_night_band_layer;

static char temperature_buffer[8];

#define KEY_TEMPERATURE 0

/// Pebble.js connectivity code
#define KEY_TEMPERATURE_C 0
#define KEY_TEMPERATURE_F 1

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE_C:
	snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
  	break;
    case KEY_TEMPERATURE_F:
        snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)t->value->int32);
        break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  text_layer_set_text(s_temp_layer,temperature_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/// General setup and clock code

// Update the local time layer from the pebble's time
static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  static char time_buffer[] = "00:00";
  static char date_buffer[] = "JAN 00";

  if(clock_is_24h_style() == true) {
    strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    strftime(time_buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  strftime(date_buffer,sizeof("JAN 00"), "%b %d", tick_time);

  text_layer_set_text(s_time_layer, time_buffer);
  text_layer_set_text(s_date_layer, date_buffer);
}

// Update the UTC time layer from the pebble's time
static void update_utc_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = gmtime(&temp);
  
  static char buffer[] = "00:00 | JAN 00"; 
  
  strftime(buffer, sizeof("00:00 | JAN 00"), "%H:%M | %b %d", tick_time);

  text_layer_set_text(s_utctime_layer, buffer);
}  

// Called when the main window is initialized
static void main_window_load(Window *window) {
  // Setup the main time layer
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  s_time_layer = text_layer_create(GRect(0,0, window_bounds.size.w,75));
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer,GTextAlignmentCenter);
  text_layer_set_text(s_time_layer, "00:00");
  
  // Setup the date layer, right below the UTC time area
  s_date_layer = text_layer_create(GRect(14,42,50,30));
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer,GTextAlignmentLeft);
  text_layer_set_text(s_date_layer, "JAN 00");
 
  // Setup the temp layer, to the right of the date layer
  s_temp_layer = text_layer_create(GRect(window_bounds.size.w-48,42,30,30));
  text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_color(s_temp_layer, GColorWhite);
  text_layer_set_text_alignment(s_temp_layer,GTextAlignmentLeft);
  text_layer_set_text(s_temp_layer, "...");

  // Setup the UTC time layer
  s_utctime_layer = text_layer_create(GRect(0,window_bounds.size.h-30,window_bounds.size.w,30));
  text_layer_set_font(s_utctime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(s_utctime_layer, GColorBlack);
  text_layer_set_text_color(s_utctime_layer, GColorWhite);
  text_layer_set_text_alignment(s_utctime_layer,GTextAlignmentCenter);
  text_layer_set_text(s_utctime_layer, "00:00 JAN 01");

  // Setup the band info layers
  s_band_layer = text_layer_create(GRect(5,75,window_bounds.size.w,60));
  text_layer_set_font(s_band_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(s_band_layer, GColorClear);
  text_layer_set_text_color(s_band_layer, GColorBlack);
  text_layer_set_text_alignment(s_band_layer,GTextAlignmentLeft);
  //text_layer_set_text(s_band_layer, "80m-40m\n30m-20m\n17m-15m\n12m-10m");
  text_layer_set_text(s_band_layer,"\n    Loading...\n    Data from hamqsl.com\n");

  s_day_band_layer = text_layer_create(GRect(65,75,30,60));
  text_layer_set_font(s_day_band_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(s_day_band_layer, GColorClear);
  text_layer_set_text_color(s_day_band_layer, GColorBlack);
  text_layer_set_text_alignment(s_day_band_layer,GTextAlignmentLeft);
//  text_layer_set_text(s_day_band_layer, "Good\nGood\nFair\nPoor");

  s_night_band_layer = text_layer_create(GRect(100,75,30,60));
  text_layer_set_font(s_night_band_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_background_color(s_night_band_layer, GColorClear);
  text_layer_set_text_color(s_night_band_layer, GColorBlack);
  text_layer_set_text_alignment(s_night_band_layer,GTextAlignmentLeft);
//  text_layer_set_text(s_night_band_layer, "Good\nGood\nFair\nPoor");

  // Add all layers to main window
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_utctime_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_utctime_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_band_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_day_band_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_night_band_layer));

  // Initial refresh of times.
  update_time();
  update_utc_time();
}

// Called when the window is destroyed.
static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_temp_layer);
  text_layer_destroy(s_utctime_layer);
  text_layer_destroy(s_band_layer);
  text_layer_destroy(s_day_band_layer);
  text_layer_destroy(s_night_band_layer);
}

// Update any information that needs updating when time changes
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_utc_time();

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
     dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void init() {
  // Create the main window and store pointer
  s_main_window = window_create();

  // Connect the load and unload handlers to the window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Display the main window on the watch, true will animate its arrival
  window_stack_push(s_main_window, true);
  
  // Register so our tick handler is called every minute
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
