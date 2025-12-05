#include <stdint.h>
#include "../../fx_core.h"
#include "framebuffer.h"

static FxInterpreterHandle* H = nullptr;

static void fb_draw_rect(int x,int y,int w,int h,unsigned short color,bool fill){
    uint32_t c = ((uint32_t)color) | 0xFF000000u;
    if(fill){
        fb_rect(x,y,w,h,c);
    } else {
        if(w<=0||h<=0) return;
        fb_rect(x,y,w,1,c);
        fb_rect(x,y+h-1,w,1,c);
        fb_rect(x,y,1,h,c);
        fb_rect(x+w-1,y,1,h,c);
    }
}

// 5x7 font for ASCII 32..127 (subset)
static const uint8_t font5x7[][7] = {
    // Space (32)
    {0,0,0,0,0,0,0},
};

static void draw_char5x7(int x,int y,char ch,unsigned short color,int size){
    uint32_t c = ((uint32_t)color) | 0xFF000000u;
    int idx = (int)ch - 32;
    const uint8_t* rows = nullptr;
    if(idx>=0 && idx<1){ rows = font5x7[idx]; }
    if(!rows){
        // simple fallback: box
        fb_rect(x,y,5*size,7*size,c);
        return;
    }
    for(int r=0;r<7;r++){
        uint8_t bits = rows[r];
        for(int col=0; col<5; col++){
            if(bits & (1<<(4-col))){ fb_rect(x+col*size, y+r*size, size, size, c); }
        }
    }
}

static void fb_draw_text(int x,int y,const std::string& text,unsigned short color,int size){
    if(size<1) size=1;
    int cur=x;
    for(char ch: text){
        if(ch=='\n'){ y += 8*size; cur = x; continue; }
        draw_char5x7(cur,y,ch,color,size);
        cur += (6*size);
    }
}

static bool touch_read_stub(int* x,int* y,bool* pressed){ *x=0; *y=0; *pressed=false; return true; }

