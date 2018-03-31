/*

Copyright (C) 2017 Mark Reed

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-------------------------------------------------------------------

*/

#include <pebble.h>

#define TOTAL_TIME_DIGITS 4

static const uint32_t WEATHER_ICONS[] = {
  RESOURCE_ID_CLEAR_DAY,
  RESOURCE_ID_CLEAR_NIGHT,
  RESOURCE_ID_WINDY,
  RESOURCE_ID_COLD,
  RESOURCE_ID_PARTLY_CLOUDY_DAY,
  RESOURCE_ID_PARTLY_CLOUDY_NIGHT,
  RESOURCE_ID_HAZE,
  RESOURCE_ID_CLOUD,
  RESOURCE_ID_RAIN,
  RESOURCE_ID_SNOW,
  RESOURCE_ID_HAIL,
  RESOURCE_ID_CLOUDY,
  RESOURCE_ID_STORM,
  RESOURCE_ID_FOG,
  RESOURCE_ID_NA,
};

// Setting values
static bool fill;
static bool weather_status;
static bool bluetoothvibe_status;
static bool hourlyvibe_status;
static enum formatKeys1 { FORMAT_WEEK2 = 0, FORMAT_STEP2, FORMAT_HR2, FORMAT_MMDDYY2, FORMAT_END2 = FORMAT_MMDDYY2 } row1right;
static enum formatKeys { FORMAT_WEEK = 0, FORMAT_MMDDYY, FORMAT_HR, FORMAT_STEPS, FORMAT_END = FORMAT_STEPS } row2left;
static enum languageKeys { LANG_EN = 0, LANG_NL, LANG_DE, LANG_FR, LANG_HR, LANG_ES, LANG_IT, LANG_NO, LANG_SW, LANG_FI, LANG_DA, LANG_TU, LANG_CA, LANG_SL, LANG_PO, LANG_HU, LANG_END = LANG_HU } current_language;
static enum digitcolorKeys { C_WHITE = 0, C_RED, C_GREEN, C_BLUE, C_YELLOW, C_END = C_YELLOW } color_status;

// Setting keys
enum settingKeys {
  SETTING_STATUS_KEY,
  SETTING_LANGUAGE_KEY,
  SETTING_FORMAT_KEY,
  SETTING_TEMPERATURE_KEY,
  SETTING_ICON_KEY,
  SETTING_WEATHERSTATUS_KEY,
  HOURLYVIBE_KEY,
  BLUETOOTHVIBE_KEY,
  SECS_KEY,
  SETTING_COLORSTATUS_KEY
};

static Window *s_main_window;
static Layer *window_layer;

static GBitmap *s_time_digits[TOTAL_TIME_DIGITS];
static BitmapLayer *s_time_digits_layers[TOTAL_TIME_DIGITS];

static const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

const int BIG_DIGIT2_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM2_0,
  RESOURCE_ID_IMAGE_NUM2_1,
  RESOURCE_ID_IMAGE_NUM2_2,
  RESOURCE_ID_IMAGE_NUM2_3,
  RESOURCE_ID_IMAGE_NUM2_4,
  RESOURCE_ID_IMAGE_NUM2_5,
  RESOURCE_ID_IMAGE_NUM2_6,
  RESOURCE_ID_IMAGE_NUM2_7,
  RESOURCE_ID_IMAGE_NUM2_8,
  RESOURCE_ID_IMAGE_NUM2_9
};


static GBitmap *dateblank_image;
static GBitmap *heart_image;
static GBitmap *heart_image2;
static BitmapLayer *dateblank_layer;
static BitmapLayer *dateblank_layer2;
static BitmapLayer *heart_layer;
static BitmapLayer *heart_layer2;

int charge_percent = 0;

GBitmap *img_battery_100;
GBitmap *img_battery_90;
GBitmap *img_battery_80;
GBitmap *img_battery_70;
GBitmap *img_battery_60;
GBitmap *img_battery_50;
GBitmap *img_battery_40;
GBitmap *img_battery_30;
GBitmap *img_battery_20;
GBitmap *img_battery_10;
GBitmap *img_battery_charge;
BitmapLayer *layer_batt_img;

TextLayer *battery_text_layer;
TextLayer *text_daynum_layer;

static GFont futura;
static GFont futura2;
static GFont futura3;
static GFont futura4;

BitmapLayer *layer_conn_img;
GBitmap *img_bt_connect;
GBitmap *img_bt_disconnect;

int cur_day = -1;

static TextLayer *steps_label, *steps_label2, *hr_label, *hr_label2;

static GBitmap        *icon_bitmap = NULL;
BitmapLayer *layer_quiettime;                          
GBitmap *bitmap_quiettime;    

static BitmapLayer    *icon_layer;
static TextLayer      *temp_layer;

static AppSync        app;
static uint8_t        sync_buffer[256];

//TextLayer *s_hrm_layer;
static int s_hr;
static char s_current_hr_buffer[8];

static Layer  *s_progress_layer;
static Layer  *s_calprogress_layer;
static Layer  *s_distprogress_layer;
static Layer  *s_active_layer;
static Layer  *s_progress_layer2;
static Layer  *s_calprogress_layer2;
static Layer  *s_distprogress_layer2;
static Layer  *s_active_layer2;
static Layer  *s_step_arc;
static Layer  *s_step_arc_avg;
static Layer  *s_step_arc2;
static Layer  *s_step_arc_avg2;

static int s_step_count = 0, s_step_goal = 0, s_cal_count = 0, s_cal_goal = 0, s_dist_count = 0, s_dist_goal = 0, s_active_count = 0, s_active_goal = 0, s_step_average = 0;;


// Current heart rate
static void get_hr() {
		  s_hr = (int)health_service_peek_current_value(HealthMetricHeartRateBPM);
}

static void display_heart_rate() {
		
  if(s_hr>0) {
    snprintf(s_current_hr_buffer,sizeof(s_current_hr_buffer),"%d",s_hr);
    text_layer_set_text(hr_label,s_current_hr_buffer);
    text_layer_set_text(hr_label2,s_current_hr_buffer);
//    layer_set_hidden(bitmap_layer_get_layer(heart_layer),false);
//    layer_set_hidden(bitmap_layer_get_layer(heart_layer2),false);
//    layer_set_hidden(text_layer_get_layer(hr_label),false);
//    layer_set_hidden(text_layer_get_layer(hr_label2),false);
//  } else {
//    layer_set_hidden(bitmap_layer_get_layer(heart_layer),true);
//    layer_set_hidden(bitmap_layer_get_layer(heart_layer2),true);
//    layer_set_hidden(text_layer_get_layer(hr_label),true);
//    layer_set_hidden(text_layer_get_layer(hr_label2),true);
  }
}

