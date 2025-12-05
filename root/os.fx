import Config:1.0.0 as config
import IO:1.0.0 as io
import Touch:1.0.0 as touch
import Display:1.0.0 as display

var int SCREEN_W = 1024;
var int SCREEN_H = 600;

var string programName = "";
var string programPath = "";

var bool startedOpeningSettings = false;
var bool settingsOpened = false;
var bool pressedPrev = false;

var string screen = "none";

var int btnOffX = 0;
var int btnOffY = 0;
var int btnBtX = 0;
var int btnBtY = 0;
var int btnWifiX = 0;
var int btnWifiY = 0;

var bool btOn = false;
var bool wifiOn = false;
var bool powerOff = false;

func void log(string text){
    io.print("[OS] " + text);
}

func void tryOpenSettings(){
    var int ev = touch.read();
    var bool pressed = ev.pressed;

    var bool didPress = pressed != pressedPrev && pressed;
    pressedPrev = pressed;



    if(pressed){
        var int x = ev.x;
        var int y = ev.y;
        if(x < 100 && (startedOpeningSettings == false)){
            startedOpeningSettings = true;
        }
        if(x > 100 && startedOpeningSettings){
            startedOpeningSettings = false;
            settingsOpened = true;
            renderOverlay("settings");
        }
    }

    if (didPress){
        var int x = ev.x;
        var int y = ev.y;
        if(x > 100 && (settingsOpened || startedOpeningSettings)){
            settingsOpened = false;
            startedOpeningSettings = false;
            renderOverlay("none");
        }
    }
}

func void renderOverlay(string screenOpen){
    screen = screenOpen;

    if(screen == "settings"){
        renderSettings();
    }
}

func void drawButton(int x,int y,int w,int h,string label){
    display.drawRect(x,y,w,h,65535,false);
    display.drawText(x+12,y+12,label,65535,1);
}

func void renderSettings(){
    display.drawRect(0,0,SCREEN_W,SCREEN_H,0,true);
    display.drawRect(0,0,SCREEN_W,30,65535,false);
    display.drawText(10,8,"Settings",65535,1);
    display.drawText(SCREEN_W-80,8,"Close",65535,1);

    var int bw = 200;
    var int bh = 48;
    var int gap = 16;
    var int mx = 20;
    var int my = 60;

    btnOffX = mx; btnOffY = my;
    btnBtX = mx; btnBtY = my + bh + gap;
    btnWifiX = mx; btnWifiY = btnBtY + bh + gap;

    drawButton(btnOffX,btnOffY,bw,bh,"Off");
    drawButton(btnBtX,btnBtY,bw,bh,"Bluetooth");
    drawButton(btnWifiX,btnWifiY,bw,bh,"WiFi");
    display.drawText(btnBtX+bw+20,btnBtY+14, btOn,65535,1);
    display.drawText(btnWifiX+bw+20,btnWifiY+14, wifiOn,65535,1);
}

func void handleSettingsTouch(){
    var int ev = touch.read();
    var bool pressed = ev.pressed;
    var bool didPress = pressed != pressedPrev && pressed;
    pressedPrev = pressed;

    if(didPress){
        var int x = ev.x;
        var int y = ev.y;
        var int bw = 200;
        var int bh = 48;
        if((x >= btnOffX) && (x < btnOffX + bw) && (y >= btnOffY) && (y < btnOffY + bh)){
            io.print("[OS] Power off");
            powerOff = true;
            display.drawRect(0,0,SCREEN_W,SCREEN_H,0,true);
            display.drawText(SCREEN_W/2 - 60, SCREEN_H/2, "Powering off...", 65535, 1);
        } else if ((x >= btnBtX) && (x < btnBtX + bw) && (y >= btnBtY) && (y < btnBtY + bh)){
            btOn = !btOn;
            io.print("[OS] Bluetooth toggled");
            renderSettings();
        } else if ((x >= btnWifiX) && (x < btnWifiX + bw) && (y >= btnWifiY) && (y < btnWifiY + bh)){
            wifiOn = !wifiOn;
            io.print("[OS] WiFi toggled");
            renderSettings();
        } else {
            if(x > SCREEN_W-100 && y < 30){
                settingsOpened = false;
                startedOpeningSettings = false;
                renderOverlay("none");
            } else if(x > 100){
                settingsOpened = false;
                startedOpeningSettings = false;
                renderOverlay("none");
            }
        }
    }
}

func int setup(){
    log("BOOTING");
    log("Loading .config");
    var int cfg = config.load(".config");
    log("Loaded .config");
    programName = cfg.data.loadProgram;
    programPath = "programs/" + programName + "/main.fx";
    log("Running program \"" + programPath + "\" " + "(file: \"" + programPath + "\")");
    import programPath as loadProgram
    log("Program loaded to memory");
    loadProgram.setup();
    return 0;
}

func void loop(float dt){
    tryOpenSettings();

    if (screen == "none"){
        if(!powerOff){
            try { loadProgram.loop(dt); } except { }
        }
    } else if (screen == "settings"){
        handleSettingsTouch();
    }
    
}
