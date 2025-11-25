import IO:1.0.0 as io
import Math:1.0.0 as math

var string TEXT = "example";

func int setup(){
    io.print("Program initiated");

    io.print(math.add(2, 3));

    var int i = 0;

    for (i; i < 10; i){
        io.print(TEXT);
    }
    return 0;
}

func int loop(float dt){

    return 0;
}
