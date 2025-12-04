#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include "fx_core.h"

static FxInterpreterHandle* H = nullptr;

static const int FX_W = 1024;
static const int FX_H = 600;
static const int SCALE = 1;

static HWND g_hwnd = nullptr;
static HDC g_backDC = nullptr;
static HBITMAP g_backBmp = nullptr;
static HBITMAP g_oldBmp = nullptr;
static int g_mouseX = 0;
static int g_mouseY = 0;
static bool g_mousePressed = false;
static bool g_panic = false;
static std::vector<std::string> g_log;
static std::string g_mainPath;
static LARGE_INTEGER g_freq;
static LARGE_INTEGER g_prev;

static COLORREF rgb565(unsigned short c){ int r=(c>>11)&31; int g=(c>>5)&63; int b=c&31; r = (r*255)/31; g = (g*255)/63; b = (b*255)/31; return RGB(r,g,b); }

static std::string read_file_desktop(const std::string& path){ std::ifstream in(path, std::ios::binary); if(!in.good()) return std::string(); std::ostringstream ss; ss << in.rdbuf(); return ss.str(); }
static void print_cmd(const std::string& s){ std::cout << s; }
static bool touch_read_mouse(int* x, int* y, bool* pressed){ *x = g_mouseX; *y = g_mouseY; *pressed = g_mousePressed; return true; }

static void clear_backbuffer(unsigned short color){ RECT rc{0,0,FX_W,FX_H}; HBRUSH br = CreateSolidBrush(rgb565(color)); FillRect(g_backDC, &rc, br); DeleteObject(br); }
static void drawRect_gdi(int x,int y,int w,int h,unsigned short color,bool fill){ if(!g_backDC) return; if(x==0 && y==0 && fill) clear_backbuffer(color); HPEN pen = CreatePen(PS_SOLID, 1, rgb565(color)); HBRUSH br = fill? CreateSolidBrush(rgb565(color)) : (HBRUSH)GetStockObject(HOLLOW_BRUSH); HPEN oldPen = (HPEN)SelectObject(g_backDC, pen); HBRUSH oldBr = (HBRUSH)SelectObject(g_backDC, br); Rectangle(g_backDC, x, y, x+w, y+h); SelectObject(g_backDC, oldPen); SelectObject(g_backDC, oldBr); DeleteObject(pen); if(fill) DeleteObject(br); InvalidateRect(g_hwnd, NULL, FALSE); }
static void drawText_gdi(int x,int y,const std::string& text,unsigned short color,int size){ if(!g_backDC) return; SetBkMode(g_backDC, TRANSPARENT); SetTextColor(g_backDC, rgb565(color)); TextOutA(g_backDC, x, y, text.c_str(), (int)text.size()); InvalidateRect(g_hwnd, NULL, FALSE); }

static std::string trim(const std::string& s){ size_t a=0; while(a<s.size() && (s[a]==' '||s[a]=='\t'||s[a]=='\r'||s[a]=='\n')) a++; size_t b=s.size(); while(b>a && (s[b-1]==' '||s[b-1]=='\t'||s[b-1]=='\r'||s[b-1]=='\n')) b--; return s.substr(a,b-a); }
static std::string read_config_program(){ std::ifstream in(".config"); if(!in.good()) return std::string(); std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()); std::string line; bool inData=false; std::string prog; for(size_t i=0;i<=text.size();++i){ char c = (i<text.size()? text[i] : '\n'); if(c=='\n'){ std::string t=trim(line); line.clear(); if(t.size()==0) continue; if(t=="[data]"){ inData=true; continue; } if(inData){ size_t eq=t.find('='); if(eq!=std::string::npos){ std::string key=trim(t.substr(0,eq)); std::string val=trim(t.substr(eq+1)); if(key=="loadProgram"){ prog=val; break; } } } } else { line.push_back(c); } } return prog; }

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){ switch(msg){ case WM_PAINT: { PAINTSTRUCT ps; HDC hdc = BeginPaint(hWnd, &ps); int W = FX_W*SCALE, H = FX_H*SCALE; StretchBlt(hdc, 0, 0, W, H, g_backDC, 0, 0, FX_W, FX_H, SRCCOPY); EndPaint(hWnd, &ps); return 0; } case WM_LBUTTONDOWN: g_mousePressed=true; case WM_MOUSEMOVE: { int x = LOWORD(lParam); int y = HIWORD(lParam); g_mouseX = x / SCALE; g_mouseY = y / SCALE; return 0; } case WM_LBUTTONUP: g_mousePressed=false; return 0; case WM_DESTROY: PostQuitMessage(0); return 0; default: return DefWindowProc(hWnd, msg, wParam, lParam); } }

