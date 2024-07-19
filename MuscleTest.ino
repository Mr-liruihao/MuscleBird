#include "Arduino_GigaDisplayTouch.h"
#include "Arduino_GigaDisplay_GFX.h"

#define INPUT_PIN A0

#define CYAN    0x07FF
#define RED     0xf800
#define BLUE    0x001F
#define GREEN   0x07E0
#define MAGENTA 0xF81F
#define WHITE   0xffff
#define BLACK   0x0000
#define YELLOW  0xFFE0

GigaDisplay_GFX tft;
Arduino_GigaDisplayTouch touchDetector;

int wide = 480;
int high = 800;
int counter;

void resetArduino() {
  NVIC_SystemReset();
}

class Rectangle {
public:
    int x, y, width, height;

    // Default Constructor
    Rectangle() : x(0), y(0), width(0), height(0) {}

    // Parameterized Constructor
    Rectangle(int a, int b, int wide, int high)
        : x(a), y(b), width(wide), height(high) {}

    // Method to draw the rectangle on a display (assuming tft is your display object)
    void draw(GigaDisplay_GFX &tft, uint16_t color) const {
        tft.fillRect(x, y, width, height, color);
    }

    // Method to check if a point is inside the rectangle
    bool contains(int px, int py) const {
        return (px >= x && px <= x + width && py >= y && py <= y + height);
    }

    // Method to check if this rectangle intersects with another rectangle
    bool intersects(const Rectangle &other) const {
        return !(other.x > x + width ||
                 other.x + other.width < x ||
                 other.y > y + height ||
                 other.y + other.height < y);
    }
};

class Ball {
public:
    int rad;
    int posx;
    int posy;
    double velx;
    double vely;
    Rectangle hitbox;

    // Default Constructor
    Ball() : hitbox(posx, posy, 2 * rad, 2 * rad)
    {
      posx = 120;
      posy = 2 * 10;
      velx = 0;
      vely = 0;
      rad = 10;
      updatehitbox();
    }

    void update() {
        vely += 0.4; // Acceleration
        posx += velx;
        posy += vely;
        if (posy + rad > high) {
            vely = (vely * -0.8);
        }
        if(posy - rad < 0)
        {
          vely = 0;
          posy =  2 * rad;
        }
        draw();
        updatehitbox();
    }

    void updatehitbox() {
        hitbox.x = posx - rad;
        hitbox.y = posy - rad;
        hitbox.width = 2 * rad;
        hitbox.height = 2 * rad;
    }

    void draw() const {
        tft.fillCircle(posx, posy, rad, BLACK);
    }
};

class Pipe {
public:
    Rectangle Top;
    Rectangle Bot;
    int posx;
    bool scored;
    int pipewide;

    // Default Constructor
    Pipe()
        : posx(wide), scored(false), pipewide(40),
          Top(posx, random(0, 0.45 * high) - high, pipewide, high),
          Bot(posx, Top.y + high + 80, pipewide, high) {}

    Pipe(Rectangle top, Rectangle bot)
    {
      pipewide = 40;
      Top = top;
      Bot = bot;
    }

    void update() {
        posx -= 5;
        Top.x = posx;
        Bot.x = posx;
        draw();
       }

    void draw() const {
        tft.fillRect(Top.x, Top.y, pipewide, high, WHITE);
        tft.fillRect(Bot.x, Bot.y, pipewide, high, WHITE);
    }
};

Ball pinball;
//one pipe on the screen for ez, two for mid, three for hard later
Pipe obstacle; 

void setup() {
    tft.begin();
    touchDetector.begin();
    counter = 0;
}

void loop() {
    
    tft.fillScreen(CYAN);
    pinball.update();
    obstacle.update();
      if(obstacle.Top.contains(pinball.posx, pinball.posy))
      {
       resetArduino();
      }
      if(obstacle.Bot.contains(pinball.posx, pinball.posy))
      {
       resetArduino();
      }
    if(obstacle.Top.x + obstacle.pipewide < 0)
    {
       Pipe temp;
       obstacle = temp;
     }

    float sensor_value = analogRead(A1);
    float signal = EMGFilter(sensor_value);

    uint8_t contacts;
    GDTpoint_t points[5];

    contacts = touchDetector.getTouchPoints(points);
    if (contacts > 0) {
        pinball.vely = -5;
    }
     if (abs(signal) > 35) {
         pinball.vely -= 1;
     }
    delay(1);
  Serial.println(signal);
}

float EMGFilter(float input) {
    float output = input;
    static float z1, z2;
    float x = output - 0.05159732 * z1 - 0.36347401 * z2;
    output = 0.01856301 * x + 0.03712602 * z1 + 0.01856301 * z2;
    z2 = z1;
    z1 = x;

    x = output - -0.53945795 * z1 - 0.39764934 * z2;
    output = 1.00000000 * x + -2.00000000 * z1 + 1.00000000 * z2;
    z2 = z1;
    z1 = x;

    x = output - 0.47319594 * z1 - 0.70744137 * z2;
    output = 1.00000000 * x + 2.00000000 * z1 + 1.00000000 * z2;
    z2 = z1;
    z1 = x;

    x = output - -1.00211112 * z1 - 0.74520226 * z2;
    output = 1.00000000 * x + -2.00000000 * z1 + 1.00000000 * z2;
    z2 = z1;
    z1 = x;

    return output;
}

