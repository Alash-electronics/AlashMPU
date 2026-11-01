// Минимальная имитация Arduino.h для запуска тестов на компьютере (g++).
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
using __FlashStringHelper = char;
#define F(x) ((const __FlashStringHelper *)(x))
#define HEX 16
#define DEC 10
inline uint32_t g_us = 1000000;
inline uint32_t micros() { return g_us; }
inline uint32_t millis() { return g_us / 1000; }
inline void delay(uint32_t ms) { g_us += ms * 1000; }
class Print {
 public:
  void print(const char *s) { printf("%s", s); }
  void print(char c) { printf("%c", c); }
  void print(unsigned v, int base = DEC) { printf(base == HEX ? "%X" : "%u", v); }
  void print(int v, int base = DEC) { printf(base == HEX ? "%X" : "%d", v); }
  void print(float v, int d = 2) { printf("%.*f", d, v); }
  void println(const char *s = "") { printf("%s\n", s); }
  void println(unsigned v, int base = DEC) { print(v, base); printf("\n"); }
  void println(int v, int base = DEC) { print(v, base); printf("\n"); }
  void println(float v, int d = 2) { print(v, d); printf("\n"); }
};
