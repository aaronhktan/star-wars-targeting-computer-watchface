#include <pebble.h>

static Window *s_main_window;
static Layer *s_window_layer, *s_foreground_layer;
static char s_time_text[6] = "00:00", s_battery_text[5] = "100%", s_date_text[12], s_steps_text[7] = "034617";
static GFont s_stencil_font_large, s_stencil_font_small, s_stencil_font_tiny;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void foreground_update_proc(Layer *s_foreground_layer, GContext *ctx) {
	GRect bounds = layer_get_bounds(s_window_layer);
	
	graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack));
	
	GSize time_text_bounds = graphics_text_layout_get_content_size(s_time_text, s_stencil_font_large, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_time_text, s_stencil_font_large, GRect(92 - 0.5 * time_text_bounds.w, 10, time_text_bounds.w, time_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

	GSize date_text_bounds = graphics_text_layout_get_content_size(s_date_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_date_text, s_stencil_font_tiny, GRect(92 - 0.5 * date_text_bounds.w, 41, date_text_bounds.w, date_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	GSize battery_text_bounds = graphics_text_layout_get_content_size(s_battery_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentRight);
	graphics_draw_text(ctx, s_battery_text, s_stencil_font_tiny, GRect(113 - battery_text_bounds.w, 2, battery_text_bounds.w, battery_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);
	
	graphics_context_set_text_color(ctx, PBL_IF_COLOR_ELSE(GColorOrange, GColorWhite));
	GSize step_text_bounds = graphics_text_layout_get_content_size(s_steps_text, s_stencil_font_small, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_steps_text, s_stencil_font_small, GRect(72 - 0.5 * step_text_bounds.w, 147, step_text_bounds.w, step_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void update_ui() {
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  strftime(s_time_text, sizeof(s_time_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	strftime(s_date_text, sizeof(s_date_text), "%F", tick_time);
	layer_mark_dirty(s_foreground_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_ui();
}

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

#if defined(PBL_HEALTH)
static void health_handler(HealthEventType event, void *context) {
	HealthValue value = health_service_sum_today(HealthMetricStepCount);
	snprintf(s_steps_text, sizeof(s_steps_text), "%06d", (int)value);
	layer_mark_dirty(s_foreground_layer);
}
#endif

static void initialize_ui() {
	GRect bounds = layer_get_bounds(s_window_layer);
	
	s_background_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_background_color(s_background_layer, GColorWhite);
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(s_main_window), bitmap_layer_get_layer(s_background_layer));
	
	s_foreground_layer = layer_create(bounds);
	layer_set_update_proc(s_foreground_layer, foreground_update_proc);
	layer_add_child(window_get_root_layer(s_main_window), s_foreground_layer);
}

static void main_window_load(Window *window) {
	s_window_layer = window_get_root_layer(window);
	initialize_ui();
	battery_handler();
	#if defined(PBL_HEALTH)
	health_handler(HealthMetricStepCount, NULL);
	#endif	
	update_ui();
}

static void main_window_unload(Window *window) {
	gbitmap_destroy(s_background_bitmap);
	bitmap_layer_destroy(s_background_layer);
	
	layer_destroy(s_foreground_layer);
	
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	health_service_events_unsubscribe();
}

static void init() {
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	s_stencil_font_large = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STENCIL_30));
	s_stencil_font_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STENCIL_16));
	s_stencil_font_tiny = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STENCIL_12));
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	battery_state_service_subscribe(battery_handler);
	#if defined(PBL_HEALTH)
	health_service_events_subscribe(health_handler, NULL);
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