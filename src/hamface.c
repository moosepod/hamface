#include <pebble.h>

/* Main file for hamface Pebble watch face.
  
   Displays local and utc time/date.

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
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
