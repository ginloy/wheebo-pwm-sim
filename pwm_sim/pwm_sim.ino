#include "TimerOne.h"
#include "Thread.h"
#include "StaticThreadController.h"

#define PWM_PIN 9
#define PERIOD 20000
#define STABLE_PULSE_WIDTH 1500
#define SWITCH_DELAY 20
#define FULL_DUTY 1023

unsigned long calculate_duty_cycle(unsigned long pulse_width_micros);
const unsigned long STABLE_DUTY_CYCLE = calculate_duty_cycle(STABLE_PULSE_WIDTH);

bool START = false;

Thread io_thread = Thread();
Thread led_thread = Thread();
StaticThreadController<2> controller(&io_thread, &led_thread);

void setup() {
  // put your setup code here, to run once:
  pinMode(PWM_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Timer1.initialize(PERIOD);
  Timer1.pwm(PWM_PIN, STABLE_DUTY_CYCLE);
  Serial.begin(9600);
  io_thread.setInterval(20);
  io_thread.onRun(io);
  led_thread.setInterval(100);
  led_thread.onRun(blink_led);
}

void loop() {
  controller.run();
}

void io() {
  if (!Serial.available()) {
    return;
  }
  if (!START) {
    String inp = Serial.readString();
    long pulse_width = inp.toInt();
    if (pulse_width <= 0 || pulse_width > PERIOD) {
      Serial.println("Invalid input");
      return;
    }
    START = true;

    Timer1.setPwmDuty(PWM_PIN, calculate_duty_cycle(pulse_width));
    return;
  }
  String inp = Serial.readString();
  long pulse_width = inp.toInt();
  if (pulse_width != 0) {
    if (pulse_width > 1023) {
      Serial.println("Invalid input");
      return;
    }
    Timer1.setPwmDuty(PWM_PIN, STABLE_DUTY_CYCLE);
    delay(SWITCH_DELAY);
    Timer1.setPwmDuty(PWM_PIN, calculate_duty_cycle(pulse_width));
    return;
  }
  inp.trim();
  if (inp == "q") {
    START = false;
    Timer1.setPwmDuty(PWM_PIN, STABLE_DUTY_CYCLE);
  }
}

void blink_led() {
  if (!START) {
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
}

unsigned long calculate_duty_cycle(unsigned long pulse_width_micros) {
  return round((float)pulse_width_micros / PERIOD * FULL_DUTY);
}