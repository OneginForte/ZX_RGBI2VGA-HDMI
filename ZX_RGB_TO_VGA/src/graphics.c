//
// Title:	        Pico-mposite Graphics Primitives
// Description:		A hacked-together composite video output for the Raspberry Pi Pico
// Author:	        Dean Belfield
// Created:	        01/02/2021
// Last Updated:	02/03/2022
//
// Modinfo:
// 03/02/2022:      Fixed bug in print_char, typos in comments
// 05/02/2022:      Added support for colour
// 07/02/2022:      Added filled primitives
// 08/07/2022:      Optimised filled circle drawing
// 20/02/2022:      Added scroll_up, bitmap now initialised in cvideo.c
// 02/03/2022:      Added blit

//#include <math.h>

#include "memory.h"

#include "charset.h"            // The character set
//#include "cvideo.h"
#include "g_config.h"
#include "graphics.h"

uint8_t *bitmap;



void init_m(void) {
    bitmap = malloc(BUF_SZ);
    cls(0);
}

// Clear the screen
// - c: Background colour to fill screen with
//
void cls(unsigned char c) {
    memset(bitmap, c, BUF_SZ);
}

// Scroll the screen up
// - c: Background colour to fill blank area with
// - rows: Number of pixel rows to scroll up by
//
void scroll_up(unsigned char c, int rows) {
    memcpy(bitmap, &bitmap[BUF_W * rows], (V_BUF_H - rows) * BUF_W);
    memset(&bitmap[BUF_W * (BUF_H - rows)], c, rows * BUF_W);
}

// Print a character
// - x: X position on screen (pixels)
// - y: Y position on screen (pixels)
// - c: Character to print (ASCII 32 to 127)
// - bc: Background colour 
// - fc: Foreground colour 
//
void print_char(int x, int y, int c, unsigned char bc, unsigned char fc) {
    int char_index;
    unsigned char * ptr;

    if(c >= 32 && c < 128) {
        char_index = (c - 32) * 8;
        int x1 = x /2 ;
        ptr = &bitmap[(BUF_W)/2*y + x1 + 3];
        for(int row = 0; row < 8; row++) {
            unsigned char data = charset[char_index + row];
            for(int bit = 0; bit < 4; bit ++) {
                //uint8_t ch = i >> 4;
                //uint8_t cl = i & 0xf;
                if (x & 1)
                {
                    //unsigned char s = *(ptr - bit);
                    *(ptr - bit) = *(ptr - bit) | (((data >> bit) & 1) ? (fc & 0xf) : (bc & 0xf));
                    *(ptr - bit) = *(ptr - bit) | (((data >> (bit + 1)) & 1) ? (fc << 4) : (bc << 4));
                    bit++;
                }
                else 
                {
                    // unsigned char s = *(ptr - bit);
                    *(ptr - bit) = *(ptr - bit) | (((data >> bit) & 1) ? (fc << 4) : (bc << 4));
                    *(ptr - bit) = *(ptr - bit) | (((data >> (bit + 1)) & 1) ? (fc & 0xf) : (bc & 0xf));
                    bit++;
                    // packed = ((x & 0xf) << 4) | (y & 0xf)
                    // uint8_t c = ((i & 1) << 3) | (i >> 1);
                }
            }
            ptr += BUF_W/2;
        }
    }
}

// Print a string
// - x: X position on screen (pixels)
// - y: Y position on screen (pixels)
// - s: Zero terminated string
// - bc: Background 
// - fc: Foreground colour (
//
void print_string(int x, int y, char *s, unsigned char bc, unsigned char fc) {
    for(int i = 0; i < strlen(s); i++) {
        print_char(x + i * 8, y, s[i], bc, fc);
    }
}

// Plot a point
// - x: X position on screen
// - y: Y position on screen
// - c: Pixel colour
//
void plot(int x, int y, unsigned char c) {
    if (x >= 0 && x < BUF_W && y >= 0 && y < BUF_H)
    {
        bitmap[BUF_W * y + x] = c;
    }
}

