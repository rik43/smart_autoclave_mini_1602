#include "global.h"
#include "const.h"
#include "timer.h"
#include "app/app.h"
#include "net/connector.h"
#include "time/TimerTimeout.h"
#include "settings/SettingsStore.h"
// #include "display/Display.h"
#include "utils/LEDManager.h"
#include "ui/AppController.h"


LEDManager ledManager;

TimerTimeout timer;

SettingsStore settings;

// DwinDisplay dwinDisplay(Serial2);
// Display display(dwinDisplay, settings);

AppController appController(ledManager);

App app(settings);

Connector connector(settings, appController);


IRAM_ATTR void enc_isr() {
  appController.enc.tickISR();
}
IRAM_ATTR void enc_btn_isr() {
  appController.enc.pressISR();
}


void printVersion();

void setupTick() {
  static uint8_t tick;
  tick++;
  ledManager.bits(tick);
}


void setup() {
  setupTick();

  delay(300);

  setupTick();

  Serial.begin(115200); // DEBUG
  Serial2.begin(115200); // DWIN
  pinMode(PIN_RELAY, OUTPUT);

  setupTick();

  settings.start();

  setupTick();

  // display.setup();  //delay(1000); // wait for reset display
  appController.begin();

  setupTick();

  // connector.setDisplay(&display);
  connector.start();
  
  setupTick();

  delay(1500); // пауза перед сменой заставки на экран

  setupTick();

  connector.publishConnectionState(); // update status icons on display

  setupTick();

  // display.start();

  setupTick();

  app.setup();

  setupTick();

  printVersion();

  setupTick();

  appController.enc.setEncISR(true);
  attachInterrupt(digitalPinToInterrupt(ENCODER_S1), enc_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_S2), enc_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_KEY), enc_btn_isr, FALLING);


  //DwinFirmwareUpdate updater(Serial2, 13, "http://192.168.1.35:3535/dwin/13TouchFile.bin");
  //DwinFirmwareUpdate updater(Serial2, 13, "https://smart-cloud-dev.zagk.ru/fw/av-a1/scr/0/13TouchFile.bin");
  //updater.updateBlock();

  //ledManager.normalMode();
  //ledManager.leds[0].pulse(0, 2000);
  //ledManager.leds[1].pulse(0, 1000);
  //ledManager.leds[2].blink(0, 500, 500);
  //ledManager.leds[3].blink(0, 250, 250);
  // ledManager.pulseIdle();
  ledManager.normalMode();
  ledManager.leds[0].pulse(1000);
  ledManager.leds[1].ledOff();
}

void loop() {

  appController.loop();

  // display.loop();

  connector.loop();

  app.loop();

  timer.loop();

  //ledManager.loop();

  vTaskDelay(pdMS_TO_TICKS(1)); // delay 1ms for other tasks
}



// Структура для поиска номера версии в бинарнике на сервере
// ПО на сервере будет искать маркеры начала и конца и извлекать код между ними
struct __attribute__((packed)) VersionInfo {
    uint32_t versionMarkerStart = 0x12345678; // Start marker
    uint32_t firmwareVersion = FIRMWARE_VERSION; // версия ПО
    uint32_t versionMarkerEnd = 0x87654321; // End marker
} versionInfo;
 
void printVersion() {
  Serial.print("FW version: ");
  Serial.println(versionInfo.firmwareVersion); // строка важна чтобы компилятор не удалил переменную
  Serial.print("HW version: ");
  Serial.println(settings.getHardwareVersion());
}



/*
// + в файле global.cpp закомментировать 3 строки с "connector"
#include "global.h"
#include "const.h"
#include "app/app.h"
#include "settings/SettingsStore.h"

SettingsStore settings;
App app(settings);

void setup() {
  Serial.begin(115200); // DEBUG
  Serial2.begin(115200); // DWIN
  pinMode(PIN_RELAY, OUTPUT);

  app.setup();

  //uint32_t pid = 1;
  uint32_t id = 1;

  uint32_t startMem = ESP.getFreeHeap();

  for (uint32_t pid = 1; pid <= 3; ++pid) {
    prt("====================================", pid);

    uint32_t startCycleMem = ESP.getFreeHeap();

    prt(" START HEAP ========= ", ESP.getFreeHeap());

    app.compiler.create(pid, pid);

    uint32_t line = 0;

    app.compiler.addLine(pid, line++, "<P5");
    app.compiler.addLine(pid, line++, "@TMP1:float@");
    app.compiler.addLine(pid, line++, "@TMP2:float@");
    app.compiler.addLine(pid, line++, "@TMP3:float@");
    app.compiler.addLine(pid, line++, "@TMP4:float@");
    app.compiler.addLine(pid, line++, "@NEXT:bool@");
    app.compiler.addLine(pid, line++, ">P");
    app.compiler.addLine(pid, line++, "<B1");
    app.compiler.addLine(pid, line++, "{2");
    //app.compiler.addLine(pid, line++, "@Compare@set={TMP1}&a=1&sign=<&b=2");
    //app.compiler.addLine(pid, line++, "@Compare@set={TMP2}&a=1&sign=<&b=2");
    // app.compiler.addLine(pid, line++, "@S@set={TMP1}&a=1&b=1");
    app.compiler.addLine(pid, line++, "@S@set={TMP1}&a=1&b=2&c=3");
    // app.compiler.addLine(pid, line++, "@S@set={TMP2}&a=1");
    app.compiler.addLine(pid, line++, "@Next@");
    app.compiler.addLine(pid, line++, "}");
    app.compiler.addLine(pid, line++, ">B");

    prt("       INIT ========= ", ESP.getFreeHeap());

    app.programStart(pid);

    //prt("     UPLOAD ========= ", ESP.getFreeHeap()); // == RUN HEAP

    app.loop();

    prt("        RUN ========= ", ESP.getFreeHeap());

    app.programStop(false);

    prt("       STOP ========= ", ESP.getFreeHeap());

    uint32_t endCycleMem = ESP.getFreeHeap();
    prt("MEM LEAK ", startCycleMem - endCycleMem);
  }

  uint32_t endMem = ESP.getFreeHeap();

  prt("================ TOTAL MEM LEAK ", startMem - endMem);
}

void loop() {
  delay(1000);
}
*/