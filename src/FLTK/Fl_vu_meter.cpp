#include <stdlib.h>
#include <math.h>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Color_Chooser.H>

#include "Fl_vu_meter.h"
#include "cfg.h"

VUMeter::VUMeter(int X, int Y, int W, int H, const char *L) : Fl_Group(X, Y, W, H, L)
{
    channel_height = 13;
    peak_bar_width = 2;
    max_value = 0;
    min_value = -54.0;
    left_dB = -90;
    right_dB = -90;
    left_peak_dB = -90;
    right_peak_dB = -90;
    bg_color = color();
    width = w();
    height = h();
    x_origin = x();
    y_origin = y();
    left_chan_y1 = y_origin;
    left_chan_y2 = left_chan_y1 + channel_height - 1;
    right_chan_y1 = y_origin + height - channel_height;
    right_chan_y2 = right_chan_y1 + channel_height - 1;

    solid_green = fl_rgb_color((uchar)0, (uchar)210, (uchar)0);
    solid_yellow = fl_rgb_color((uchar)230, (uchar)230, (uchar)0);
    solid_red = fl_rgb_color((uchar)200, (uchar)0, (uchar)0);

    tick_distance_dB = 6;
    num_of_ticks = (0 - min_value) / tick_distance_dB;
    tick_distance_px = width / num_of_ticks;

    vu_end_xpos = x_origin + width;
}

void VUMeter::value(float left, float right, float left_peak, float right_peak)
{
    left_dB = left;
    right_dB = right;
    left_peak_dB = left_peak;
    right_peak_dB = right_peak;

    redraw();
}

int VUMeter::dB_to_xpos(float dB)
{
    float dB_span = max_value - min_value;

    int xpos = (int)(x_origin + width / dB_span * (dB_span - abs(dB)));

    if (xpos < x_origin) {
        xpos = x_origin;
    }

    return xpos;
}

