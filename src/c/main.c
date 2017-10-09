#include <pebble.h>

#define SETTINGS_KEY 0
#define SETTINGS_VERSION_KEY 1
#define D_MAX_NUM_FRAMES 40

static Window *s_main_window;
static Layer *s_window_layer, *s_foreground_layer;
static char s_time_text[6] = "00:00", s_battery_text[5] = "100%", s_date_text[12], s_steps_text[7] = "034617", s_temperature_text[8];
static GFont s_stencil_font_large, s_stencil_font_small, s_stencil_font_tiny;
static BitmapLayer *s_background_layer, *s_animation_layer, *s_weather_layer, *s_connection_layer;
static GBitmap *s_background_bitmap, *s_animation_bitmap, *s_weather_bitmap, *s_connection_bitmap;
static bool is_animating = false;
static int frames_elapsed = 0;
static AppTimer *s_weather_timer;

/************************************************************************* Settings */

typedef struct ClaySettings {
	bool vibrate_on_disconnect;
	bool hourly_vibration;
	bool health_enabled;
	bool weather_enabled;
	bool animate_on_shake;
} ClaySettings;

static ClaySettings settings;

int settings_version = 1;

static void settings_init() {
	settings.vibrate_on_disconnect = true;
	settings.hourly_vibration = false;
	settings.health_enabled = true;
	settings.weather_enabled = true;
	settings.animate_on_shake = true;
	persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void save_settings() {
	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
	persist_write_int(SETTINGS_VERSION_KEY, 1);
}

/************************************************************************* Weather */

static void request_weather() {
	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	dict_write_uint8(iter, 0, 0);
	
	app_message_outbox_send();
	
	s_weather_timer = app_timer_register(30 * 1000 * SECONDS_PER_MINUTE, request_weather, NULL);
}

static void draw_weather(int icon) {
	if (s_weather_bitmap) {
		gbitmap_destroy(s_weather_bitmap);
	}
	
	switch (icon) {
		case 1:
			s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SUNNY_ICON);
			break;
		case 2:
			s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PARTLY_SUNNY_ICON);
			break;
		case 3:
			s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CLOUDY_ICON);
			break;
		case 4:
			s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_RAIN_ICON);
			break;
		case 5:
			s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_THUNDERSTORM_ICON);
			break;
		case 6:
			s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SNOW_ICON);
			break;
		case 7:
			s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NIGHT_CLEAR_ICON);
			break;
		case 8:
			s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NIGHT_CLOUDY_ICON);
			break;
		default:
			break;
	}
	if (icon != 0) {
		bitmap_layer_set_bitmap(s_weather_layer, s_weather_bitmap);
	}
}

/************************************************************************* Drawing Procedures */