// Average daily step count for this time of day
static void get_step_average() {
  const time_t start = time_start_of_today();
  const time_t end = time(NULL);
  s_step_average = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeWeekly);
  if(s_step_average>s_step_goal)
    s_step_average=s_step_goal;
//  APP_LOG(APP_LOG_LEVEL_DEBUG,"Step average: %d",s_step_average);
}

static void health_handler(HealthEventType event, void *context) {

//  if(event == HealthEventHeartRateUpdate ) {
  if(event != HealthEventSleepUpdate) {
    get_hr();
    display_heart_rate(); 
	     get_step_average();
  }	
 
  static char s_value_buffer[8];
	
const HealthMetric metric = HealthMetricWalkedDistanceMeters;
	
  if (event == HealthEventMovementUpdate) {
    // display the step count
    snprintf(s_value_buffer, sizeof(s_value_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));
	  
    text_layer_set_text(steps_label, s_value_buffer);
    text_layer_set_text(steps_label2, s_value_buffer);
	  
  const time_t start = time_start_of_today();
  const time_t end = start + SECONDS_PER_DAY;
//  const time_t end = time(NULL);
	  
  s_dist_count = (int)health_service_sum_today(metric);
  s_dist_goal = (int)health_service_sum_averaged(HealthMetricWalkedDistanceMeters , start, end, HealthServiceTimeScopeDaily);  
	  
  s_cal_count = (int)health_service_sum_today(HealthMetricActiveKCalories );
  s_cal_goal = (int)health_service_sum_averaged(HealthMetricActiveKCalories , start, end, HealthServiceTimeScopeDaily);  
	  
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
  s_step_goal = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);  
  //APP_LOG(APP_LOG_LEVEL_DEBUG,"Step goal: %d",s_step_goal);
	  

  s_active_count = (int)health_service_sum_today(HealthMetricActiveSeconds);
  s_active_goal = (int)health_service_sum_averaged(HealthMetricActiveSeconds, start, end, HealthServiceTimeScopeDaily);  

  }
}

static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, GColorRed);
#else
  graphics_context_set_fill_color(ctx, GColorWhite);
#endif
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 3, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE((360 * s_step_count) / s_step_goal));
}

static void cal_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, GColorChromeYellow );
#else
  graphics_context_set_fill_color(ctx, GColorWhite);
#endif
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 3, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE((360 * s_cal_count) / s_cal_goal));
}

static void active_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, GColorYellow);
#else
  graphics_context_set_fill_color(ctx, GColorWhite);
#endif
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 3, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE((360 * s_active_count) / s_active_goal));
}

static void dist_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, GColorCyan  );
#else
  graphics_context_set_fill_color(ctx, GColorWhite);
#endif
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 3, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE((360 * s_dist_count) / s_dist_goal));
}


// only step count arcs
static void steparc_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, GColorElectricUltramarine   );
#else
  graphics_context_set_fill_color(ctx, GColorWhite);
#endif
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 6, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE((360 * s_step_count) / s_step_goal));
}

static void steparcavg_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, GColorMagenta   );
#else
  graphics_context_set_fill_color(ctx, GColorWhite);
#endif
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 4, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE((360 * s_step_average) / s_step_goal));
}


static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
	
  GRect bounds = gbitmap_get_bounds(*bmp_image);

  GRect main_frame = GRect(origin.x, origin.y, bounds.size.w, bounds.size.h);
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), main_frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}

void update_battery(BatteryChargeState charge_state) {
	
    static char battery_text[] = "xxx";

    if (charge_state.is_charging) {
		bitmap_layer_set_bitmap(layer_batt_img, img_battery_charge);
		snprintf(battery_text, sizeof(battery_text), "%d", charge_state.charge_percent);
 } else {
        snprintf(battery_text, sizeof(battery_text), "%d", charge_state.charge_percent);
		
       if (charge_state.charge_percent <= 10) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_10);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorRed );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
        } else if (charge_state.charge_percent <= 20) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_20);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorRed );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
		} else if (charge_state.charge_percent <= 30) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_30);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorOrange );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
		} else if (charge_state.charge_percent <= 40) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_40);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorVividCerulean );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
		} else if (charge_state.charge_percent <= 50) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_50);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorVividCerulean );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
		} else if (charge_state.charge_percent <= 60) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_60);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorVividCerulean );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
		} else if (charge_state.charge_percent <= 70) {
           bitmap_layer_set_bitmap(layer_batt_img, img_battery_70);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorVividCerulean );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
	   } else if (charge_state.charge_percent <= 80) {
           bitmap_layer_set_bitmap(layer_batt_img, img_battery_80);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorVividCerulean );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
	   } else if (charge_state.charge_percent <= 90) {
           bitmap_layer_set_bitmap(layer_batt_img, img_battery_90);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorVividCerulean );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif
		} else if (charge_state.charge_percent <= 100) {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
#ifdef PBL_COLOR
	text_layer_set_text_color(battery_text_layer, GColorVividCerulean );
#else
	text_layer_set_text_color(battery_text_layer, GColorWhite);
#endif

		} else {
            bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
        } 		
    } 
    charge_percent = charge_state.charge_percent;
    text_layer_set_text(battery_text_layer, battery_text);
}

void toggle_bluetooth_icon(bool connected) {
	
    if (connected) {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
    } else {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_disconnect);
	}
}

void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

static unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;
  return display_hour ? display_hour : 12;
}

