import Config:1.0.0 as config
import IO:1.0.0 as io

var string programName = "";
var string programPath = "";

func void log(string text){
    io.print("[OS] " + text);
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
    loadProgram.loop(dt);
}

