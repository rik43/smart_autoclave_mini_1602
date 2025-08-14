#pragma once

#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define prt(x, y) Serial.print(x); Serial.print(": "); Serial.println(y)
#else 
#define debug(x)
#define debugln(x)
#define prt(x, y)
#endif

class Event;
class Command;

// отправить событие на контроллера/телефон
void emit(Event& event);

// получение команды с контроллера/телефона
void dispatch(const char* message);

// отправка внутренней команды (например с дисплея)
void dispatch(Command* cmd); // @deprecated

void lcdDraw();
void startAutoclaveProcess();
void stopAutoclaveProcess();
