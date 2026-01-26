#pragma once

// =============================================================================
// BUBBLE ANIMATION - Fermentation visualization for OLED
// =============================================================================
//
// This module provides animated bubble graphics to visualize sourdough
// fermentation activity. Bubbles rise from the bottom, bounce off jar
// walls, and respawn when they reach the surface.
// =============================================================================

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/**
 * Single bubble in the liquid
 *
 * Each bubble has position, velocity, and pulsating size effect
 * to create a more organic appearance.
 */
struct Bubble {
    float x, y;         // Position
    float vx, vy;       // Velocity (including horizontal drift)
    float baseR;        // Base radius
    float pulse;        // Pulsation phase
    float pulseSpeed;   // Pulsation speed

    /**
     * Randomize bubble properties and spawn at bottom
     *
     * @param left   Left boundary
     * @param right  Right boundary
     * @param bottom Bottom boundary (spawn area)
     * @param top    Top boundary
     */
    void randomize(int left, int right, int bottom, int top) {
        x = random(left, right);
        y = random(bottom - 10, bottom);
        vx = (random(-10, 10) * 0.03);        // Slight horizontal drift
        vy = -(0.4 + random(0, 10) * 0.05);   // Upward velocity
        baseR = random(1, 3);                  // Random size
        pulse = random(0, 100) * 0.1;          // Random phase
        pulseSpeed = 0.1 + random(0, 10) * 0.02;
    }

    /**
     * Get current radius (with pulsation effect)
     */
    float radius() const {
        return baseR + sin(pulse) * 0.5;
    }
};

/**
 * Bubble management system
 *
 * Handles physics, wall collisions, and respawning for all bubbles.
 */
class BubbleSystem {
public:
    static const int COUNT = 16;    // Number of bubbles
    Bubble bubbles[COUNT];

    int left, right, top, bottom;   // Boundaries

    BubbleSystem() {}

    /**
     * Initialize bubble system with boundaries
     *
     * @param L Left boundary
     * @param R Right boundary
     * @param T Top boundary
     * @param B Bottom boundary
     */
    void init(int L, int R, int T, int B) {
        left = L; right = R; top = T; bottom = B;
        for (int i = 0; i < COUNT; i++)
            bubbles[i].randomize(left, right, bottom, top);
    }

    /**
     * Update all bubble physics (call each frame)
     */
    void update() {
        for (int i = 0; i < COUNT; i++) {
            auto &b = bubbles[i];

            // Update pulsation
            b.pulse += b.pulseSpeed;

            // Apply velocity
            b.x += b.vx;
            b.y += b.vy;

            // Bounce off jar walls with damping
            if (b.x < left + 3 || b.x > right - 3)
                b.vx *= -0.8;

            // Respawn at bottom when bubble reaches surface
            if (b.y < top + 12)
                b.randomize(left, right, bottom, top);
        }
    }
};

/**
 * Jar renderer with 3D effect and liquid
 *
 * Draws a glass jar with pseudo-3D shading and animated liquid surface.
 */
class JarRenderer {
public:
    int x = 35;         // Left position
    int y = 2;          // Top position
    int w = 55;         // Width
    int h = 60;         // Height

    int waterLevel = 38;  // Liquid surface distance from top

    /**
     * Draw the complete jar with liquid
     *
     * @param d Reference to OLED display
     */
    void drawJar(Adafruit_SSD1306 &d) {
        // Jar lid
        d.fillRect(x, y - 3, w, 3, SSD1306_WHITE);

        // Jar outline
        d.drawRoundRect(x, y, w, h, 6, SSD1306_WHITE);

        // Inner glass reflection (3D effect)
        d.drawRoundRect(x+3, y+2, w-6, h-4, 6, SSD1306_WHITE);

        // Liquid fill with pseudo-gradient
        for (int i = 0; i < 18; i++) {
            int brightness = (i % 2 == 0);  // Alternating pattern
            d.drawLine(x+4, y + waterLevel + i,
                       x + w - 4, y + waterLevel + i, brightness);
        }

        // Animated wave on liquid surface
        for (int i = 0; i < w-8; i++) {
            int yy = y + waterLevel + sin((i + millis()*0.01) * 0.3) * 2;
            d.drawPixel(x + 4 + i, yy, SSD1306_WHITE);
        }
    }
};