static std::string read_file_baremetal(const std::string& path){
    if(path=="root/os.fx"){
        return std::string(
            "import Config:1.0.0 as config\n"
            "import IO:1.0.0 as io\n"
            "import Touch:1.0.0 as touch\n"
            "import Display:1.0.0 as display\n"
            "var int SCREEN_W = 1024;\n"
            "var int SCREEN_H = 600;\n"
            "var string programName = \"\";\n"
            "var string programPath = \"\";\n"
            "var bool startedOpeningSettings = false;\n"
            "var bool settingsOpened = false;\n"
            "var bool pressedPrev = false;\n"
            "var string screen = \"none\";\n"
            "var int btnOffX=0; var int btnOffY=0; var int btnBtX=0; var int btnBtY=0; var int btnWifiX=0; var int btnWifiY=0;\n"
            "var bool btOn=false; var bool wifiOn=false; var bool powerOff=false;\n"
            "func void log(string text){ io.print(\"[OS] \" + text); }\n"
            "func void tryOpenSettings(){ var int ev=touch.read(); var bool pressed=ev.pressed; var bool didPress=pressed!=pressedPrev && pressed; pressedPrev=pressed; if(pressed){ var int x=ev.x; var int y=ev.y; if(x<100 && (startedOpeningSettings==false)){ startedOpeningSettings=true; } if(x>100 && startedOpeningSettings){ startedOpeningSettings=false; settingsOpened=true; renderOverlay(\"settings\"); } } if(didPress){ var int x=ev.x; var int y=ev.y; if(x>100 && (settingsOpened || startedOpeningSettings)){ settingsOpened=false; startedOpeningSettings=false; renderOverlay(\"none\"); } } }\n"
            "func void renderOverlay(string screenOpen){ screen=screenOpen; if(screen==\"settings\"){ renderSettings(); } }\n"
            "func void drawButton(int x,int y,int w,int h,string label){ display.drawRect(x,y,w,h,65535,false); display.drawText(x+12,y+12,label,65535,1); }\n"
            "func void renderSettings(){ display.drawRect(0,0,SCREEN_W,SCREEN_H,0,true); display.drawRect(0,0,SCREEN_W,30,65535,false); display.drawText(10,8,\"Settings\",65535,1); display.drawText(SCREEN_W-80,8,\"Close\",65535,1); var int bw=200; var int bh=48; var int gap=16; var int mx=20; var int my=60; btnOffX=mx; btnOffY=my; btnBtX=mx; btnBtY=my+bh+gap; btnWifiX=mx; btnWifiY=btnBtY+bh+gap; drawButton(btnOffX,btnOffY,bw,bh,\"Off\"); drawButton(btnBtX,btnBtY,bw,bh,\"Bluetooth\"); drawButton(btnWifiX,btnWifiY,bw,bh,\"WiFi\"); display.drawText(btnBtX+bw+20,btnBtY+14, btOn,65535,1); display.drawText(btnWifiX+bw+20,btnWifiY+14, wifiOn,65535,1); }\n"
            "func void handleSettingsTouch(){ var int ev=touch.read(); var bool pressed=ev.pressed; var bool didPress=pressed!=pressedPrev && pressed; pressedPrev=pressed; if(didPress){ var int x=ev.x; var int y=ev.y; var int bw=200; var int bh=48; if((x>=btnOffX)&&(x<btnOffX+bw)&&(y>=btnOffY)&&(y<btnOffY+bh)){ io.print(\"[OS] Power off\"); powerOff=true; display.drawRect(0,0,SCREEN_W,SCREEN_H,0,true); display.drawText(SCREEN_W/2-60,SCREEN_H/2,\"Powering off...\",65535,1); } else if((x>=btnBtX)&&(x<btnBtX+bw)&&(y>=btnBtY)&&(y<btnBtY+bh)){ btOn=!btOn; io.print(\"[OS] Bluetooth toggled\"); renderSettings(); } else if((x>=btnWifiX)&&(x<btnWifiX+bw)&&(y>=btnWifiY)&&(y<btnWifiY+bh)){ wifiOn=!wifiOn; io.print(\"[OS] WiFi toggled\"); renderSettings(); } else { if(x>SCREEN_W-100 && y<30){ settingsOpened=false; startedOpeningSettings=false; renderOverlay(\"none\"); } else if(x>100){ settingsOpened=false; startedOpeningSettings=false; renderOverlay(\"none\"); } } } }\n"
            "func int setup(){ log(\"BOOTING\"); log(\"Loading .config\"); var int cfg=config.load(\".config\"); log(\"Loaded .config\"); programName=cfg.data.loadProgram; programPath=\"programs/\" + programName + \"/main.fx\"; log(\"Running program \" + programPath + \" \" + \"(file: \" + programPath + \")\"); import programPath as loadProgram; log(\"Program loaded to memory\"); loadProgram.setup(); return 0; }\n"
            "func void loop(float dt){ tryOpenSettings(); if(screen==\"none\"){ if(!powerOff){ try { loadProgram.loop(dt); } except { } } } else if(screen==\"settings\"){ handleSettingsTouch(); } }\n"
        );
    }
    if(path=="programs/bookWorm/main.fx"){
        return std::string(
            "import IO:1.0.0 as io\n"
            "import Display:1.0.0 as display\n"
            "import Touch:1.0.0 as touch\n"
            "import String:1.0.0 as str\n"
            "import List:1.0.0 as list\n"
            "var int SCREEN_W=1024; var int SCREEN_H=600; var int ITEM_H=24; var int STATE_MENU=0; var int STATE_PAGE=1; var int state=0; var int selectedIndex=0; var string selectedBook=\"\"; var int pageIndex=0; var int linesPerPage=28; var int maxPageIndex=0; var bool pressedPrev=false; var int books=[]; var int pages=[];\n"
            "func void renderMenu(){ display.drawRect(0,0,SCREEN_W,SCREEN_H,0,true); var int i=0; for(i=0;i<list.len(books);i){ var string name=books[i]; var int y=20 + i*ITEM_H; display.drawRect(5,y-4,SCREEN_W-10,ITEM_H,65535,false); display.drawText(12,y,name,65535,1); } }\n"
            "func void buildPages(string text){ var int lines=str.splitLines(text); pages=lines; maxPageIndex=list.len(pages) / linesPerPage; }\n"
            "func void renderPage(){ display.drawRect(0,0,SCREEN_W,SCREEN_H,0,true); display.drawRect(0,0,200,30,65535,false); display.drawText(5,5,pageIndex,65535,1); display.drawText(30,5,\"| Previous |\",65535,1); display.drawText(110,5,\"Home |\",65535,1); display.drawText(160,5,\"Next\",65535,1); var int start=pageIndex*linesPerPage; var int end=start+linesPerPage; var int i=start; for(i=start; (i<end) && (i<list.len(pages)); i){ var string ln=pages[i]; display.drawText(15,40 + (i-start)*20,ln,65535,1); } }\n"
            "func int setup(){ var string entries=io.readdir(\"data/bookWorm\"); books=str.splitLines(entries); state=STATE_MENU; renderMenu(); return 0; }\n"
            "func void loop(float dt){ var int ev=touch.read(); var bool pressed=ev.pressed; var bool didPress=pressed!=pressedPrev && pressed; pressedPrev=pressed; if(didPress){ var int x=ev.x; var int y=ev.y; if(state==STATE_MENU){ var int row=(y-16)/ITEM_H; if((row>=0)&&(row<list.len(books))){ selectedIndex=row; selectedBook=books[row]; var string path=\"data/bookWorm/\" + selectedBook; var string content=io.readFile(path); buildPages(content); pageIndex=0; state=STATE_PAGE; renderPage(); } } else { if(y<30){ if(x<110){ if(pageIndex>0){ pageIndex=pageIndex-1; } renderPage(); } else if(x<160){ state=STATE_MENU; renderMenu(); } else if(x<200){ if(pageIndex<maxPageIndex){ pageIndex=pageIndex+1; renderPage(); } } } } } }\n"
        );
    }
    if(path==".config"){
        return std::string("[data]\nloadProgram=bookWorm\n");
    }
    return std::string();
}

extern "C" void fx_kernel_main(){
    if(!fb_init(1024,600,32)){
        while(1){}
    }
    fb_clear(0x00FFFFFF);
    H = fx_create(read_file_baremetal, nullptr);
    fx_set_display(H, fb_draw_rect, fb_draw_text);
    fx_set_touch(H, touch_read_stub);
    fx_load(H, std::string("root/os.fx"));
    fx_call_setup(H);
    while(1){ fx_call_loop(H, 0.016f); }
}