static void foreground_update_proc(Layer *s_foreground_layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(s_window_layer);
	
	graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
	
	#if PBL_RECT
	GSize time_text_bounds = graphics_text_layout_get_content_size(s_time_text, s_stencil_font_large, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_time_text, s_stencil_font_large, GRect(92 - 0.5 * time_text_bounds.w, 10, time_text_bounds.w, time_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

	GSize date_text_bounds = graphics_text_layout_get_content_size(s_date_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_date_text, s_stencil_font_tiny, GRect(92 - 0.5 * date_text_bounds.w, 41, date_text_bounds.w, date_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	GSize battery_text_bounds = graphics_text_layout_get_content_size(s_battery_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentRight);
	graphics_draw_text(ctx, s_battery_text, s_stencil_font_tiny, GRect(113 - battery_text_bounds.w, 2, battery_text_bounds.w, battery_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	
	if (settings.weather_enabled) {
		GSize temperature_text_bounds = graphics_text_layout_get_content_size(s_temperature_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentLeft);
		graphics_draw_text(ctx, s_temperature_text, s_stencil_font_tiny, GRect(55, 2, temperature_text_bounds.w, temperature_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	}
	#else
	GSize time_text_bounds = graphics_text_layout_get_content_size(s_time_text, s_stencil_font_large, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_time_text, s_stencil_font_large, GRect(90 - 0.5 * time_text_bounds.w, 17, time_text_bounds.w, time_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

	GSize date_text_bounds = graphics_text_layout_get_content_size(s_date_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_date_text, s_stencil_font_tiny, GRect(90 - 0.5 * date_text_bounds.w, 47, date_text_bounds.w, date_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	GSize battery_text_bounds = graphics_text_layout_get_content_size(s_battery_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentRight);
	graphics_draw_text(ctx, s_battery_text, s_stencil_font_tiny, GRect(90 - battery_text_bounds.w, 8, battery_text_bounds.w, battery_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);	
	
	if (settings.weather_enabled)
		graphics_context_set_fill_color(ctx, GColorRed);
		graphics_fill_rect(ctx, GRect(142, 74, 26, 25), 3, GCornersAll);
		GSize temperature_text_bounds = graphics_text_layout_get_content_size(s_temperature_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
		graphics_context_set_text_color(ctx, GColorBlack);
		graphics_draw_text(ctx, s_temperature_text, s_stencil_font_tiny, GRect(142 + 13 - 0.5 * temperature_text_bounds.w, 74 + 10 - 0.5 * temperature_text_bounds.h, temperature_text_bounds.w, temperature_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	#endif
	
	graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorOrange, GColorWhite));
	
	#if PBL_RECT
	GSize step_text_bounds = graphics_text_layout_get_content_size(s_steps_text, s_stencil_font_small, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_steps_text, s_stencil_font_small, GRect(72 - 0.5 * step_text_bounds.w, 147, step_text_bounds.w, step_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	#else
	GSize step_text_bounds = graphics_text_layout_get_content_size(s_steps_text, s_stencil_font_small, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_steps_text, s_stencil_font_small, GRect(90 - 0.5 * step_text_bounds.w, 138, step_text_bounds.w, step_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	#endif
}

/************************************************************************* Time Updaters */

static void update_time() {
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  strftime(s_time_text, sizeof(s_time_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	strftime(s_date_text, sizeof(s_date_text), "%F", tick_time);
	
	if (tick_time->tm_min == 0) {
		if (settings.hourly_vibration) {
			vibes_short_pulse();
		}
	}
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
	layer_mark_dirty(s_foreground_layer);
}

/************************************************************************* Battery Updater */

static void battery_handler() {
	BatteryChargeState state = battery_state_service_peek();
	if (!state.is_charging) {
		int s_battery_level = state.charge_percent;
		snprintf(s_battery_text, sizeof(s_battery_text), "%d", s_battery_level);
	} else {
		snprintf(s_battery_text, sizeof(s_battery_text), "CHG");
	}
	layer_mark_dirty(s_foreground_layer);
}

/************************************************************************* Animation Updater */

static void timer_handler(void * context) {
	uint32_t frame_delay = frames_elapsed > 22 ? 75 : 100;
	
	gbitmap_destroy(s_animation_bitmap);
	
	switch(frames_elapsed) {
		case 1:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_2);
			break;
		case 2:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_3);
			break;
		case 3:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_4);
			break;
		case 4:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_5);
			break;
		case 5:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_6);
			break;
		case 6:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_7);
			break;
		case 7:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_8);
			break;
		case 8:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_9);
			break;
		case 9:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_10);
			break;
		case 10:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_11);
			break;
		case 11:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_12);
			break;
		case 12:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_13);
			break;
		case 13:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_14);
			break;
		case 14:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_15);
			break;
		case 15:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_16);
			break;
		case 16:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_17);
			break;
		case 17:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_18);
			break;
		case 18:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_19);
			break;
		case 19:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_20);
			break;
		case 20:
			s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_21);
			break;
		case D_MAX_NUM_FRAMES:
			bitmap_layer_destroy(s_animation_layer);
			is_animating = false;
			break;
		default:
			if (frames_elapsed % 2 == 0) {
				s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_23);
			} else {
				s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_22);
			}
			break;
	}
	
	bitmap_layer_set_bitmap(s_animation_layer, s_animation_bitmap);
	frames_elapsed++;
	
	if (frames_elapsed <= D_MAX_NUM_FRAMES) {
		app_timer_register(frame_delay, timer_handler, NULL);
	}
}