static void update_display(struct tm *tick_time) {

		
// Display hour
	unsigned short display_hour = get_display_hour(tick_time->tm_hour);	
/*
if (fill)	{
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(21, 28));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(53, 28));
#else
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(4, 25));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(37, 25));
#endif
	
} else {
	
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(21, 28));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(53, 28));
#else
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(4, 25));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(37, 25));
#endif
	
}
	
if (fill) {	
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(90, 28));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(122, 28));		
#else
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(73, 25));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(106, 25));
#endif

} else {
	
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(90, 28));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(122, 28));		
#else
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(73, 25));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(106, 25));
#endif
	
}
	*/
 if (!clock_is_24h_style()) {
    if (display_hour / 10 == 0) {
    	layer_set_hidden(bitmap_layer_get_layer(s_time_digits_layers[0]), true);
    } else {
    	layer_set_hidden(bitmap_layer_get_layer(s_time_digits_layers[0]), false);
    }
  }
	
  if ( !clock_is_24h_style()) {
	  
//	  if (display_hour / 10 == 0) {
		  
		  if (fill) {
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(21, 28));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(53, 28));
#else 
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(-3, 25));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(27, 25));
#endif

#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(90, 28));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(122, 28));		
#else
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(66, 25));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(99, 25));
#endif		  
		  } else {
			  
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(21, 28));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(53, 28));
#else 
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(-3, 25));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(27, 25));
#endif

#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(90, 28));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(122, 28));		
#else
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(66, 25));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(99, 25));
#endif				  
					  
		  }	 
		  
	
} else {
		  
			  if (fill) {
				  
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(21, 28));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(53, 28));
#else
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(4, 25));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(37, 25));
#endif	
	
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(90, 28));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(122, 28));		
#else
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(73, 25));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(106, 25));
#endif

} else {
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(21, 28));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(53, 28));
#else
  set_container_image(&s_time_digits[0], s_time_digits_layers[0], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour / 10], GPoint(4, 25));
  set_container_image(&s_time_digits[1], s_time_digits_layers[1], BIG_DIGIT2_IMAGE_RESOURCE_IDS[display_hour % 10], GPoint(37, 25));
#endif	
	
#if PBL_PLATFORM_CHALK
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(90, 28));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(122, 28));		
#else
  set_container_image(&s_time_digits[2], s_time_digits_layers[2], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min / 10], GPoint(73, 25));
  set_container_image(&s_time_digits[3], s_time_digits_layers[3], BIG_DIGIT2_IMAGE_RESOURCE_IDS[tick_time->tm_min % 10], GPoint(106, 25));
#endif				  
			  }	
		 }
  }


static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {

  update_display(tick_time);
	
  layer_set_hidden(bitmap_layer_get_layer(layer_quiettime), !quiet_time_is_active());

	  if (units_changed & HOUR_UNIT) {
		    if (hourlyvibe_status) {
    			//vibe!
    			vibes_short_pulse();
  			} 
		}
	
	static char date_text1b[] = "xxx 00xx";
	
// Display daynum 
	    int new_cur_day = tick_time->tm_year*1000 + tick_time->tm_yday;
    if (new_cur_day != cur_day) {
        cur_day = new_cur_day;

	switch(tick_time->tm_mday)
  {
    case 1 :
    case 21 :
    case 31 :
      strftime(date_text1b, sizeof(date_text1b), "%a %est", tick_time);
      break;
    case 2 :
    case 22 :
      strftime(date_text1b, sizeof(date_text1b), "%a %end", tick_time);
      break;
    case 3 :
    case 23 :
      strftime(date_text1b, sizeof(date_text1b), "%a %erd", tick_time);
      break;
    default :
      strftime(date_text1b, sizeof(date_text1b), "%a %eth", tick_time);
      break;
  }
	  text_layer_set_text(text_daynum_layer, date_text1b);
	}		
}

/*
  Handle update in settings
*/

// validate upper limit (can not be higher, than 1)
// mind: value is unsigned, so it can not be less than 0
#define VALIDATE_BOOL(value) if (value > 1) return;

#define VALIDATE_MAXIMUM(name, value, max) \
if (value > (uint8_t)max) \
{ \
    APP_LOG( APP_LOG_LEVEL_ERROR, "%s boundary error: %u is not less or equal than %u", name, value, max ); \
    return; \
}

static void tuple_changed_callback( const uint32_t key, const Tuple* tuple_new, const Tuple* tuple_old, void* context )
{
  uint8_t value = tuple_new->value->uint8;

  // APP_LOG( APP_LOG_LEVEL_DEBUG, "tuple_changed_callback: %lu", key );

  switch ( key )
  {
    case SETTING_STATUS_KEY:
      VALIDATE_BOOL(value)

      persist_write_int( SETTING_STATUS_KEY, value );
      fill = value;

      break;

    case SETTING_LANGUAGE_KEY:
      VALIDATE_MAXIMUM("SETTING_LANGUAGE_KEY", value, LANG_END)

      persist_write_int( SETTING_LANGUAGE_KEY, value );
      current_language = value;
      break;


    case SETTING_ICON_KEY:
      VALIDATE_MAXIMUM("SETTING_ICON_KEY", value, ARRAY_LENGTH(WEATHER_ICONS))

      if (icon_bitmap) {
        gbitmap_destroy(icon_bitmap);
      }
      icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[tuple_new->value->uint8]);
      bitmap_layer_set_bitmap(icon_layer, icon_bitmap);

      break;

    case SETTING_TEMPERATURE_KEY:
      text_layer_set_text(temp_layer, tuple_new->value->cstring);
      break;

    case BLUETOOTHVIBE_KEY:
      VALIDATE_BOOL(value)

      persist_write_int( BLUETOOTHVIBE_KEY, value );
      bluetoothvibe_status = value;
      break;

    case HOURLYVIBE_KEY:
      VALIDATE_BOOL(value)

      persist_write_int( HOURLYVIBE_KEY, value );
      hourlyvibe_status = value;
      break;

    case SECS_KEY:
      VALIDATE_MAXIMUM("SETTING_FORMAT_KEY", value, FORMAT_END2)

      persist_write_int( SECS_KEY, value );
      row1right = value;

	  switch (row1right) {
		  
		  // widget 1
		  
	case 0: //health arcs
	
  layer_set_hidden((s_progress_layer), false);
  layer_set_hidden((s_calprogress_layer), false);
  layer_set_hidden((s_active_layer), false);
  layer_set_hidden((s_distprogress_layer), false);
	
  layer_set_hidden((s_step_arc2), true);
  layer_set_hidden((s_step_arc_avg2), true);		  

#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),false);
  layer_set_hidden(text_layer_get_layer(steps_label),false);
#else
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer2),true);
  layer_set_hidden(text_layer_get_layer(steps_label2),true);		
