
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define W 128
#define H 64

Adafruit_SSD1306 oled(W, H, &Wire, -1);

enum Mood {
  HAPPY, HUNGRY, SAD, SLEEP, PLAY, EAT, LEVELUP, NORMAL
};

Mood mood = HAPPY;

int hunger = 25;
int energy = 85;
int happiness = 80;
int level = 1;
int xp = 0;

unsigned long lastTick = 0;
unsigned long animStart = 0;
unsigned long lastFrame = 0;
unsigned long moodUntil = 0;

int animFrame = 0;
bool animating = false;
bool sleeping = false;

String evolution = "BABY";

void updateEvolution() {
  if (level >= 10) evolution = "CYBER";
  else if (level >= 5) evolution = "TEEN";
  else evolution = "BABY";
}

void drawHeart(int x, int y) {
  oled.fillCircle(x - 3, y, 4, SSD1306_WHITE);
  oled.fillCircle(x + 3, y, 4, SSD1306_WHITE);
  oled.fillTriangle(x - 7, y + 1, x + 7, y + 1,
                    x, y + 9, SSD1306_WHITE);
}

void drawFood(int x, int y) {
  oled.fillRoundRect(x, y, 16, 11, 3, SSD1306_WHITE);
  oled.fillCircle(x + 5, y + 5, 2, SSD1306_BLACK);
  oled.fillCircle(x + 11, y + 5, 2, SSD1306_BLACK);
  oled.drawLine(x + 8, y - 3, x + 8, y, SSD1306_WHITE);
}

void drawEyesOpen(int x, int y, int r) {
  oled.fillRoundRect(x - 11, y - 10, 22, 21, 6, SSD1306_WHITE);
  oled.fillCircle(x + 2, y, r, SSD1306_BLACK);
  oled.fillCircle(x + 3, y - 2, 2, SSD1306_WHITE);
}

void drawHappyEyes(int x, int y) {
  oled.drawLine(x - 10, y + 2, x - 5, y - 4, SSD1306_WHITE);
  oled.drawLine(x - 5, y - 4, x, y + 2, SSD1306_WHITE);
  oled.drawLine(x, y + 2, x + 5, y - 4, SSD1306_WHITE);
  oled.drawLine(x + 5, y - 4, x + 10, y + 2, SSD1306_WHITE);
}

void drawFace() {
  int eyeY = 31;
  int leftX = 38;
  int rightX = 90;

  // Animasi gerakan mata ketika bermain
  int offset = 0;
  if (mood == PLAY) {
    offset = (animFrame % 2 == 0) ? -4 : 4;
  }

  // Mata
  if (mood == SLEEP) {
    oled.drawLine(leftX - 11, eyeY, leftX + 11, eyeY,
                  SSD1306_WHITE);
    oled.drawLine(rightX - 11, eyeY, rightX + 11, eyeY,
                  SSD1306_WHITE);
  }
  else if (mood == HAPPY || mood == EAT ||
           mood == LEVELUP) {
    if (mood == LEVELUP && animFrame % 2 == 0) {
      drawEyesOpen(leftX, eyeY, 7);
      drawEyesOpen(rightX, eyeY, 7);
    } else {
      drawHappyEyes(leftX, eyeY);
      drawHappyEyes(rightX, eyeY);
    }
  }
  else if (mood == SAD) {
    drawEyesOpen(leftX, eyeY + 3, 4);
    drawEyesOpen(rightX, eyeY + 3, 4);
    oled.drawLine(leftX - 10, eyeY - 12,
                  leftX + 4, eyeY - 8, SSD1306_WHITE);
    oled.drawLine(rightX - 4, eyeY - 8,
                  rightX + 10, eyeY - 12, SSD1306_WHITE);
  }
  else if (mood == HUNGRY) {
    oled.drawCircle(leftX, eyeY, 9, SSD1306_WHITE);
    oled.drawCircle(rightX, eyeY, 9, SSD1306_WHITE);
    oled.fillCircle(leftX, eyeY, 3, SSD1306_WHITE);
    oled.fillCircle(rightX, eyeY, 3, SSD1306_WHITE);
  }
  else {
    drawEyesOpen(leftX + offset, eyeY, 4);
    drawEyesOpen(rightX + offset, eyeY, 4);
  }

  // Mulut berdasarkan ekspresi
  if (mood == SAD) {
    oled.drawLine(55, 49, 64, 43, SSD1306_WHITE);
    oled.drawLine(64, 43, 73, 49, SSD1306_WHITE);
  }
  else if (mood == HUNGRY || mood == EAT) {
    int r = (mood == EAT && animFrame % 2 == 0) ? 6 : 4;
    oled.fillCircle(64, 47, r, SSD1306_WHITE);
    if (r > 4) {
      oled.fillCircle(64, 47, 2, SSD1306_BLACK);
    }
  }
  else if (mood == SLEEP) {
    oled.drawCircle(64, 46, 3, SSD1306_WHITE);
  }
  else {
    oled.drawLine(53, 43, 58, 48, SSD1306_WHITE);
    oled.drawLine(58, 48, 70, 48, SSD1306_WHITE);
    oled.drawLine(70, 48, 75, 43, SSD1306_WHITE);
  }
}