// Draw a line
// - x1, y1: Coordinates of start point
// - x2, y2: Coordinates of last point
// - c: Pixel colour
//
// Draw a line
// - x1, y1: Coordinates of start point
// - x2, y2: Coordinates of last point
// - c: Pixel colour
//
void draw_line(int x1, int y1, int x2, int y2, unsigned char c) {
    int dx, dy, sx, sy, e, xp, yp, i;
    
    dx = x2 - x1;                   // Horizontal length
    dy = y2 - y1;                   // Vertical length

    sx = (dx > 0) - (dx < 0);       // Sign DX
    sy = (dy > 0) - (dy < 0);       // Sign DY

    dx *= sx;                       // Abs DX
    dy *= sy;                       // Abs DY

    if(dx == 0 && dy == 0) {        // For zero length lines
        plot(xp, yp, c);            // Just plot a point
        return;
    }

    xp = x1;                        // Start pixel coordinates
    yp = y1;

    if(dx > dy) {                   // If the line is longer than taller...
        e = dx >> 1;
        dy -= dx;
        for(i = 0; i < dx; i++) {
            plot(xp, yp, c);
            e += dy;
            if(e > 0) {
                yp += sy;
            }
            else {
                e += dx;
            }
            xp += sx;
        }
    }
    else {                          // If the line is taller than longer...
        e = dy >> 1;
        dx -= dy;
        for(i = 0; i < dy; i++) {
            plot(xp, yp, c);
            e += dx;
            if(e > 0) {
                xp += sx;
            }
            else {
                e += dy;
            }
            yp += sy;
        }
    }
}

// Draw a circle
// - x: X Coordinate 
// - y: Y Coordinate
// - r: Radius
// - c: Pixel colour
// - filled: Set to false to draw wireframe, true for filled
//
void draw_circle(int x, int y, int r, unsigned char c, bool filled) {
    int xp = 0;
    int yp = r;
    int d = 3 - r * 2;
    while (yp >= xp) {                  // The circle routine only plots an octant
        if(filled) {
            draw_horizontal_line(y + xp, x - yp, x + yp, c);
            draw_horizontal_line(y - xp, x - yp, x + yp, c);
        }
        else {
            plot(x + xp, y + yp, c);    // So use symmetry to draw a complete circle
            plot(x + xp, y - yp, c);   
            plot(x - xp, y + yp, c);   
            plot(x - xp, y - yp, c);           
            plot(x + yp, y + xp, c);
            plot(x + yp, y - xp, c);   
            plot(x - yp, y + xp, c);   
            plot(x - yp, y - xp, c);
        }
        xp++;
        if (d > 0) {
            if(filled) {                // We only need to draw these bits when the Y coordinate changes
              draw_horizontal_line(y + yp, x - xp, x + xp, c);
              draw_horizontal_line(y - yp, x - xp, x + xp, c);
            }
            yp--;
            d += 4 * (xp - yp) + 10;
        }
        else {
            d += 4 * xp + 6;
        }
    }
}

// Draw a polygon
// - x1 ... x3: X coordinates
// - y1 ... y3: Y coordinates
// - c: Pixel colour
// - filled: Set to false to draw wireframe, true for filled
//
void draw_polygon(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned char c, bool filled) {
    if(filled) {
        draw_triangle(x1, y1, x2, y2, x4, y4, c, true);
        draw_triangle(x2, y2, x3, y3, x4, y4, c, true);
    }
    else {
        draw_line(x1, y1, x2, y2, c);
        draw_line(x2, y2, x3, y3, c);
        draw_line(x3, y3, x4, y4, c);
        draw_line(x4, y4, x1, y1, c);
    }
}