#endif
	  
  layer_set_hidden(bitmap_layer_get_layer(heart_layer2),true);
  layer_set_hidden(text_layer_get_layer(hr_label2),true);		
  
		  break;
		 
	case 1: //step arcs
		  
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden((s_progress_layer), false);
  layer_set_hidden((s_calprogress_layer), false);
  layer_set_hidden((s_active_layer), false);
  layer_set_hidden((s_distprogress_layer), false);
#else
  layer_set_hidden((s_progress_layer), true);
  layer_set_hidden((s_calprogress_layer), true);
  layer_set_hidden((s_active_layer), true);
  layer_set_hidden((s_distprogress_layer), true);
#endif
		  
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden((s_step_arc2), true);
  layer_set_hidden((s_step_arc_avg2), true);
#else
  layer_set_hidden((s_step_arc2), false);
  layer_set_hidden((s_step_arc_avg2), false);
#endif
		  
		  
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),false);
  layer_set_hidden(text_layer_get_layer(steps_label),false);
#else
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer2),true);
  layer_set_hidden(text_layer_get_layer(steps_label2),true);		
#endif
		  
  layer_set_hidden(bitmap_layer_get_layer(heart_layer2),true);
  layer_set_hidden(text_layer_get_layer(hr_label2),true);	
		  
		  break;
		  
		case 2:  //stepcount
		
 #ifdef PBL_PLATFORM_CHALK
  layer_set_hidden((s_progress_layer), false);
  layer_set_hidden((s_calprogress_layer), false);
  layer_set_hidden((s_active_layer), false);
  layer_set_hidden((s_distprogress_layer), false);
#else
  layer_set_hidden((s_progress_layer), true);
  layer_set_hidden((s_calprogress_layer), true);
  layer_set_hidden((s_active_layer), true);
  layer_set_hidden((s_distprogress_layer), true);
#endif
		  
  layer_set_hidden((s_step_arc2), true);
  layer_set_hidden((s_step_arc_avg2), true);

#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),false);
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer2),true);
  layer_set_hidden(text_layer_get_layer(steps_label),false);
  layer_set_hidden(text_layer_get_layer(steps_label2),true);
#else		  
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer2),false);
  layer_set_hidden(text_layer_get_layer(steps_label2),false);
#endif
		  
  layer_set_hidden(bitmap_layer_get_layer(heart_layer2),true);
  layer_set_hidden(text_layer_get_layer(hr_label2),true);	
		  
		  break;
		  
		case 3: //hr
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden((s_progress_layer), false);
  layer_set_hidden((s_calprogress_layer), false);
  layer_set_hidden((s_active_layer), false);
  layer_set_hidden((s_distprogress_layer), false);
#else
  layer_set_hidden((s_progress_layer), true);
  layer_set_hidden((s_calprogress_layer), true);
  layer_set_hidden((s_active_layer), true);
  layer_set_hidden((s_distprogress_layer), true);
#endif 
		  
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),false);
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer2),true);
  layer_set_hidden(text_layer_get_layer(steps_label),false);
  layer_set_hidden(text_layer_get_layer(steps_label2),true);
#else
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),true);
  layer_set_hidden(text_layer_get_layer(steps_label),true);		
#endif
		  
		  
  layer_set_hidden((s_step_arc2), true);
  layer_set_hidden((s_step_arc_avg2), true);

  layer_set_hidden(bitmap_layer_get_layer(heart_layer2),false);
  layer_set_hidden(text_layer_get_layer(hr_label2),false);	

		break;		  
		  
	  }
		
      break;
	
	case SETTING_FORMAT_KEY:
      VALIDATE_MAXIMUM("SETTING_FORMAT_KEY", value, FORMAT_END)

      persist_write_int( SETTING_FORMAT_KEY, value );
      row2left = value;
	
  
	switch (row2left) {	
		
		// widget 2
		
	case 0: //health arcs
	 
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden((s_progress_layer2), true);
  layer_set_hidden((s_calprogress_layer2), true);
  layer_set_hidden((s_active_layer2), true);
  layer_set_hidden((s_distprogress_layer2), true);
#else
  layer_set_hidden((s_progress_layer2), false);
  layer_set_hidden((s_calprogress_layer2), false);
  layer_set_hidden((s_active_layer2), false);
  layer_set_hidden((s_distprogress_layer2), false);
#endif
		
		
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden((s_step_arc), false);
  layer_set_hidden((s_step_arc_avg), false);
#else
  layer_set_hidden((s_step_arc), true);
  layer_set_hidden((s_step_arc_avg), true);
#endif	  
	  

#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),false);
  layer_set_hidden(text_layer_get_layer(steps_label),false);
#else
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),true);
  layer_set_hidden(text_layer_get_layer(steps_label),true);		
#endif
	
  layer_set_hidden(bitmap_layer_get_layer(heart_layer),true);
  layer_set_hidden(text_layer_get_layer(hr_label),true);	
		
		break;
		
		
		case 1:  //steparcs
  layer_set_hidden((s_progress_layer2), true);
  layer_set_hidden((s_calprogress_layer2), true);
  layer_set_hidden((s_active_layer2), true);
  layer_set_hidden((s_distprogress_layer2), true);
		
  layer_set_hidden(bitmap_layer_get_layer(heart_layer),true);
  layer_set_hidden(text_layer_get_layer(hr_label),true);	

#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),false);
  layer_set_hidden(text_layer_get_layer(steps_label),false);
#else
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),true);
  layer_set_hidden(text_layer_get_layer(steps_label),true);		
#endif
		
  layer_set_hidden((s_step_arc), false);
  layer_set_hidden((s_step_arc_avg), false);
		
		break;	
		
		case 2:  //stepcount
  layer_set_hidden((s_progress_layer2), true);
  layer_set_hidden((s_calprogress_layer2), true);
  layer_set_hidden((s_active_layer2), true);
  layer_set_hidden((s_distprogress_layer2), true);
		
  layer_set_hidden(bitmap_layer_get_layer(heart_layer),true);
  layer_set_hidden(text_layer_get_layer(hr_label),true);	
		
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden((s_step_arc), false);
  layer_set_hidden((s_step_arc_avg), false);
#else
  layer_set_hidden((s_step_arc), true);
  layer_set_hidden((s_step_arc_avg), true);
