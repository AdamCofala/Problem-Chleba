
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// -------------------------------------------------------------
// BUBBLE – pojedynczy bąbelek w cieczy
// -------------------------------------------------------------
struct Bubble {
    float x, y;         // pozycja
    float vx, vy;       // prędkość (w tym boczna)
    float baseR;        // bazowy promień
    float pulse;        // faza pulsowania
    float pulseSpeed;   // szybkość pulsowania

    void randomize(int left, int right, int bottom, int top) {
        x = random(left, right);
        y = random(bottom - 10, bottom);
        vx = (random(-10, 10) * 0.03);    // trochę bocznego dryfu
        vy = -(0.4 + random(0, 10) * 0.05);
        baseR = random(1, 3);
        pulse = random(0, 100) * 0.1;
        pulseSpeed = 0.1 + random(0, 10) * 0.02;
    }

    float radius() const {
        return baseR + sin(pulse) * 0.5;  // pulsowanie
    }
};

// -------------------------------------------------------------
// BUBBLE SYSTEM – zarządza ruchem, odbiciami, respawnami
// -------------------------------------------------------------
class BubbleSystem {
public:
    static const int COUNT = 16;
    Bubble bubbles[COUNT];

    int left, right, top, bottom;

    BubbleSystem() {}

    void init(int L, int R, int T, int B) {
        left = L; right = R; top = T; bottom = B;
        for (int i = 0; i < COUNT; i++)
            bubbles[i].randomize(left, right, bottom, top);
    }

    void update() {
        for (int i = 0; i < COUNT; i++) {
            auto &b = bubbles[i];

            // Update pulsowania
            b.pulse += b.pulseSpeed;

            // Ruch
            b.x += b.vx;
            b.y += b.vy;

            // Odbicia od ścian słoika
            if (b.x < left + 3 || b.x > right - 3)
                b.vx *= -0.8;  // odbicie z tłumieniem

            // Zniknął nad cieczą → respawn na dole
            if (b.y < top + 12)
                b.randomize(left, right, bottom, top);
        }
    }
};

// -------------------------------------------------------------
// JAR RENDERER – wszystko co rysuje słoik 3D + gradient cieczy
// -------------------------------------------------------------
class JarRenderer {
public:
    int x = 35;     // lewy punkt
    int y = 2;      // górny punkt
    int w = 55;     // szerokość
    int h = 60;     // wysokość

    int waterLevel = 38; // od góry

    void drawJar(Adafruit_SSD1306 &d) {

        // --- Pokrywka ---
        d.fillRect(x, y - 3, w, 3, SSD1306_WHITE);

        // --- Słoik outline ---
        d.drawRoundRect(x, y, w, h, 6, SSD1306_WHITE);

        // --- Wewnętrzne odbicie szkła ---
        d.drawRoundRect(x+3, y+2, w-6, h-4, 6, SSD1306_WHITE);

        // --- Gradient cieczy ---
        for (int i = 0; i < 18; i++) {
            int brightness = (i % 2 == 0);  // pseudo-gradient
            d.drawLine(x+4, y + waterLevel + i, 
                       x + w - 4, y + waterLevel + i, brightness);
        }

        // --- Fala na powierzchni cieczy ---
        for (int i = 0; i < w-8; i++) {
            int yy = y + waterLevel + sin((i + millis()*0.01) * 0.3) * 2;
            d.drawPixel(x + 4 + i, yy, SSD1306_WHITE);
        }
    }
};
