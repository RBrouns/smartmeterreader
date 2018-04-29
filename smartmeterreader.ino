// This #include statement was automatically added by the Particle IDE.
#include <blynk.h>

#include <blynk.h>

#include "ProfilesManager.h"

/*Targeted smart meter: Kaifa
RJ11 config:
Pin 2: Request to send --> D1
Pin 3: Ground ---> GND
Pin 5: Transmitted data --> RX

Data:
Baudrate = 115200
Data bits = 8
Stop bits = 1
Parity = None
*/

//Reference: http://domoticx.com/p1-poort-slimme-meter-hardware/

#include <blynk.h>
#include <map>
#include <vector>

char auth[] = "560ce4e29a5344e1869b841de8c9de5a";

int currentUse;
String lastTelegram;
ProfilesManager profilesManager;

void setup() {
    Particle.function("addAppliance", onAddAppliance);
    Particle.function("getEeprom", onGetEeprom);
    Particle.function("clearEeprom", onClearEeprom);
    Particle.function("setAppName", onSetApplianceName);
    
    profilesManager.begin();
    Blynk.begin(auth);
    
    Serial1.begin(115200, SERIAL_8N1);

    Particle.publish("BOOT", "Boot completed");
    
    Blynk.tweet(String::format("Test, my tweet %d,%d", Time.hour(), Time.minute()));
    Blynk.run();
}

void loop() {
    if(lastTelegram.indexOf("!") != -1){
        //Message is complete
        int newCurrentUse = getAKwValue(":1.7.0(");
        //int newCurrentUse = (random(1,10) > 8? 2000 : 205);
        int currentSolar = getAKwValue(":2.7.0(");
        int totalUseDay = getAKwValue(":1.8.2(");
        int totalUseNight = getAKwValue(":1.8.1(");
        Particle.publish("VALUES",  String::format("Currently using: %d", newCurrentUse));
        Blynk.virtualWrite(V0, newCurrentUse);
        Blynk.virtualWrite(V1, totalUseDay);
        Blynk.virtualWrite(V2, totalUseNight);
        lastTelegram = "";
        
        if(currentUse != 0){
            evalNewCurrentUse(newCurrentUse);
        }
        currentUse = newCurrentUse;
    }
    
    Blynk.run();
}

//SerialEvent1 is invoked by Particle's OS when data is available
void serialEvent1(){
    while(Serial1.available() > 0){
        lastTelegram.concat((char)Serial1.read());
    }
}

int getAKwValue(String searchString){
   int startIndex = lastTelegram.indexOf(searchString);
    if(startIndex == -1){
        return -1;
    }
    
    //Take substring in full telegram --> 000.312    
    startIndex+=searchString.length();
    String substring = lastTelegram.substring(startIndex, lastTelegram.indexOf("*kW",startIndex));
    
    //Remove all periods, thereby preventing that 1.312 is read as 1, instead of 1312
    while(substring.indexOf(".") != -1){
        substring.remove(substring.indexOf("."),1);
    }
    
    //Remove leading data: e.g. 000312 --> 312. But make sure that 00000 is not fully emptied
    while(substring.charAt(0) == '0' && substring.length() != 1){
        substring.remove(0,1);
    }

    return substring.toInt();
}

void evalNewCurrentUse(int newUse){
    
    int16_t delta = abs(newUse - currentUse);
    
    //Check for significant change
    if(delta > 100){
        String result = profilesManager.findMatch(delta);
        if(result == FIND_NODATA || result == FIND_NOMATCH){
            char newName[8] = "unknown";
            profilesManager.createProfile(newName, delta);
        }else{
            Particle.publish("App Lookup","Got match with:" + result);
        }
        
        Blynk.tweet(String::format("At %d:%d , the " + result + " turned on, started using %d Watts/h", Time.hour(), Time.minute(), delta));
    }
}

int onAddAppliance(String s){
    int splitIndex = s.indexOf(",");
    if(splitIndex != -1){
        char name[8];
        s.substring(0, splitIndex).toCharArray(name,8);
        int usesWh = s.substring(splitIndex).toInt();
        profilesManager.createProfile(name,usesWh);
        return 1;
    }
    return 0;
}

int onGetEeprom(String s){
    String msg = profilesManager.getStoredProfiles();
    Particle.publish("Profiles", msg);
    return 1;
}

int onClearEeprom(String s){
    ProfilesManager::clearAllProfiles();
    return 1;
}

int onSetApplianceName(String s){
    int splitIndex = s.indexOf(",");
    if(splitIndex != -1){
        int nrAppliance = s.substring(0, splitIndex).toInt();
        char newName[8];
        s.substring(splitIndex+1).toCharArray(newName,8);

        profilesManager.updateAppName(nrAppliance,newName);
        return 1;
    }
}