#endif
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),false);
  layer_set_hidden(text_layer_get_layer(steps_label),false);

		break;
		
		
		case 3: //hr
  layer_set_hidden((s_progress_layer2), true);
  layer_set_hidden((s_calprogress_layer2), true);
  layer_set_hidden((s_active_layer2), true);
  layer_set_hidden((s_distprogress_layer2), true);
	

#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden((s_step_arc), false);
  layer_set_hidden((s_step_arc_avg), false);
#else
  layer_set_hidden((s_step_arc), true);
  layer_set_hidden((s_step_arc_avg), true);
#endif
		

  layer_set_hidden(bitmap_layer_get_layer(heart_layer),false);
  layer_set_hidden(text_layer_get_layer(hr_label),false);	
		
#ifdef PBL_PLATFORM_CHALK
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),false);
  layer_set_hidden(text_layer_get_layer(steps_label),false);
#else
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),true);
  layer_set_hidden(text_layer_get_layer(steps_label),true);		
#endif
		
		break;
		
	}  	  
 
      break;

	  
	  case SETTING_COLORSTATUS_KEY:
           VALIDATE_MAXIMUM("SETTING_COLORSTATUS_KEY", value, C_END)

      persist_write_int( SETTING_COLORSTATUS_KEY, value );
      color_status = value;

	  break;

	  case SETTING_WEATHERSTATUS_KEY:
      VALIDATE_BOOL(value)

      persist_write_int( SETTING_WEATHERSTATUS_KEY, value );
      weather_status = value;
			
	  break;


    default:
      APP_LOG(APP_LOG_LEVEL_INFO, "unknown tuple key: %lu", key);
  }

  // Refresh display
  // Get current time
  time_t now = time( NULL );
  struct tm *tick_time = localtime( &now );

  // Force update to avoid a blank screen at startup of the watchface
  handle_tick(tick_time, 0);
}

/*
  Handle errors
*/
static void app_error_callback( DictionaryResult dict_error, AppMessageResult app_message_error, void* context ) {
  APP_LOG( APP_LOG_LEVEL_ERROR, "app error: %d", app_message_error );
//  vibes_double_pulse();
}


static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(s_main_window);
	
  window_set_background_color(window, GColorBlack);

	futura = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURAITALIC_16));
	futura2 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURAITALIC_15));
	futura3 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURAITALIC_18));
	futura4 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FUTURAITALIC_20));
	
//------- battery stuff ----//
	
  img_battery_100    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_100);
  img_battery_90    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_90);
  img_battery_80    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_80);
  img_battery_70    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_70);
  img_battery_60    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_60);
  img_battery_50    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_50);
  img_battery_40    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_40);
  img_battery_30    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_30);
  img_battery_20    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_20);
  img_battery_10    = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_10);
  img_battery_charge = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATT_CHARGING);
		
	
//------- time -------//
	
// Create time and date layers
  GRect dummy_frame = GRect(0, 0, 0, 0);
	
 for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    s_time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_time_digits_layers[i]));
  }

  bitmap_quiettime = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_QT);
#if PBL_PLATFORM_CHALK	
  layer_quiettime = bitmap_layer_create(GRect(71,159, 11,14));
#else
  layer_quiettime = bitmap_layer_create(GRect(10,3, 11,14));
#endif
  bitmap_layer_set_bitmap(layer_quiettime,bitmap_quiettime);
  layer_add_child(window_layer,bitmap_layer_get_layer(layer_quiettime));
	

//------- day/date --------//

  // Setup daynum and simple time layers
	
#ifdef PBL_PLATFORM_CHALK
  text_daynum_layer = text_layer_create(GRect(  0, 8,  180,  24 ));
  text_layer_set_text_alignment(text_daynum_layer, GTextAlignmentCenter);	
  text_layer_set_font(text_daynum_layer, futura);
#else
  text_daynum_layer = text_layer_create(GRect(  10, -1,  126,  24 ));
  text_layer_set_text_alignment(text_daynum_layer, GTextAlignmentRight);	
  text_layer_set_font(text_daynum_layer, futura3);
#endif			
  text_layer_set_background_color(text_daynum_layer, GColorClear);
  text_layer_set_text_color(text_daynum_layer, GColorWhite );
  layer_add_child( window_layer, text_layer_get_layer( text_daynum_layer ) );

	
//------------- heart rate 1 ------------//
	
// heart
  heart_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EMPTY);
#if PBL_PLATFORM_CHALK
  GRect frame2 = GRect(0, 0, 0, 0);
#elif PBL_PLATFORM_DIORITE
  GRect frame2 = GRect(80, 121, 50, 50);
#else
  GRect frame2 = GRect(0, 0, 0, 0);
#endif
  heart_layer = bitmap_layer_create(frame2);
  bitmap_layer_set_bitmap(heart_layer, heart_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(heart_layer));	
  layer_set_hidden(bitmap_layer_get_layer(heart_layer),true);
	
// Create Heart Rate Layer
#if PBL_PLATFORM_CHALK
  hr_label = text_layer_create(GRect(0, 0, 0, 0));
#elif PBL_PLATFORM_DIORITE
  hr_label = text_layer_create(GRect(74, 129, 60, 20));
#else
  hr_label = text_layer_create(GRect(0, 0, 0, 0));
#endif
  text_layer_set_background_color(hr_label, GColorClear);
  text_layer_set_text_color(hr_label, GColorWhite);
  text_layer_set_text_alignment(hr_label, GTextAlignmentCenter);
  text_layer_set_font(hr_label, futura);
//  text_layer_set_text(hr_label, "--");
  layer_add_child(window_layer, text_layer_get_layer(hr_label));
  layer_set_hidden(text_layer_get_layer(hr_label),true);	
	

//----- weather -----//

#ifdef PBL_PLATFORM_CHALK
	icon_layer = bitmap_layer_create(GRect(106, 130, 25, 25));	
#elif PBL_PLATFORM_APLITE	
	icon_layer = bitmap_layer_create(GRect(78, 86, 56, 56));		
#else
	icon_layer = bitmap_layer_create(GRect( 26, 72,  25,  25 ));
#endif	
    layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));
    
#if PBL_PLATFORM_CHALK
	temp_layer = text_layer_create(GRect(86, 111, 70, 20));	
    text_layer_set_text_alignment(temp_layer, GTextAlignmentCenter);
    text_layer_set_font(temp_layer, futura);
