#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include <DHT.h>
#include <DHT_U.h>

#include <CayenneLPP.h>

#define DHTPIN 23
#define DHTYPE DHT22

DHT dht(DHTPIN,DHTYPE);
CayenneLPP lpp(51);

uint8_t NWSK[16] = { 0x61, 0x34, 0x35, 0x76, 0x10, 0x74, 0xC8, 0xCA, 0x8A, 0xE5, 0x4F, 0x51, 0x14, 0x73, 0x11, 0x29 };
uint8_t APPSKEY[16] = { 0x89, 0x50, 0x4E, 0xD1, 0xD7, 0xC0, 0xE0, 0x13, 0xB2, 0x9D, 0x78, 0x97, 0x82, 0xB7, 0x15, 0x17 };
uint32_t DEVADDR = 0x26031C3E;

void os_getArtEui(u1_t* buf){}
void os_getDevEui(u1_t* buf){}
void os_getDevKey(u1_t* buf){}

const unsigned TX_INTERVAL = 10;
unsigned long previousMillis = 0;

const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = { 26, 33, 32 }  
};

void onEvent(ev_t ev){
  switch(ev){
    case EV_TXCOMPLETE:
    Serial.println("[LMIC] Radio TX complete");
    digitalWrite(LED_BUILTIN, LOW);
    break;  
    default:
      Serial.println("Evento desconocido");
    break;
  } 
}
void setup() {
  Serial.begin(115200);
  Serial.println("[INFO] Iniciando");
  pinMode(LED_BUILTIN,OUTPUT);
  dht.begin();
  os_init();
  LMIC_reset();
  LMIC_setSession(0x1,DEVADDR,NWSK,APPSKEY);
  for(int chan=0;chan<72;++chan){
    LMIC_disableChannel(chan);
  }  
  //904.3 Mhz banda de 915 Mhz 902-928 Mhz
  LMIC_enableChannel(10);
  LMIC_setLinkCheckMode(0);
  LMIC_setDrTxpow(DR_SF7,20);
  previousMillis = millis();
}

void loop() {
  if(millis()>previousMillis+TX_INTERVAL*1000){
    getInfoAndSend();
    previousMillis=millis();
  }
  os_runloop_once();  
}

void enviar_datos(uint8_t *mydata, uint16_t len){
  if(LMIC.opmode & OP_TXRXPEND){
    Serial.println("[LMIC] OP_TXRXPEND, not sending");
  }else{
    LMIC_setTxData2(1,mydata,len,0);
  }
}

void getInfoAndSend(){
  digitalWrite(LED_BUILTIN,HIGH);
  //Leer sensores y transmitir 
  float temp = dht.readTemperature(); 
  lpp.addTemperature(1,temp);
  enviar_datos(lpp.getBuffer(),lpp.getSize());
}
