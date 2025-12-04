import List:1.0.0 as list
import Time:1.0.0 as time
import Arduino:1.0.0 as arduino

-- 5s -> on/off
-- 1 click -> next action
-- 2 clicks -> execute action



var int BUTTON_PIN = 4;

var float PRESS_THREASHHOLD = 10.0;

var string actions = [];
var int actionIndex = 0;

var bool lastValue = false;
var float interactionStart = 0.0;
var float interactionEnd = 0.0;


func void addAction(string action){
    list.append(actions, action);
}

func void nextAction(){
    actionIndex = (actionIndex + 1) % list.len(actions);
}

func string executeAction(){
    if (list.len(action) <= 0) {
        return "";
    }
    
    return actions[actionIndex];
}


func void init(){
    arduino.pinMode(BUTTON_PIN, arduino.INPUT);
}


func string loop(float dt){ -- returns action to execute empty if none
    var bool currentValue = arduino.digitalRead(BUTTON_PIN) == arduino.HIGH;


}