#elif PBL_PLATFORM_APLITE	
	temp_layer = text_layer_create(GRect(0, 144, 144, 20));	
    text_layer_set_text_alignment(temp_layer, GTextAlignmentCenter);
    text_layer_set_font(temp_layer, futura4);
#else 
	temp_layer = text_layer_create(GRect( 6, 96,  60,  20 ));	
    text_layer_set_text_alignment(temp_layer, GTextAlignmentCenter);
    text_layer_set_font(temp_layer, futura);
#endif
    text_layer_set_text_color(temp_layer, GColorWhite);
    text_layer_set_background_color(temp_layer, GColorClear);
//    text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(temp_layer));	


//------------- heart rate 2 ------------//


// heart
  heart_image2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EMPTY);
#if PBL_PLATFORM_CHALK
  GRect frame2b = GRect(0, 0, 0, 0);
#elif PBL_PLATFORM_DIORITE
  GRect frame2b = GRect(80, 72, 50, 50);
#else
  GRect frame2b = GRect(0, 0, 0, 0);
#endif
  heart_layer2 = bitmap_layer_create(frame2b);
  bitmap_layer_set_bitmap(heart_layer2, heart_image2);
  layer_add_child(window_layer, bitmap_layer_get_layer(heart_layer2));	
  layer_set_hidden(bitmap_layer_get_layer(heart_layer2),true);
	
// Create Heart Rate Layer
#if PBL_PLATFORM_CHALK
  hr_label2 = text_layer_create(GRect(0, 0, 0, 0));
#elif PBL_PLATFORM_DIORITE
  hr_label2 = text_layer_create(GRect(70,80, 70, 20));
#else
  hr_label2 = text_layer_create(GRect(0, 0, 0, 0));
#endif
  text_layer_set_background_color(hr_label2, GColorClear);
  text_layer_set_text_color(hr_label2, GColorWhite);
  text_layer_set_text_alignment(hr_label2, GTextAlignmentCenter);
  text_layer_set_font(hr_label2, futura);
//  text_layer_set_text(hr_label2, "--");
  layer_add_child(window_layer, text_layer_get_layer(hr_label2));
  layer_set_hidden(text_layer_get_layer(hr_label2),true);
	


	//------------- battery ------------//
	
#if PBL_PLATFORM_CHALK	
  layer_batt_img  = bitmap_layer_create(GRect(71, 76, 37, 37));
#elif PBL_PLATFORM_APLITE	
  layer_batt_img  = bitmap_layer_create(GRect(10, 86, 48, 48));
#else
  layer_batt_img  = bitmap_layer_create(GRect(12, 118, 48, 48));
#endif
  bitmap_layer_set_bitmap(layer_batt_img, img_battery_100);
  layer_add_child(window_layer, bitmap_layer_get_layer(layer_batt_img));
	
#ifdef PBL_PLATFORM_CHALK
	battery_text_layer = text_layer_create(GRect(61, 84, 55, 24));	
    text_layer_set_font(battery_text_layer, futura);
#elif PBL_PLATFORM_APLITE
	battery_text_layer = text_layer_create(GRect( 1, 98,  67,  20 ));
    text_layer_set_font(battery_text_layer, futura2);
#else
	battery_text_layer = text_layer_create(GRect( 3, 130,  67,  20 ));
    text_layer_set_font(battery_text_layer, futura2);
#endif
	text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
	text_layer_set_text_color(battery_text_layer, GColorWhite);
    text_layer_set_font(battery_text_layer, futura);
	text_layer_set_background_color(battery_text_layer, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));


//------------- health ------------//

	
//#ifdef PBL_HEALTH
// walking man
  dateblank_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EMPTY2);
#if PBL_PLATFORM_CHALK
  GRect frame = GRect(53, 132, 14, 21);
#else
  GRect frame = GRect(98, 126, 14, 21);
#endif
  dateblank_layer = bitmap_layer_create(frame);
  bitmap_layer_set_bitmap(dateblank_layer, dateblank_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(dateblank_layer));		
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer),true);
//#endif
	
// walking man 2
  dateblank_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EMPTY2);
#if PBL_PLATFORM_CHALK
  GRect frameW2 = GRect(0, 0, 0, 0);
#else
  GRect frameW2 = GRect(98, 76, 14, 21);
#endif
  dateblank_layer2 = bitmap_layer_create(frameW2);
  bitmap_layer_set_bitmap(dateblank_layer2, dateblank_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(dateblank_layer2));		
  layer_set_hidden(bitmap_layer_get_layer(dateblank_layer2),true);

//steps number	
#if PBL_PLATFORM_CHALK	
  steps_label = text_layer_create(GRect(22, 111, 80, 22));
  text_layer_set_text_alignment(steps_label, GTextAlignmentCenter);
#else
  steps_label = text_layer_create(GRect(70, 145, 70, 20));
  text_layer_set_text_alignment(steps_label, GTextAlignmentCenter);
#endif
  text_layer_set_background_color(steps_label, GColorClear);
  text_layer_set_text_color(steps_label, GColorWhite   );
#ifdef PBL_PLATFORM_CHALK
  text_layer_set_font(steps_label, futura);
#else
  text_layer_set_font(steps_label, futura);	
#endif
//  text_layer_set_text(steps_label, "00000");
  layer_add_child(window_layer, text_layer_get_layer(steps_label));
  layer_set_hidden(text_layer_get_layer(steps_label),true);

//steps number2
#if PBL_PLATFORM_CHALK	
  steps_label2 = text_layer_create(GRect(0, 77, 180, 22));
  text_layer_set_text_alignment(steps_label2, GTextAlignmentCenter);
#else
  steps_label2 = text_layer_create(GRect(70, 96, 70, 20));
  text_layer_set_text_alignment(steps_label2, GTextAlignmentCenter);
#endif
  text_layer_set_background_color(steps_label2, GColorClear);
  text_layer_set_text_color(steps_label2, GColorWhite   );
#ifdef PBL_PLATFORM_CHALK
  text_layer_set_font(steps_label2, futura);
#else
  text_layer_set_font(steps_label2, futura);	
#endif
//  text_layer_set_text(steps_label, "00000");
  layer_add_child(window_layer, text_layer_get_layer(steps_label2));
  layer_set_hidden(text_layer_get_layer(steps_label2),true);
	
	
	