// Draw a  triangle
// - x1 ... x3: X coordinates
// - y1 ... y3: Y coordinates
// - c: Pixel colour
// - filled: Set to false to draw wireframe, true for filled
//
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned char c, bool filled) {
    if(!filled) {
        draw_line(x1, y1, x2, y2, c);
        draw_line(x2, y2, x3, y3, c);
        draw_line(x3, y3, x1, y1, c);
        return;
    }

    struct Line line1;
    struct Line line2;
    
    int i;

    //
    // First sort the points by Y ascending
    // 
    if(y1 > y3) {       // a
        swap(&x1, &x3);
        swap(&y1, &y3);
    }
    if(y1 > y2) {
        swap(&x1, &x2); //b
        swap(&y1, &y2);
    }
    if(y2 > y3) {       // c
        swap(&x2, &x3);
        swap(&y2, &y3);
    }
    
    // Now draw the longest line (a->c) and (a->b) together
    //
    init_line(&line1, x1, y1, x3, y3);  // a
    init_line(&line2, x1, y1, x2, y2);  // b

    while(line2.h > 0) {
        draw_horizontal_line(line1.yp, line1.xp, line2.xp, c);
        step_line(&line1);
        step_line(&line2);
        line1.yp++;
        line2.h--;
    }

    // And finish with line b->c
    //
    init_line(&line2, x2, y2, x3, y3);  // c

    while(line2.h > 0) {
        draw_horizontal_line(line1.yp, line1.xp, line2.xp, c);
        step_line(&line1);
        step_line(&line2);
        line1.yp++;
        line2.h--;
    }

    draw_horizontal_line(line1.yp, line1.xp, line2.xp, c);
}

// Optimised horizontal line
// - y1: Y coordinate
// - x1: First X coordinate
// - X2: Second X coordinate
// - c: Colour
//
void draw_horizontal_line(int y1, int x1, int x2, int c) {
    if(x1 > x2) {                       // Always draw the line from left to right
        swap(&x2, &x1);
    }
    if(x1 < 0) {                        // If x1 < 0
        if(x2 < 0) {                    // If x2 is also < 0
            return;                     // Don't need to draw
        }
        x1 = 0;                         // Clip x1 to 0
    }
    if (x2 > BUF_W)
    {                                   // if x2 > width
        if (x1 > BUF_W)
        {                               // if x1 is also > width
            return;                     // Don't need to draw
        }
        x2 = BUF_W;                   // Clip x2 to width
    }
//  for(int i = x1; i <= x2; i++) {     // This is slow...
//      plot(i, y1, c);                 // so we'll use memset to fill the line in memory
//  }
    memset(&bitmap[BUF_W * y1 + x1], c, x2 - x1 + 1);
}

// Swap two numbers
// - a: Integer 1
// - b: Integer 2
//
void swap(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}

// Iniitialise a line structure
// Used for filled triangle routine
// - line: Pointer to a line structure
// - x1, y1: Coordinates of start point
// - x2, y2: Coordinates of last point
//
void init_line(struct Line *line, int x1, int y1, int x2, int y2) {
    line->dx = x2 - x1;
    line->dy = y2 - y1;
    line->sx = (line->dx > 0) - (line->dx < 0); // Sign DX
    line->sy = (line->dy > 0) - (line->dy < 0); // Sign DY
    line->dx *= line->sx;                       // Abs DX
    line->dy *= line->sy;                       // Abs DY
    line->xp = x1;                              // Start pixel coordinates
    line->yp = y1;
    line->quad = line->dx > line->dy;           // True if line is longer than taller 
    line->h = line->dy;
    if(line->quad) {                   
        line->e = line->dx >> 1;
        line->dy -= line->dx;
    }
    else {
        line->e = line->dy >> 1;
        line->dx -= line->dy;
    }
}

// Do one step of the line
// Used for filled triangle routine
// - line: Pointer to a line structure
// Returns
// - true if the line has incremented onto the next pixel row
//
void step_line(struct Line *line) {
    if(line->quad) {
        while(true) {
            line->e += line->dy;
            if(line->e > 0) {
                line->xp += line->sx;
                break;
            }
            else {
                line->xp += line->sx;
                line->e += line->dx;
            }
        }
    }
    else {                        
        line->e += line->dx;
        if(line->e > 0) {
            line->xp += line->sx;
        }
        else {
            line->e += line->dy;
        }
    }
}

// Blit (non-scaling)
// - data: Source data
// - sx, sy: Source X and Y in array of pixels
// - sw, sh: Source width and height
// - dx, dy: Destination X and Y on screen
//
void blit(const void * data, int sx, int sy, int sw, int sh, int dx, int dy) {
    void * src = (void *)data + (sw * sy) + sx;
    void *dst = bitmap + (BUF_W * dy) + dx;
    for(int i = 0; i < sh; i++) {
        memcpy(dst, src, sw);
        dst += BUF_W;
        src += sw;
    }
}