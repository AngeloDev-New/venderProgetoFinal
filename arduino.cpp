#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DHT.h>
#define RX 0
#define TX 1
#define PREV 2
#define NEXT 3
#define LUZ 4
#define REGISTRO 5
#define DHTPIN 6
#define VENTO 7
//pinos display
#define P4 8
#define P6 9
#define P11 10
#define P12 11
#define P13 12
#define P14 13
#define UTERRA A0
#define BLT A1

#define DHTTYPE DHT11

#define CLCLICK 2000
#define ATTDISPLAY 5000
#define CONFIRMARCLICK 500
SoftwareSerial BT(RX, TX);
float * last_UT;
int last_US;
void pinModes(int mode,int * ios);
void SOLENOIDE(bool ATIVAR);
void LAMPADA(bool ATIVAR);
void VENTOINHA(bool ATIVAR);
int UMIDADE_SOLO();
char* ftoC(float numero,int casas = 0);
float* UMIDADE_AR_TEMPERATURA();
void BOTAO();
void testeVentoinha(int tempo);
void testeLampada(int tempo);
void testeSolenoide(int tempo);
LiquidCrystal lcd(P4,P6,P11,P12,P13,P14);
DHT dht(DHTPIN,DHTTYPE);
float temperaturaAlvo,umidadeAlvo; 
//false==menu_temperatura
bool menu_umidade;
float* UT;
int US;
void smart_data();
void setup() {
  
  UT = 0;
  US = 0;
  last_UT = 0;
  last_US = 0;
  escolhaMenuUmidade(true);
  Serial.begin(9600);
  lcd.begin(16,2);
  int outs[] = {LUZ, REGISTRO, VENTO,BLT};
  digitalWrite(LUZ,HIGH);
  digitalWrite(REGISTRO,HIGH);
  int ins[] = {PREV,NEXT,UTERRA};
  pinModes(OUTPUT,outs);
  pinModes(INPUT,ins);
  //////////////////////////////////////////
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Inicializando");
  testeVentoinha(5000);
  testeLampada(5000);
  testeSolenoide(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  BT.begin(9600);
  lcd.print("Bluetoon Ativado");
  Serial.println("Arduino pronto para comandos!");
  digitalWrite(BLT,HIGH);
  delay(2000);
  lcd.clear();

}

void loop() {
  //delay(500);
  smart_data();
  UT = UMIDADE_AR_TEMPERATURA();
  US = (UMIDADE_SOLO()/1024)*100;
  if(temperaturaAlvo<UT[1]){
    LAMPADA(false);
    VENTOINHA(true);
  }else{
    LAMPADA(true);
    VENTOINHA(false);
  }if(umidadeAlvo<UMIDADE_SOLO()){
    SOLENOIDE(false);
  }else{
    SOLENOIDE(true);
  }
  if(menu_umidade){
    if (last_US != US){
      lcd.setCursor(0,1);
      //print umidade do ar
      lcd.print(String(UT[0]));
      lcd.setCursor(4,1);
      lcd.print("/");
      lcd.setCursor(5,1);
      //print umidade do solo
      lcd.print("US");
      lcd.setCursor(9,1);
      lcd.print("/");
      lcd.setCursor(10,1);
      //print umidade alvo
      lcd.print("String(umidadeAlvo)");
      Serial.print(String(umidadeAlvo));
    }
  BOTAO();
  }else{
      delay(100);
      //lcd.setCursor(linha , coluna);
      //print temperatura atual
      lcd.print(String(UT[1]));
      lcd.setCursor(7,1);
      lcd.print("/");
      lcd.setCursor(9,1);
      //print temperatura alvo
      lcd.print(String(temperaturaAlvo));
      Serial.print(String(temperaturaAlvo));
    
  BOTAO();
  }
  last_UT = UT;
  last_US = US;
}
void smart_data(){
    if (BT.available()) {
    String comando = BT.readStringUntil('\n'); // Recebe o comando
    Serial.println("Comando recebido: " + comando);

    if (comando.startsWith("TEMP:")) {
      String tempStr = comando.substring(5);
      temperaturaAlvo = tempStr.toInt();
      // Serial.println("Temperatura ajustada para: " + String(temperatura));
    } else if (comando.startsWith("UMID:")) {
      String umidadeStr = comando.substring(5);
      umidadeAlvo = umidadeStr.toInt();
      // Serial.println("Umidade ajustada para: " + String(umidade));
    }
  }else{
    temperaturaAlvo = 25;
    umidadeAlvo = 40;
  }

}
void ativarOut(int pino,int tempo,bool reles = false){
  if(reles){
    digitalWrite(pino,LOW);
    delay(tempo);
    digitalWrite(pino,HIGH); 
  }else{
    digitalWrite(pino,HIGH);
    delay(tempo);
    digitalWrite(pino,LOW);
  }
}
void testeVentoinha(int tempo){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Teste Ventoinha");
  ativarOut(VENTO, tempo);
}
void testeLampada(int tempo){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Teste Lampada");
  ativarOut(LUZ, tempo,true);
}
void testeSolenoide(int tempo){
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("Teste Registro");
  ativarOut(REGISTRO, tempo,true);

}

void BOTAO(){
  int i = 0;
  bool prev;
  if(digitalRead(PREV)||digitalRead(NEXT)){
    for(i = 0;digitalRead(PREV);i++){
      prev = true;
      if(i==CLCLICK){break;}
    }
    for(i = 0;digitalRead(NEXT);i++){
      prev = false;
      if(i==CLCLICK){break;}
    }
    if(i<CLCLICK){
      //click curto
      if(prev){
        //botao atras
        if(menu_umidade){
          umidadeAlvo--; 
        }else{
          temperaturaAlvo--;
        }
      }else{
        //botao adiante
        if(menu_umidade){
          umidadeAlvo++;
        }else{
          temperaturaAlvo++;
        }  
      }
    }else{
  //click longo
      if(menu_umidade){
        escolhaMenuUmidade(true);
        
      }else{
        escolhaMenuUmidade(false);
      }
    }
      delay(CONFIRMARCLICK);
  }
}
void escolhaMenuUmidade(bool umidade){
        menu_umidade = umidade;
        lcd.clear();
        lcd.setCursor(0,0);
        if (umidade){lcd.print("Umidade AR/SL/AV");}
        else{lcd.print("Temp. ALVO/ATUAL");}
}
float* UMIDADE_AR_TEMPERATURA(){
  float Umidade = dht.readHumidity();
  float Temperatura = dht.readTemperature();
  if(isnan(Umidade)||isnan(Temperatura)){
    Serial.println("Falha na leitura");  
  }else{
    float * retorno = new float[2];
    retorno[0] = Umidade;
    retorno[1] = Temperatura;
    Serial.println(retorno[0]);
    Serial.println(retorno[1]);
    return retorno;
  } 
}

char* ftoC(float numero,int casas = 0){
  char num[2+casas];
  dtostrf(numero,2+casas,casas,num);
  return num;
}
int UMIDADE_SOLO(){
  return analogRead(UTERRA);
}
void VENTOINHA(bool ATIVAR){
  if (ATIVAR){
    digitalWrite(VENTO,HIGH);
  }else{
    digitalWrite(VENTO,LOW);
  }
}
//RELE INVERTE SINAL ENTAO ATIVADO==LOW
void LAMPADA(bool ATIVAR){
  if(ATIVAR){
    digitalWrite(LUZ,LOW);
  }else{
    digitalWrite(LUZ,HIGH);   
  }
}
void SOLENOIDE(bool ATIVAR){
  if (ATIVAR){
    digitalWrite(REGISTRO,LOW);
  }else{
    digitalWrite(REGISTRO,HIGH);
  }
}
void pinModes(int mode,int * ios){
  int i=0;
  while(ios[i]){
    pinMode(ios[i++],mode);
  }
}