// Steps Progress indicator - outer ring
#ifdef PBL_PLATFORM_CHALK	
  s_progress_layer = layer_create(GRect(129,74,41,41));
#else
  s_progress_layer = layer_create(GRect(80,72,48,48 ));
#endif 
  layer_set_update_proc(s_progress_layer, progress_layer_update_proc);
  layer_add_child(window_layer, s_progress_layer);
	
// Cal Progress indicator - ring2
#ifdef PBL_PLATFORM_CHALK	
  s_calprogress_layer = layer_create(GRect(133,78,33,33));
#else
  s_calprogress_layer = layer_create(GRect(84,76,40,40));
#endif 
  layer_set_update_proc(s_calprogress_layer, cal_layer_update_proc);
  layer_add_child(window_layer, s_calprogress_layer);

// Active Progress indicator -  ring3
#ifdef PBL_PLATFORM_CHALK	
  s_active_layer = layer_create(GRect(137,82,25,25));
#else
  s_active_layer = layer_create(GRect(88,80,32,32));
#endif 
  layer_set_update_proc(s_active_layer, active_layer_update_proc);
  layer_add_child(window_layer, s_active_layer);
	
// Dist Progress indicator - inner ring
#ifdef PBL_PLATFORM_CHALK	
  s_distprogress_layer = layer_create(GRect(141,86,17,17));
#else
  s_distprogress_layer = layer_create(GRect(92,84,24,24));
#endif 
  layer_set_update_proc(s_distprogress_layer, dist_layer_update_proc);
  layer_add_child(window_layer, s_distprogress_layer);
	
  layer_set_hidden((s_progress_layer), true);
  layer_set_hidden((s_calprogress_layer), true);
  layer_set_hidden((s_active_layer), true);
  layer_set_hidden((s_distprogress_layer), true);
	
	
	
// Steps Progress indicator - outer ring
#ifdef PBL_PLATFORM_CHALK	
  s_progress_layer2 = layer_create(GRect(80,70,41,41));
#else
  s_progress_layer2 = layer_create(GRect(80,120,48,48 ));
#endif 
  layer_set_update_proc(s_progress_layer2, progress_layer_update_proc);
  layer_add_child(window_layer, s_progress_layer2);
	
// Cal Progress indicator - ring2
#ifdef PBL_PLATFORM_CHALK	
  s_calprogress_layer2 = layer_create(GRect(12,74,33,33));
#else
  s_calprogress_layer2 = layer_create(GRect(84,124,40,40));
#endif 
  layer_set_update_proc(s_calprogress_layer2, cal_layer_update_proc);
  layer_add_child(window_layer, s_calprogress_layer2);

// Active Progress indicator -  ring3
#ifdef PBL_PLATFORM_CHALK	
  s_active_layer2 = layer_create(GRect(16,78,25,25));
#else
  s_active_layer2 = layer_create(GRect(88,128,32,32));
#endif 
  layer_set_update_proc(s_active_layer2, active_layer_update_proc);
  layer_add_child(window_layer, s_active_layer2);
	
// Dist Progress indicator - inner ring
#ifdef PBL_PLATFORM_CHALK	
  s_distprogress_layer2 = layer_create(GRect(20,82,17,17));
#else
  s_distprogress_layer2 = layer_create(GRect(92,132,24,24));
#endif 
  layer_set_update_proc(s_distprogress_layer2, dist_layer_update_proc);
  layer_add_child(window_layer, s_distprogress_layer2);
	
  layer_set_hidden((s_progress_layer2), true);
  layer_set_hidden((s_calprogress_layer2), true);
  layer_set_hidden((s_active_layer2), true);
  layer_set_hidden((s_distprogress_layer2), true);
	
	

// Steps indicator - outer ring
#ifdef PBL_PLATFORM_CHALK	
  s_step_arc = layer_create(GRect(10,74,41,41));
#else
  s_step_arc = layer_create(GRect(80,120,48,48 ));
#endif 
  layer_set_update_proc(s_step_arc, steparc_update_proc);
  layer_add_child(window_layer, s_step_arc);
  layer_set_hidden((s_step_arc), true);
	
// Steps average indicator - ring2
#ifdef PBL_PLATFORM_CHALK	
  s_step_arc_avg = layer_create(GRect(17,81,27,27));
#else
  s_step_arc_avg = layer_create(GRect(88,128,32,32));
#endif 
  layer_set_update_proc(s_step_arc_avg, steparcavg_update_proc);
  layer_add_child(window_layer, s_step_arc_avg);
  layer_set_hidden((s_step_arc_avg), true);


// Steps indicator 2 - outer ring
#ifdef PBL_PLATFORM_CHALK	
  s_step_arc2 = layer_create(GRect(130,76,41,41));
#else
  s_step_arc2 = layer_create(GRect(80,72,48,48 ));
#endif 
  layer_set_update_proc(s_step_arc2, steparc_update_proc);
  layer_add_child(window_layer, s_step_arc2);
  layer_set_hidden((s_step_arc2), true);
	
// Steps average indicator 2 - ring2
#ifdef PBL_PLATFORM_CHALK	
  s_step_arc_avg2 = layer_create(GRect(136,82,30,30));
#else
  s_step_arc_avg2 = layer_create(GRect(88,80,32,32));
#endif 
  layer_set_update_proc(s_step_arc_avg2, steparcavg_update_proc);
  layer_add_child(window_layer, s_step_arc_avg2);
  layer_set_hidden((s_step_arc_avg2), true);
	
//------------- bluetooth ------------//

	
  img_bt_connect     = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHON);
  img_bt_disconnect  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHOFF);
	
#if PBL_PLATFORM_CHALK	
  layer_conn_img = bitmap_layer_create(GRect(94,161, 13,13));
#else
  layer_conn_img = bitmap_layer_create(GRect(28,3, 13,13));
#endif
  bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
  layer_add_child(window_layer, bitmap_layer_get_layer(layer_conn_img)); 	

	
	// update the battery on launch
   update_battery(battery_state_service_peek());
   bluetooth_connection_callback(bluetooth_connection_service_peek());

}

static void main_window_unload(Window *window) {
	
for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(s_time_digits_layers[i]));
    gbitmap_destroy(s_time_digits[i]);
    bitmap_layer_destroy(s_time_digits_layers[i]);
	s_time_digits[i] = NULL;
	s_time_digits_layers[i] = NULL;
  }
	
  text_layer_destroy( battery_text_layer );
  text_layer_destroy( text_daynum_layer );
  text_layer_destroy( steps_label );
  text_layer_destroy( steps_label2 );
  text_layer_destroy( hr_label );
  text_layer_destroy( hr_label2 );