static void start_animation() {
		if (!is_animating) {
		is_animating = true;
		frames_elapsed = 1;

		#if PBL_RECT
		s_animation_layer = bitmap_layer_create(GRect(5, 64, 134, 83));
		#else
		s_animation_layer = bitmap_layer_create(GRect(39, 67, 101, 67));
		#endif
		
		s_animation_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FRAME_1);
		bitmap_layer_set_bitmap(s_animation_layer, s_animation_bitmap);
		layer_add_child(bitmap_layer_get_layer(s_background_layer), bitmap_layer_get_layer(s_animation_layer));

		app_timer_register(100, timer_handler, NULL);
	}
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
	if (settings.animate_on_shake) {
		start_animation();
	}
}

/************************************************************************* Bluetooth Updater */

static void bluetooth_callback(bool connected) {
	if (!connected) {
		if (settings.vibrate_on_disconnect) {
			vibes_long_pulse();
		}
		#if PBL_RECT
		s_connection_layer = bitmap_layer_create(GRect(6, 47, 37, 11));
		#else
		s_connection_layer = bitmap_layer_create(GRect(63, 164, 55, 6));
		#endif
		s_connection_bitmap = gbitmap_create_with_resource(RESOURCE_ID_DISCONNECTED_ICON);
		bitmap_layer_set_bitmap(s_connection_layer, s_connection_bitmap);
		layer_add_child(bitmap_layer_get_layer(s_background_layer), bitmap_layer_get_layer(s_connection_layer));
	} else {
		layer_remove_from_parent(bitmap_layer_get_layer(s_connection_layer));
	}
}

static void bluetooth_init() {
	bluetooth_callback(connection_service_peek_pebble_app_connection());
}

/************************************************************************* Health Updater */

#if defined(PBL_HEALTH)
static void update_step_count() {
	HealthValue value = health_service_sum_today(HealthMetricStepCount);
	snprintf(s_steps_text, sizeof(s_steps_text), "%06d", (int)value);
}

static void health_handler(HealthEventType event, void *context) {
	update_step_count();
	layer_mark_dirty(s_foreground_layer);
}
#endif

/************************************************************************* Setup and Teardown */

static void initialize_ui() {
	GRect bounds = layer_get_bounds(s_window_layer);
	
	s_background_layer = bitmap_layer_create(bounds);
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_background_layer));
	
	s_foreground_layer = layer_create(bounds);
	layer_set_update_proc(s_foreground_layer, foreground_update_proc);
	layer_add_child(window_get_root_layer(s_main_window), s_foreground_layer);
	
	#if PBL_RECT
	s_weather_layer = bitmap_layer_create(GRect(6, 2, 37, 38));
	#else
	s_weather_layer = bitmap_layer_create(GRect(142, 104, 26, 25));
	#endif
	if (settings.weather_enabled) {
		s_weather_timer = app_timer_register(30 * 100 * SECONDS_PER_MINUTE, request_weather, NULL);
		layer_add_child(bitmap_layer_get_layer(s_background_layer), bitmap_layer_get_layer(s_weather_layer));
	}
}

static void main_window_load(Window *window) {
	s_window_layer = window_get_root_layer(window);
	initialize_ui();
	update_time();
	battery_handler();
	bluetooth_init();
	#if defined(PBL_HEALTH)
	if (settings.health_enabled) {
		update_step_count();
	}
	#endif
	layer_mark_dirty(s_foreground_layer);
	start_animation();
}