void VUMeter::draw()
{
    Fl_Group::draw();

    float offset = abs(min_value);
    float left_vol = (left_dB + offset) / offset * width;
    float right_vol = (right_dB + offset) / offset * width;

    int x1;

    double red, green, blue, gradient;

    // Clear whole widget area
    fl_rectf(x_origin, y_origin, width, height, bg_color);

    if (cfg.gui.vu_mode == VU_MODE_SOLID) {
        fl_color(cfg.gui.vu_low_color);
        fl_rectf(x_origin, left_chan_y1, dB_to_xpos(left_dB) - x_origin, channel_height - 1);

        if (left_dB > cfg.gui.vu_mid_range_start) {
            fl_color(cfg.gui.vu_mid_color);
            fl_rectf(dB_to_xpos(cfg.gui.vu_mid_range_start), left_chan_y1, dB_to_xpos(left_dB) - dB_to_xpos(cfg.gui.vu_mid_range_start), channel_height - 1);
        }
        if (left_dB > cfg.gui.vu_high_range_start) {
            fl_color(cfg.gui.vu_high_color);
            fl_rectf(dB_to_xpos(cfg.gui.vu_high_range_start), left_chan_y1, dB_to_xpos(left_dB) - dB_to_xpos(cfg.gui.vu_high_range_start), channel_height - 1);
        }

        fl_color(cfg.gui.vu_low_color);
        fl_rectf(x_origin, right_chan_y1, dB_to_xpos(right_dB) - x_origin, channel_height - 1);
        if (right_dB > cfg.gui.vu_mid_range_start) {
            fl_color(cfg.gui.vu_mid_color);
            fl_rectf(dB_to_xpos(cfg.gui.vu_mid_range_start), right_chan_y1, dB_to_xpos(right_dB) - dB_to_xpos(cfg.gui.vu_mid_range_start), channel_height - 1);
        }
        if (right_dB > cfg.gui.vu_high_range_start) {
            fl_color(cfg.gui.vu_high_color);
            fl_rectf(dB_to_xpos(cfg.gui.vu_high_range_start), right_chan_y1, dB_to_xpos(right_dB) - dB_to_xpos(cfg.gui.vu_high_range_start),
                     channel_height - 1);
        }
    }
    else { // Gradient
        int gradient_offset = 10;
        for (int x_pos = 0; x_pos <= width - 2; x_pos += 1) {
            x1 = x_origin + x_pos;

            // fl_color(bg_color);
            if (left_vol > x_pos) {
                if (x1 < dB_to_xpos(cfg.gui.vu_mid_range_start) - gradient_offset) {
                    fl_color(cfg.gui.vu_low_color);
                }
                else if (x1 < dB_to_xpos(cfg.gui.vu_mid_range_start)) {
                    uchar r1, g1, b1;
                    uchar r2, g2, b2;
                    uchar r, g, b;
                    int c1 = cfg.gui.vu_low_color;
                    int c2 = cfg.gui.vu_mid_color;
                    r1 = (c1 & 0xFF000000) >> 24;
                    g1 = (c1 & 0x00FF0000) >> 16;
                    b1 = (c1 & 0x0000FF00) >> 8;
                    r2 = (c2 & 0xFF000000) >> 24;
                    g2 = (c2 & 0x00FF0000) >> 16;
                    b2 = (c2 & 0x0000FF00) >> 8;

                    // Interpolate color
                    int i = x1 - (dB_to_xpos(cfg.gui.vu_mid_range_start) - gradient_offset);
                    r = round(r1 + (float)i / gradient_offset * (r2 - r1));
                    g = round(g1 + (float)i / gradient_offset * (g2 - g1));
                    b = round(b1 + (float)i / gradient_offset * (b2 - b1));
                    fl_color(fl_rgb_color(r, g, b));
                }
                else if (x1 < dB_to_xpos(cfg.gui.vu_high_range_start) - gradient_offset) {
                    fl_color(cfg.gui.vu_mid_color);
                }
                else if (x1 < dB_to_xpos(cfg.gui.vu_high_range_start)) {
                    uchar r1, g1, b1;
                    uchar r2, g2, b2;
                    uchar r, g, b;
                    int c1 = cfg.gui.vu_mid_color;
                    int c2 = cfg.gui.vu_high_color;
                    int i = x1 - (dB_to_xpos(cfg.gui.vu_high_range_start) - gradient_offset);
                    r1 = (c1 & 0xFF000000) >> 24;
                    g1 = (c1 & 0x00FF0000) >> 16;
                    b1 = (c1 & 0x0000FF00) >> 8;
                    r2 = (c2 & 0xFF000000) >> 24;
                    g2 = (c2 & 0x00FF0000) >> 16;
                    b2 = (c2 & 0x0000FF00) >> 8;

                    r = round(r1 + (float)i / gradient_offset * (r2 - r1));
                    g = round(g1 + (float)i / gradient_offset * (g2 - g1));
                    b = round(b1 + (float)i / gradient_offset * (b2 - b1));
                    fl_color(fl_rgb_color(r, g, b));
                }
                else {
                    fl_color(cfg.gui.vu_high_color);
                }
                fl_line(x1, left_chan_y1, x1, left_chan_y2);
            }

            // fl_color(bg_color);
            if (right_vol > x_pos) {
                if (x1 < dB_to_xpos(cfg.gui.vu_mid_range_start) - gradient_offset) {
                    fl_color(cfg.gui.vu_low_color);
                }
                else if (x1 < dB_to_xpos(cfg.gui.vu_mid_range_start)) {
                    uchar r1, g1, b1;
                    uchar r2, g2, b2;
                    uchar r, g, b;
                    int c1 = cfg.gui.vu_low_color;
                    int c2 = cfg.gui.vu_mid_color;
                    r1 = (c1 & 0xFF000000) >> 24;
                    g1 = (c1 & 0x00FF0000) >> 16;
                    b1 = (c1 & 0x0000FF00) >> 8;
                    r2 = (c2 & 0xFF000000) >> 24;
                    g2 = (c2 & 0x00FF0000) >> 16;
                    b2 = (c2 & 0x0000FF00) >> 8;

                    int i = x1 - (dB_to_xpos(cfg.gui.vu_mid_range_start) - gradient_offset);
                    r = round(r1 + (float)i / gradient_offset * (r2 - r1));
                    g = round(g1 + (float)i / gradient_offset * (g2 - g1));
                    b = round(b1 + (float)i / gradient_offset * (b2 - b1));
                    fl_color(fl_rgb_color(r, g, b));
                }
                else if (x1 < dB_to_xpos(cfg.gui.vu_high_range_start) - gradient_offset) {
                    fl_color(cfg.gui.vu_mid_color);
                }
                else if (x1 < dB_to_xpos(cfg.gui.vu_high_range_start)) {
                    uchar r1, g1, b1;
                    uchar r2, g2, b2;
                    uchar r, g, b;
                    int c1 = cfg.gui.vu_mid_color;
                    int c2 = cfg.gui.vu_high_color;
                    int i = x1 - (dB_to_xpos(cfg.gui.vu_high_range_start) - gradient_offset);
                    r1 = (c1 & 0xFF000000) >> 24;
                    g1 = (c1 & 0x00FF0000) >> 16;
                    b1 = (c1 & 0x0000FF00) >> 8;
                    r2 = (c2 & 0xFF000000) >> 24;
                    g2 = (c2 & 0x00FF0000) >> 16;
                    b2 = (c2 & 0x0000FF00) >> 8;

                    r = round(r1 + (float)i / gradient_offset * (r2 - r1));
                    g = round(g1 + (float)i / gradient_offset * (g2 - g1));
                    b = round(b1 + (float)i / gradient_offset * (b2 - b1));
                    fl_color(fl_rgb_color(r, g, b));
                }
                else {
                    fl_color(cfg.gui.vu_high_color);
                }
                fl_line(x1, right_chan_y1, x1, right_chan_y2);
            }
        }
    }

    int left_peak_pos, right_peak_pos;

    fl_color(FL_BLACK);
    if (left_peak_dB >= left_dB) {
        left_peak_pos = dB_to_xpos(left_peak_dB);
        if (left_peak_pos >= vu_end_xpos) {
            left_peak_pos -= peak_bar_width + 2; // The +2 assures that the peak bar is visible at 0 dB
        }

        fl_rectf(left_peak_pos, left_chan_y1, peak_bar_width, channel_height - 1);
    }

    if (right_peak_dB >= right_dB) {
        right_peak_pos = dB_to_xpos(right_peak_dB);
        if (right_peak_pos >= vu_end_xpos) {
            right_peak_pos -= peak_bar_width + 2;
        }
        fl_rectf(right_peak_pos, right_chan_y1, peak_bar_width, channel_height - 1);
    }

    fl_draw_box(FL_THIN_DOWN_FRAME, x_origin, left_chan_y1, width, channel_height, bg_color);
    fl_draw_box(FL_THIN_DOWN_FRAME, x_origin, right_chan_y1, width, channel_height, bg_color);
    fl_draw_box(FL_THIN_DOWN_FRAME, x_origin, y_origin, width, height, FL_DARK1);

    fl_color(FL_BLACK);
    fl_font(FL_HELVETICA, 10);

#if defined(__APPLE__)
    fl_draw("L", x_origin - 9, y_origin + 8);
    fl_draw("R", x_origin - 9, y_origin + height - 3);
#else
    fl_draw("L", x_origin - 9, y_origin + 9);
    fl_draw("R", x_origin - 9, y_origin + height - 2);
#endif

    int tick_pos;
    char dB_val[4];
    fl_font(FL_HELVETICA, 9);

#if defined(__APPLE__)
    fl_draw("dB", x_origin - 13, y_origin + height / 2 + 2);
#else
    fl_draw("dB", x_origin - 13, y_origin + height / 2 + 3);
#endif
    for (int n = 1; n <= num_of_ticks; n++) {
        tick_pos = x_origin + n * tick_distance_px;

        fl_line(tick_pos, y_origin, tick_pos, y_origin + 3);
        fl_line(tick_pos, y_origin + height - 1, tick_pos, y_origin + height - 4);

        snprintf(dB_val, sizeof(dB_val), "%d", (int)min_value + n * tick_distance_dB);

#if defined(__APPLE__)
        fl_draw(dB_val, tick_pos - 5, y_origin + height / 2 + 2);
#else
        fl_draw(dB_val, tick_pos - 5, y_origin + height / 2 + 3);
#endif
    }
}
