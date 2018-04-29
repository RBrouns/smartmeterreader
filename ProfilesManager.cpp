#include "application.h"
#include "ProfilesManager.h"

#define EEPROM_BLOCK_SIZE       10

struct Appliance {
  char name[8];
  int16_t usesWh;
};

ProfilesManager::ProfilesManager(void){
     
}

void ProfilesManager::begin(){
    updateNextFreeAddress();
}    

void ProfilesManager::createProfile(char * name, int16_t usesWh){
    if(nextFreeAddress != -1){
        Appliance newApp;
        strcpy(newApp.name, name);
        newApp.usesWh = usesWh;
        EEPROM.put(0, newApp);
        updateNextFreeAddress();
        
        Particle.publish("PROFILES", String::format("Created new appliance profile: {name:unknown, usesWh:%d} of size:%d", usesWh, sizeof(newApp)));
    }
}

String ProfilesManager::findMatch(int searchUse){
    if(nextFreeAddress == 0){
         return FIND_NODATA;
    }
    
    for(int i=0;i<nextFreeAddress;i+=EEPROM_BLOCK_SIZE){
        Appliance app;
        EEPROM.get(0, app);
        if(abs(app.usesWh - searchUse) < 100){
            return app.name;
        }
    }
    
    return FIND_NOMATCH;
}

void ProfilesManager::updateNextFreeAddress(){
    for(int i=0;i<EEPROM.length();i++){
        if(EEPROM.read(i) == 0xFF){
            nextFreeAddress = i;
            return;
        }
    }
    //If all addresses are occupied
    nextFreeAddress = -1;
}

String ProfilesManager::getStoredProfiles(){
    String fullString;
    
    for(int i=0;i<nextFreeAddress;i+=EEPROM_BLOCK_SIZE){
        Appliance app;
        EEPROM.get(0, app);
        fullString.concat(app.name);
        fullString += ",";
    }
    
    fullString = String::format("Stored profiles: %d , names:" + fullString, (int)round(nextFreeAddress / EEPROM_BLOCK_SIZE));
    return fullString;
}

void ProfilesManager::clearAllProfiles(){
    EEPROM.clear();
}

void ProfilesManager::updateAppName(int zeroBasedindex, char * name){
    int add = zeroBasedindex*EEPROM_BLOCK_SIZE;
    Appliance app;
    EEPROM.get(add, app);
    
    //Update the name and store at same place
    strcpy(app.name, name);
    EEPROM.put(add, app);
}