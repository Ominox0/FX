#pragma once
#include <string>

typedef std::string (*ReadFileFn)(const std::string& path);
typedef void (*PrintFn)(const std::string& s);
typedef void (*DrawRectFn)(int x, int y, int w, int h, unsigned short color, bool fill);
typedef void (*DrawTextFn)(int x, int y, const std::string& text, unsigned short color, int size);
typedef bool (*TouchReadFn)(int* x, int* y, bool* pressed);

struct FxInterpreterHandle;

FxInterpreterHandle* fx_create(ReadFileFn readFile, PrintFn print);
void fx_load(FxInterpreterHandle* h, const std::string& mainPath);
void fx_call_setup(FxInterpreterHandle* h);
void fx_call_loop(FxInterpreterHandle* h, float dt);
void fx_set_display(FxInterpreterHandle* h, DrawRectFn drawRect, DrawTextFn drawText);
void fx_set_touch(FxInterpreterHandle* h, TouchReadFn touchRead);
const char* fx_last_error(FxInterpreterHandle* h);
void fx_clear_error(FxInterpreterHandle* h);
void fx_destroy(FxInterpreterHandle* h);
