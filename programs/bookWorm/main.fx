import IO:1.0.0 as io
import Display:1.0.0 as display
import Touch:1.0.0 as touch
import String:1.0.0 as str
import List:1.0.0 as list

var int SCREEN_W = 1024;
var int SCREEN_H = 600;
var int ITEM_H = 24;
var int STATE_MENU = 0;
var int STATE_PAGE = 1;

var int state = 0;
var int selectedIndex = 0;
var string selectedBook = "";
var int pageIndex = 0;
var int linesPerPage = 28;

var int maxPageIndex = 0;

var bool pressedPrev = false;

var int books = [];
var int pages = [];

func void renderMenu(){
    display.drawRect(0,0,SCREEN_W,SCREEN_H,0,true);
    var int i = 0;
    for (i = 0; i < list.len(books); i){
        var string name = books[i];
        var int y = 20 + i*ITEM_H;
        display.drawRect(5,y-4,SCREEN_W-10,ITEM_H,65535,false);
        display.drawText(12,y,name,65535,1);
    }
}

func void buildPages(string text){
    var int lines = str.splitLines(text);
    pages = lines;

    maxPageIndex = list.len(pages) / linesPerPage;
}

func void renderPage(){
    display.drawRect(0,0,SCREEN_W,SCREEN_H,0,true);
    display.drawRect(0,0,200,30,65535,false);

    display.drawText(5,5, pageIndex,65535,1);
    display.drawText(30,5,"| Previous |",65535,1);
    display.drawText(110 ,5,"Home |",65535,1);
    display.drawText(160 ,5,"Next",65535,1);
    var int start = pageIndex*linesPerPage;
    var int end = start + linesPerPage;
    var int i = start;
    for (i = start; (i < end) && (i < list.len(pages)); i){
        var string ln = pages[i];
        display.drawText(15,40 + (i-start)*20,ln,65535,1);
    }
}

func int setup(){
    var string entries = io.readdir("data/bookWorm");
    books = str.splitLines(entries);
    state = STATE_MENU;
    renderMenu();
    return 0;
}

func void loop(float dt){
    var int ev = touch.read();
    var bool pressed = ev.pressed;

    var bool didPress = pressed != pressedPrev && pressed;
    pressedPrev = pressed;

    if(didPress){
        var int x = ev.x;
        var int y = ev.y;
        if(state == STATE_MENU){
            var int row = (y - 16) / ITEM_H;
            if((row >= 0) && (row < list.len(books))){
                selectedIndex = row;
                selectedBook = books[row];
                var string path = "data/bookWorm/" + selectedBook;
                var string content = io.readFile(path);
                buildPages(content);
                pageIndex = 0;
                state = STATE_PAGE;
                renderPage();
            }
        } else {
            if(y < 30){
                if(x < 110){
                    if(pageIndex > 0){ pageIndex = pageIndex - 1; }
                    renderPage();
                } else if (x < 160){
                    state = STATE_MENU;
                    renderMenu();
                } else if (x < 200) {
                    if(pageIndex < maxPageIndex){
                        pageIndex = pageIndex + 1;

                        renderPage();
                    }
                }
            }
        }
    }
}