static void main_window_unload(Window *window) {
	if (is_animating) {
		gbitmap_destroy(s_animation_bitmap);
		bitmap_layer_destroy(s_animation_layer);
	}
	
	if (s_weather_bitmap) {
		gbitmap_destroy(s_weather_bitmap);
	}
	
	if (s_weather_layer) {
		bitmap_layer_destroy(s_weather_layer);
	}
	
	app_timer_cancel(s_weather_timer);
	
	gbitmap_destroy(s_connection_bitmap);
	bitmap_layer_destroy(s_connection_layer);
	
	gbitmap_destroy(s_background_bitmap);
	bitmap_layer_destroy(s_background_layer);
	
	layer_destroy(s_foreground_layer);
	
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	#if defined(PBL_HEALTH)
	health_service_events_unsubscribe();
	#endif
	accel_tap_service_unsubscribe();
}

/************************************************************************* Communication */

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_temperature);
	Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_conditions);
	Tuple *disconnect_enabled_tuple = dict_find(iterator, MESSAGE_KEY_disconnectEnabled);
	
	if (temp_tuple && conditions_tuple) {
		snprintf(s_temperature_text, sizeof(s_temperature_text), "%s", temp_tuple->value->cstring);
		layer_mark_dirty(s_foreground_layer);
		draw_weather(conditions_tuple->value->int8);
	} else if (disconnect_enabled_tuple) {
		Tuple *hourly_vibration_tuple = dict_find(iterator, MESSAGE_KEY_hourlyVibrationEnabled);
		Tuple *health_enabled_tuple = dict_find(iterator, MESSAGE_KEY_healthEnabled);
		Tuple *weather_enabled_tuple = dict_find(iterator, MESSAGE_KEY_weatherEnabled);
		Tuple *animate_on_shake_tuple = dict_find(iterator, MESSAGE_KEY_animateOnShake);
		
		settings.vibrate_on_disconnect = disconnect_enabled_tuple->value->int32 == 1;
		settings.hourly_vibration = hourly_vibration_tuple->value->int32 == 1;
		settings.health_enabled = health_enabled_tuple->value->int32 == 1;
		settings.weather_enabled = weather_enabled_tuple->value->int32 == 1;
		settings.animate_on_shake = animate_on_shake_tuple->value->int32 == 1;
		
		save_settings();
		
		if (settings.weather_enabled) {
			request_weather();
			layer_add_child(bitmap_layer_get_layer(s_background_layer), bitmap_layer_get_layer(s_weather_layer));
		} else {
			snprintf(s_temperature_text, sizeof(s_temperature_text), " ");
			layer_remove_from_parent(bitmap_layer_get_layer(s_weather_layer));
		}
		
		if (settings.animate_on_shake) {
			accel_tap_service_subscribe(accel_tap_handler);
		} else {
			accel_tap_service_unsubscribe();
		}
		
		#if defined(PBL_HEALTH)
		if (settings.health_enabled) {
			health_service_events_subscribe(health_handler, NULL);
		} else {
			health_service_events_unsubscribe();
			snprintf(s_steps_text, sizeof(s_steps_text), "034617");
		}
		#endif
		
		layer_mark_dirty(s_foreground_layer);
	}
}

/************************************************************************* Main app Logic */

static void init() {
	app_message_register_inbox_received(inbox_received_callback);
	app_message_open(256, 128);
	
	settings_init();
	
	// Create window and load fonts
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	s_stencil_font_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STENCIL_30));
	s_stencil_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STENCIL_16));
	s_stencil_font_tiny = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STENCIL_12));
	
	// Subscribe to services
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	battery_state_service_subscribe(battery_handler);
	if (settings.animate_on_shake) {
		accel_tap_service_subscribe(accel_tap_handler);
	}
	connection_service_subscribe((ConnectionHandlers) {
		.pebble_app_connection_handler = bluetooth_callback
	});
	#if defined(PBL_HEALTH)
	if (settings.health_enabled) {
		health_service_events_subscribe(health_handler, NULL);
	}
	#endif
	
	window_stack_push(s_main_window, true);
}

static void deinit() {
	fonts_unload_custom_font(s_stencil_font_large);
	fonts_unload_custom_font(s_stencil_font_small);
	fonts_unload_custom_font(s_stencil_font_tiny);
	window_destroy(s_main_window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}