String getMoodName() {
  if (sleeping || mood == SLEEP) return "TIDUR";
  if (mood == EAT) return "MAKAN";
  if (mood == PLAY) return "MAIN";
  if (mood == LEVELUP) return "LEVEL UP!";
  if (hunger >= 70) return "LAPAR";
  if (happiness <= 25) return "SEDIH";
  if (energy <= 20) return "LELAH";
  if (happiness >= 70) return "SENANG";
  return "NORMAL";
}

void drawScreen() {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);

  // Header
  oled.setCursor(0, 0);
  oled.print("CYBER PET ");
  oled.print(evolution);
  oled.setCursor(111, 0);
  oled.print(level);
  oled.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Efek animasi khusus
  if (mood == EAT) {
    int x = 52 + (animFrame % 3) * 5;
    drawFood(x, 15);
  }

  if (mood == LEVELUP) {
    // Bintang berganti posisi
    int p = animFrame % 4;
    oled.drawLine(12 + p * 5, 23, 16 + p * 5, 27,
                  SSD1306_WHITE);
    oled.drawLine(16 + p * 5, 23, 12 + p * 5, 27,
                  SSD1306_WHITE);
    oled.drawLine(102 - p * 5, 23, 106 - p * 5, 27,
                  SSD1306_WHITE);
    oled.drawLine(106 - p * 5, 23, 102 - p * 5, 27,
                  SSD1306_WHITE);
  }

  drawFace();

  if (mood == SLEEP) {
    oled.setCursor(99, 16);
    oled.print("Z");
    oled.setCursor(108, 12);
    oled.print("z");
    oled.setCursor(116, 8);
    oled.print("z");
  }

  if (mood == PLAY) {
    drawHeart(15, 30);
    drawHeart(113, 30);
  }

  // Panel bawah
  oled.drawLine(0, 53, 127, 53, SSD1306_WHITE);
  oled.setCursor(0, 55);
  oled.print("H");
  oled.print(hunger);
  oled.print(" E");
  oled.print(energy);
  oled.print(" XP");
  oled.print(xp);

  oled.setCursor(0, 63);
  // Baris ini terlalu rendah untuk sebagian font,
  // jadi status ditampilkan pada header Serial juga.

  oled.display();
}

void showStatus() {
  Serial.println("\n--- STATUS CYBER PET ---");
  Serial.print("Evolusi    : "); Serial.println(evolution);
  Serial.print("Level      : "); Serial.println(level);
  Serial.print("XP         : "); Serial.print(xp); Serial.println("/100");
  Serial.print("Lapar      : "); Serial.print(hunger); Serial.println("%");
  Serial.print("Energi     : "); Serial.print(energy); Serial.println("%");
  Serial.print("Bahagia    : "); Serial.print(happiness); Serial.println("%");
  Serial.print("Ekspresi   : "); Serial.println(getMoodName());
  Serial.println("------------------------");
}

void addXP(int amount) {
  xp += amount;

  while (xp >= 100) {
    xp -= 100;
    level++;
    updateEvolution();

    mood = LEVELUP;
    animating = true;
    animStart = millis();
    moodUntil = millis() + 3000;
    animFrame = 0;

    Serial.print("EVOLUSI! Sekarang: ");
    Serial.print(evolution);
    Serial.print(" - Level ");
    Serial.println(level);
  }
}

