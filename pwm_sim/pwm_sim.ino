#include "TimerOne.h"
#include "Thread.h"
#include "StaticThreadController.h"

#define PWM_PIN 9
#define PERIOD 20000
#define LOWEST 1100
#define HIGHEST 1880
#define STABLE_PULSE_WIDTH 1500
#define SWITCH_DELAY 10
const unsigned long STABLE_DUTY_CYCLE = round(STABLE_PULSE_WIDTH / 20000.0 * 1023.0);
const unsigned long AMPLITUDE = (HIGHEST - LOWEST) / 2;
const unsigned long VERTICAL_DISPLACEMENT = HIGHEST - AMPLITUDE;

bool START = false;
unsigned long WAVE_PERIOD = 2000;
unsigned long DUTY_CYCLE = 0;

Thread pwm_update_thread = Thread();
Thread io_thread = Thread();
Thread led_thread = Thread();
StaticThreadController<3> controller(&pwm_update_thread, &io_thread, &led_thread);

void setup() {
  // put your setup code here, to run once:
  pinMode(PWM_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Timer1.initialize(PERIOD);
  Timer1.pwm(PWM_PIN, STABLE_DUTY_CYCLE);
  Serial.begin(9600);
  pwm_update_thread.setInterval(20);
  pwm_update_thread.onRun(update_duty_cycle);
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
    WAVE_PERIOD = inp.toInt();
    if (WAVE_PERIOD <= 0) {
      Serial.println("Invalid input");
      return;
    }
    START = true;

    DUTY_CYCLE = round(WAVE_PERIOD / 20000.0 * 1023);
    Serial.println(DUTY_CYCLE);

    Timer1.setPwmDuty(PWM_PIN, DUTY_CYCLE);
    // Timer1.setPwmDuty(PWM_PIN, DUTY_CYCLE);
    return;
  }
  String inp = Serial.readString();
  int temp = inp.toInt();
  if (temp != 0) {
    Timer1.setPwmDuty(PWM_PIN, STABLE_DUTY_CYCLE);
    delay(SWITCH_DELAY);
    Timer1.setPwmDuty(PWM_PIN, round(temp / 20000.0 * 1023.0));
    return;
  }
  inp.trim();
  if (inp == "q") {
    START = false;
    Timer1.setPwmDuty(PWM_PIN, STABLE_DUTY_CYCLE);
  }
}

void update_duty_cycle() {
  // if (!START) {
  //   return;
  // }
  // double pulse_width = AMPLITUDE * sin(2 * PI / WAVE_PERIOD * millis()) + VERTICAL_DISPLACEMENT;
  // Serial.println(pulse_width);
  // DUTY_CYCLE = round(pulse_width / PERIOD * 1023.0);
  // // Serial.println(DUTY_CYCLE);
  // Timer1.setPwmDuty(PWM_PIN, DUTY_CYCLE);
}

void blink_led() {
  if (!START) {
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
}