#include <pebble.h>

#define D_MAX_NUM_FRAMES 40

static Window *s_main_window;
static Layer *s_window_layer, *s_foreground_layer;
static char s_time_text[6] = "00:00", s_battery_text[5] = "100%", s_date_text[12], s_steps_text[7] = "034617";
static GFont s_stencil_font_large, s_stencil_font_small, s_stencil_font_tiny;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static bool is_animating = false;
static int frames_elapsed = 0;

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
	#else
	GSize time_text_bounds = graphics_text_layout_get_content_size(s_time_text, s_stencil_font_large, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_time_text, s_stencil_font_large, GRect(90 - 0.5 * time_text_bounds.w, 17, time_text_bounds.w, time_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

	GSize date_text_bounds = graphics_text_layout_get_content_size(s_date_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter);
	graphics_draw_text(ctx, s_date_text, s_stencil_font_tiny, GRect(90 - 0.5 * date_text_bounds.w, 47, date_text_bounds.w, date_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
	
	GSize battery_text_bounds = graphics_text_layout_get_content_size(s_battery_text, s_stencil_font_tiny, GRect(0, 0, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentRight);
	graphics_draw_text(ctx, s_battery_text, s_stencil_font_tiny, GRect(90 - battery_text_bounds.w, 8, battery_text_bounds.w, battery_text_bounds.h), GTextOverflowModeWordWrap, GTextAlignmentRight, NULL);	
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

static void update_time() {
	time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  strftime(s_time_text, sizeof(s_time_text), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	strftime(s_date_text, sizeof(s_date_text), "%F", tick_time);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
	layer_mark_dirty(s_foreground_layer);
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

static void timer_handler(void * context) {
	uint32_t frame_delay = frames_elapsed > 22 ? 75 : 100;
	
	gbitmap_destroy(s_background_bitmap);
	
	switch(frames_elapsed) {
		case 1:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_2);
			break;
		case 2:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_3);
			break;
		case 3:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_4);
			break;
		case 4:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_5);
			break;
		case 5:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_6);
			break;
		case 6:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_7);
			break;
		case 7:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_8);
			break;
		case 8:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_9);
			break;
		case 9:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_10);
			break;
		case 10:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_11);
			break;
		case 11:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_12);
			break;
		case 12:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_13);
			break;
		case 13:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_14);
			break;
		case 14:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_15);
			break;
		case 15:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_16);
			break;
		case 16:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_17);
			break;
		case 17:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_18);
			break;
		case 18:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_19);
			break;
		case 19:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_20);
			break;
		case 20:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_21);
			break;
		case D_MAX_NUM_FRAMES:
			s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
			is_animating = false;
			break;
		default:
			if (frames_elapsed % 2 == 0) {
				s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_23);
			} else {
				s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_22);
			}
			break;
	}
	
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	
	frames_elapsed++;
	if (frames_elapsed <= D_MAX_NUM_FRAMES) {
		app_timer_register(frame_delay, timer_handler, NULL);
	}
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
	if (!is_animating) {
		gbitmap_destroy(s_background_bitmap);
		is_animating = true;
		frames_elapsed = 1;

		s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_FRAME_1);
		bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);

		app_timer_register(100, timer_handler, NULL);
	}
}

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

static void initialize_ui() {
	GRect bounds = layer_get_bounds(s_window_layer);
	
	s_background_layer = bitmap_layer_create(bounds);
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
	update_step_count();
	#endif
	update_time();
	layer_mark_dirty(s_foreground_layer);
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
	accel_tap_service_subscribe(accel_tap_handler);
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