//  text_layer_destroy( s_hrm_layer );
	
  fonts_unload_custom_font(futura);
  fonts_unload_custom_font(futura2);
  fonts_unload_custom_font(futura3);
  fonts_unload_custom_font(futura4);

  layer_remove_from_parent(bitmap_layer_get_layer(layer_quiettime));
  bitmap_layer_destroy(layer_quiettime);
  gbitmap_destroy(bitmap_quiettime);
	
  layer_remove_from_parent(bitmap_layer_get_layer(dateblank_layer));
  layer_remove_from_parent(bitmap_layer_get_layer(dateblank_layer2));
  bitmap_layer_destroy(dateblank_layer);
  bitmap_layer_destroy(dateblank_layer2);
  gbitmap_destroy(dateblank_image);
	
  layer_remove_from_parent(bitmap_layer_get_layer(heart_layer));
  layer_remove_from_parent(bitmap_layer_get_layer(heart_layer2));
  bitmap_layer_destroy(heart_layer);
  bitmap_layer_destroy(heart_layer2);
  gbitmap_destroy(heart_image);
  gbitmap_destroy(heart_image2);
	
  text_layer_destroy(temp_layer);

  layer_remove_from_parent(bitmap_layer_get_layer(icon_layer));
  bitmap_layer_destroy(icon_layer);
  gbitmap_destroy(icon_bitmap);

  layer_remove_from_parent(bitmap_layer_get_layer(layer_conn_img));
  bitmap_layer_destroy(layer_conn_img);
  gbitmap_destroy(img_bt_connect);
  gbitmap_destroy(img_bt_disconnect);
	
  layer_remove_from_parent(bitmap_layer_get_layer(layer_batt_img));
  bitmap_layer_destroy(layer_batt_img);  
  gbitmap_destroy(img_battery_10);
  gbitmap_destroy(img_battery_20);
  gbitmap_destroy(img_battery_30);
  gbitmap_destroy(img_battery_40);
  gbitmap_destroy(img_battery_50);
  gbitmap_destroy(img_battery_60);
  gbitmap_destroy(img_battery_70);
  gbitmap_destroy(img_battery_80);
  gbitmap_destroy(img_battery_90);
  gbitmap_destroy(img_battery_100);
  gbitmap_destroy(img_battery_charge);	
	
  layer_destroy(s_step_arc);
  layer_destroy(s_step_arc_avg);
  layer_destroy(s_step_arc2);
  layer_destroy(s_step_arc_avg2);
	
  layer_destroy(s_progress_layer);
  layer_destroy(s_progress_layer2);
  layer_destroy(s_active_layer);
  layer_destroy(s_active_layer2);
  layer_destroy(s_calprogress_layer);
  layer_destroy(s_calprogress_layer2);
  layer_destroy(s_distprogress_layer);
  layer_destroy(s_distprogress_layer2);
	
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
	
}

static void init() {
		
  memset(&s_time_digits_layers, 0, sizeof(s_time_digits_layers));
  memset(&s_time_digits, 0, sizeof(s_time_digits));

  // international support
  setlocale(LC_ALL, "");
	
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // Read persistent data

#define GET_PERSIST_VALUE_OR_DEFAULT(key, def) \
  persist_exists( key ) ? persist_read_int( key ) : def

  // Read watchface settings from persistent data or use default values
  fill			       = GET_PERSIST_VALUE_OR_DEFAULT( SETTING_STATUS_KEY,        true);
  weather_status       = GET_PERSIST_VALUE_OR_DEFAULT( SETTING_WEATHERSTATUS_KEY, true);
  current_language     = GET_PERSIST_VALUE_OR_DEFAULT( SETTING_LANGUAGE_KEY,      LANG_EN);
  row1right		       = GET_PERSIST_VALUE_OR_DEFAULT( SECS_KEY,			      FORMAT_WEEK2);
  row2left		       = GET_PERSIST_VALUE_OR_DEFAULT( SETTING_FORMAT_KEY,        FORMAT_WEEK);
  bluetoothvibe_status = GET_PERSIST_VALUE_OR_DEFAULT( BLUETOOTHVIBE_KEY,         true);
  hourlyvibe_status    = GET_PERSIST_VALUE_OR_DEFAULT( HOURLYVIBE_KEY ,           false);
  color_status 		   = GET_PERSIST_VALUE_OR_DEFAULT( SETTING_COLORSTATUS_KEY ,  C_WHITE);
  
  // Initial settings
  Tuplet initial_values[] = { TupletInteger( SETTING_STATUS_KEY, fill )
                            , TupletInteger( SETTING_WEATHERSTATUS_KEY, weather_status )
                            , TupletInteger( SETTING_LANGUAGE_KEY, current_language )
                            , TupletInteger( SETTING_FORMAT_KEY, row2left )
                            , TupletInteger( BLUETOOTHVIBE_KEY, bluetoothvibe_status )
                            , TupletInteger( HOURLYVIBE_KEY, hourlyvibe_status )
                            , TupletInteger( SECS_KEY, row1right )
                            , TupletInteger( SETTING_ICON_KEY, (uint8_t) 14)
                            , TupletCString( SETTING_TEMPERATURE_KEY, "   ")
							, TupletInteger( SETTING_COLORSTATUS_KEY, color_status )

                            };

  // Open AppMessage to transfers
  app_message_open( 256, 256 );

  // Initialize AppSync
  app_sync_init( &app, sync_buffer
               , sizeof( sync_buffer )
               , initial_values
               , ARRAY_LENGTH( initial_values )
               , tuple_changed_callback
               , app_error_callback
               , NULL
               );
	
  battery_state_service_subscribe(&update_battery);
  bluetooth_connection_service_subscribe(toggle_bluetooth_icon);
	
// Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);
//  update_display(tick_time);
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	
// subscribe to health events
  if(health_service_events_subscribe(health_handler, NULL)) {
    // force initial steps display
    health_handler(HealthEventMovementUpdate, NULL);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }	
}

static void deinit() {
  
  app_sync_deinit(&app);

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  health_service_events_unsubscribe();
	
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