static HWND create_window(){ WNDCLASSA wc{}; wc.lpfnWndProc = WndProc; wc.hInstance = GetModuleHandleA(NULL); wc.lpszClassName = "FxEmuWnd"; wc.hCursor = LoadCursor(NULL, IDC_ARROW); RegisterClassA(&wc); int W = FX_W*SCALE, H = FX_H*SCALE; HWND h = CreateWindowExA(0, wc.lpszClassName, "FX Emulator", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, W+16, H+39, NULL, NULL, wc.hInstance, NULL); ShowWindow(h, SW_SHOW); UpdateWindow(h); return h; }
static void init_backbuffer(){ HDC wndDC = GetDC(g_hwnd); g_backDC = CreateCompatibleDC(wndDC); g_backBmp = CreateCompatibleBitmap(wndDC, FX_W, FX_H); g_oldBmp = (HBITMAP)SelectObject(g_backDC, g_backBmp); ReleaseDC(g_hwnd, wndDC); clear_backbuffer(0); }

static void render_panic(){ clear_backbuffer(0); drawRect_gdi(0,0,FX_W,20,65535,false); drawText_gdi(6,4,"ERROR",65535,1); int y=26; int maxLines=12; int start = (int)std::max(0, (int)g_log.size()-maxLines); for(int i=start;i<(int)g_log.size();++i){ drawText_gdi(6,y,g_log[i],65535,1); y+=14; }
    int bw=100,bh=24; int bx=FX_W/2 - bw/2; int by=FX_H - bh - 8; drawRect_gdi(bx,by,bw,bh,65535,false); drawText_gdi(bx+12,by+6,"Try reboot",65535,1);
}

static void init_timer(){ QueryPerformanceFrequency(&g_freq); QueryPerformanceCounter(&g_prev); }
static float tick_dt(){ LARGE_INTEGER now; QueryPerformanceCounter(&now); float dt = (float)((double)(now.QuadPart - g_prev.QuadPart) / (double)g_freq.QuadPart); g_prev = now; return dt; }

int main(){ g_hwnd = create_window(); init_backbuffer(); H = fx_create(read_file_desktop, print_cmd); fx_set_display(H, drawRect_gdi, drawText_gdi); fx_set_touch(H, touch_read_mouse); g_mainPath = std::string("root/os.fx"); fx_load(H, g_mainPath); const char* err = fx_last_error(H); if(err && err[0]){ g_log.push_back(err); g_panic=true; }
    fx_call_setup(H); err = fx_last_error(H); if(err && err[0]){ g_log.push_back(err); g_panic=true; }
    init_timer();
    MSG m; while(true){ while(PeekMessage(&m, NULL, 0, 0, PM_REMOVE)){ TranslateMessage(&m); DispatchMessage(&m); } float dt = tick_dt(); if(!g_panic){ fx_call_loop(H, dt); }
        const char* err2 = fx_last_error(H); if(err2 && err2[0]){ g_log.push_back(err2); g_panic=true; fx_clear_error(H); }
        if(g_panic){ render_panic(); if(g_mousePressed){ int bw=100,bh=24; int bx=FX_W/2 - bw/2; int by=FX_H - bh - 8; int x=g_mouseX, y=g_mouseY; if(x>=bx && x<bx+bw && y>=by && y<by+bh){ fx_load(H, g_mainPath); fx_call_setup(H); g_log.clear(); g_panic=false; } } }
        Sleep(16); }
    fx_destroy(H); return 0; }