void action(String cmd) {
  cmd.trim();
  cmd.toUpperCase();

  if (cmd == "MAKAN") {
    if (sleeping) {
      Serial.println("Bangunkan pet dulu dengan BANGUN.");
      return;
    }
    hunger = max(0, hunger - 30);
    happiness = min(100, happiness + 5);
    mood = EAT;
    messageAction("NYAM! ENAK!");
    addXP(15);
    moodUntil = millis() + 2500;
  }
  else if (cmd == "MAIN") {
    if (sleeping) {
      Serial.println("Pet sedang tidur. Ketik BANGUN.");
      return;
    }
    if (energy < 15) {
      mood = SAD;
      Serial.println("Pet terlalu lelah. Ketik TIDUR.");
      moodUntil = millis() + 2000;
      return;
    }
    energy = max(0, energy - 15);
    hunger = min(100, hunger + 5);
    happiness = min(100, happiness + 20);
    mood = PLAY;
    messageAction("YEAY! SERU!");
    addXP(20);
    moodUntil = millis() + 3000;
  }
  else if (cmd == "TIDUR") {
    sleeping = true;
    mood = SLEEP;
    Serial.println("Zzz... pet sedang tidur.");
    moodUntil = 0;
  }
  else if (cmd == "BANGUN") {
    sleeping = false;
    mood = HAPPY;
    energy = min(100, energy + 5);
    messageAction("AKU BANGUN!");
    moodUntil = millis() + 2000;
  }
  else if (cmd == "SAPA") {
    happiness = min(100, happiness + 5);
    mood = HAPPY;
    messageAction("HALO TEMANKU!");
    addXP(5);
    moodUntil = millis() + 2000;
  }
  else if (cmd == "SEDIH") {
    happiness = max(0, happiness - 25);
    mood = SAD;
    messageAction("AKU SEDIH...");
    moodUntil = millis() + 3000;
  }
  else if (cmd == "STATUS") {
    showStatus();
  }
  else if (cmd == "BANTU") {
    Serial.println("\nMAKAN, MAIN, TIDUR, BANGUN, SAPA, SEDIH, STATUS");
  }
  else if (cmd.length() > 0) {
    Serial.println("Perintah tidak dikenal. Ketik BANTU.");
    return;
  }

  drawScreen();
}

void messageAction(String text) {
  Serial.print("CYBER PET: ");
  Serial.println(text);
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  Wire.begin(D2, D1);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED tidak terdeteksi. Cek alamat 0x3C/0x3D.");
    while (true) delay(100);
  }

  updateEvolution();
  drawScreen();

  Serial.println("\nCYBER PET v3.0 SIAP!");
  Serial.println("Ketik BANTU untuk daftar perintah.");
  lastTick = millis();
  lastFrame = millis();
}

void loop() {
  unsigned long now = millis();

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    action(cmd);
  }

  // Status berubah setiap 10 detik
  if (now - lastTick >= 10000) {
    lastTick = now;
    hunger = min(100, hunger + 2);

    if (sleeping) {
      energy = min(100, energy + 10);
      if (energy >= 100) {
        sleeping = false;
        mood = HAPPY;
        Serial.println("Pet bangun sendiri: energi penuh!");
      }
    } else {
      energy = max(0, energy - 2);
      happiness = max(0, happiness - 1);
    }

    drawScreen();
  }

  // Animasi gerak setiap 250 ms
  if (now - lastFrame >= 250) {
    lastFrame = now;
    animFrame++;
    if (mood == PLAY || mood == EAT || mood == LEVELUP) {
      drawScreen();
    }
  }

  // Kembalikan ekspresi setelah animasi selesai
  if (!sleeping && moodUntil > 0 && now >= moodUntil) {
    moodUntil = 0;
    mood = NORMAL;
    drawScreen();
  }

  // Level up selesai setelah animasi
  if (mood == LEVELUP && moodUntil > 0 && now >= moodUntil) {
    moodUntil = 0;
    mood = HAPPY;
    drawScreen();
  }
}
