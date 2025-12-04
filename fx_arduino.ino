#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "fx_core.h"

static FxInterpreterHandle* H = nullptr;
static bool panicMode = false;
static std::vector<std::string> errLog;
static bool readRegFT6206(uint8_t reg, uint8_t* buf, size_t len){ Wire.beginTransmission(0x38); Wire.write(reg); if(Wire.endTransmission(false)!=0) return false; size_t got = Wire.requestFrom((int)0x38, (int)len); if(got!=len) return false; for(size_t i=0;i<len;++i){ buf[i]=Wire.read(); } return true; }
static bool readTouchFT6206(int* x, int* y, bool* pressed){ uint8_t buf[7]; if(!readRegFT6206(0x02, buf, 7)) { *pressed=false; return false; } uint8_t n = buf[0]; if(n==0){ *pressed=false; return false; } uint16_t rx = ((buf[1]&0x0F)<<8) | buf[2]; uint16_t ry = ((buf[3]&0x0F)<<8) | buf[4]; *x = (int)rx; *y = (int)ry; *pressed=true; return true; }
static void drawRectStub(int x,int y,int w,int h,unsigned short color,bool fill){}
static void drawTextStub(int x,int y,const std::string& text,unsigned short color,int size){}
static std::string trim(const std::string& s){ size_t a=0; while(a<s.size() && (s[a]==' '||s[a]=='\t'||s[a]=='\r'||s[a]=='\n')) a++; size_t b=s.size(); while(b>a && (s[b-1]==' '||s[b-1]=='\t'||s[b-1]=='\r'||s[b-1]=='\n')) b--; return s.substr(a,b-a); }

static std::string readFileSd(const std::string& path){
  File f = SD.open(path.c_str(), FILE_READ);
  if(!f) return std::string();
  std::string out;
  while(f.available()){
    out.push_back((char)f.read());
  }
  f.close();
  return out;
}

static void printSerial(const std::string& s){ Serial.print(s.c_str()); }

static void addLog(const std::string& msg){ unsigned long t = millis(); char buf[64]; snprintf(buf, sizeof(buf), "[%lu] : ", t); errLog.push_back(std::string(buf)+msg); }
static void renderError(){ if(!panicMode) return; if(drawRectStub){ drawRectStub(0,0,240,320,0,true); } if(drawTextStub){ size_t maxLines = 12; size_t start = errLog.size()>maxLines? errLog.size()-maxLines : 0; for(size_t i=0;i<maxLines && (start+i)<errLog.size(); ++i){ drawTextStub(4, 20 + (int)i*20, errLog[start+i], 65535, 1); } drawTextStub(4, 300, "[end of log]", 65535, 1); drawRectStub(70, 260, 100, 30, 65535, false); drawTextStub(80, 270, "Try reboot", 65535, 1); } }

void setup(){
  Serial.begin(115200);
  SD.begin(5);
  Wire.begin();
  H = fx_create(readFileSd, printSerial);
  fx_set_touch(H, readTouchFT6206);
  fx_set_display(H, drawRectStub, drawTextStub);
  File f = SD.open(".config", FILE_READ);
  std::string prog;
  if(f){
    std::string text;
    while(f.available()){ text.push_back((char)f.read()); }
    f.close();
    std::string line; bool inData=false;
    for(size_t i=0;i<=text.size();++i){ char c = (i<text.size()? text[i] : '\n'); if(c=='\n'){ std::string t=trim(line); line.clear(); if(t.size()==0) continue; if(t=="[data]"){ inData=true; continue; } if(inData){ size_t eq=t.find('='); if(eq!=std::string::npos){ std::string key=trim(t.substr(0,eq)); std::string val=trim(t.substr(eq+1)); if(key=="loadProgram"){ prog=val; break; } } } } else { line.push_back(c); } }
  }
  fx_load(H, std::string("root/os.fx"));
  const char* err = fx_last_error(H);
  if(err && err[0]){ panicMode = true; addLog(std::string("load error: ")+err); renderError(); }
  fx_call_setup(H);
  err = fx_last_error(H);
  if(err && err[0]){ panicMode = true; addLog(std::string("setup error: ")+err); renderError(); }
}

void loop(){
  static unsigned long last = 0;
  unsigned long now = millis();
  float dt = (last==0)? 0.0f : (float)(now - last) / 1000.0f;
  last = now;
  if(panicMode){ int x=0,y=0; bool pressed=false; readTouchFT6206(&x,&y,&pressed); if(pressed){ if(y>260 && y<290 && x>70 && x<170){ fx_destroy(H); H = fx_create(readFileSd, printSerial); fx_set_touch(H, readTouchFT6206); fx_set_display(H, drawRectStub, drawTextStub); fx_load(H, std::string("os.fx")); fx_call_setup(H); fx_clear_error(H); panicMode=false; errLog.clear(); } } renderError(); return; }
  fx_call_loop(H, dt);
  const char* err2 = fx_last_error(H);
  if(err2 && err2[0]){ panicMode = true; addLog(std::string("loop error: ")+err2); renderError(); }
}
