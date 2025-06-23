#include <WiFi.h> 
#include <WebServer.h> 
#include <Preferences.h>
#include <GxEPD2_BW.h> 
#include <U8g2_for_Adafruit_GFX.h> 
#include <LittleFS.h>
#include "WifiLocation.h"


U8G2_FOR_ADAFRUIT_GFX fontes; 
GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16); 
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);


Preferences preferencias;
WebServer servidor(80); 


void pagina_ajustes_GET() {
  // Lê valores do Preferences
  int velocidadeMax = preferencias.getInt("speedMax", 60);
  int aceleracaoMax = preferencias.getInt("accelMax", 10);

  // Abre o arquivo ajustes.html do LittleFS
  File file = LittleFS.open("/ajustes.html", "r");
  if (!file) {
    servidor.send(500, "text/plain", "Erro ao carregar ajustes.html");
    return;
  }

  // Lê o conteúdo do arquivo
  String pagina = file.readString();
  file.close();

  // Substitui os marcadores pelos valores reais
  pagina.replace("{{speedMax}}", String(velocidadeMax));
  pagina.replace("{{accelMax}}", String(aceleracaoMax));

  // Envia a resposta
  servidor.send(200, "text/html", pagina);
}

void pagina_ajustes_POST() {
  if (servidor.hasArg("speed") && servidor.hasArg("acceleration")) {
    int velocidade = servidor.arg("speed").toInt();
    int aceleracao = servidor.arg("acceleration").toInt();

    // Atualiza Preferences
    preferencias.putInt("speedMax", velocidade);
    preferencias.putInt("accelMax", aceleracao);

    // Confirmação
    String resposta = "<html><body><h2>Preferências atualizadas!</h2>";
    resposta += "<p>Velocidade Máxima: " + String(velocidade) + " km/h</p>";
    resposta += "<p>Aceleração Máxima: " + String(aceleracao) + " m/s²</p>";
    resposta += "<a href='/ajustes'>Voltar</a></body></html>";

    servidor.send(200, "text/html", resposta);
  } else {
    servidor.send(400, "text/plain", "Parâmetros ausentes.");
  }
}


void mostraTela() {
    tela.fillScreen(GxEPD_WHITE);

    fontes.setFont( u8g2_font_helvB12_te ); 
    fontes.setFontMode(1); 
    fontes.setCursor(10, 20); 
    fontes.print("Acelerômetro");

    fontes.setFont( u8g2_font_open_iconic_all_2x_t ); 
    fontes.setFontMode(1); 
    fontes.setCursor(10, 43); 
    fontes.print(141);

    fontes.setFont( u8g2_font_helvB12_te ); 
    fontes.setFontMode(1); 
    fontes.setCursor(30, 40); 
    fontes.print("60 km/h");

    tela.drawLine(0,50,150,50,GxEPD_BLACK);

    fontes.setFont( u8g2_font_helvB12_te ); 
    fontes.setFontMode(1); 
    fontes.setCursor(10, 70); 
    fontes.print("GPS");

    fontes.setFont( u8g2_font_open_iconic_all_2x_t ); 
    fontes.setFontMode(1); 
    fontes.setCursor(10, 73); 
    fontes.print(175);

    fontes.setFont( u8g2_font_helvR12_te ); 
    fontes.setFontMode(1); 
    fontes.setCursor(5, 90); 
    fontes.print("Lat: -39.123456");
    fontes.setCursor(5, 110); 
    fontes.print("Lon: -54.123456");

    tela.drawLine(0,120,150,120,GxEPD_BLACK);

    fontes.setFont( u8g2_font_helvB12_te ); 
    fontes.setFontMode(1); 
    fontes.setCursor(10, 140); 
    fontes.print("WiFi");

    fontes.setFont( u8g2_font_open_iconic_all_2x_t ); 
    fontes.setFontMode(1); 
    fontes.setCursor(10, 143); 
    fontes.print(247);

    fontes.setFont( u8g2_font_helvR12_te ); 
    fontes.setFontMode(1); 
    fontes.setCursor(5, 160); 
    fontes.print("Lat: -39.123456");
    fontes.setCursor(5, 180); 
    fontes.print("Lon: -54.123456");
    
    tela.display(true);
}


void setup() { 
   Serial.begin(115200); delay(500); 
   WiFi.softAP("BMW - Blackbox Mobility Watcher SETUP", "setup"); 
   Serial.println("WiFi Access Point iniciado!"); 
   Serial.print("IP: " + WiFi.softAPIP().toString()); 
    
   servidor.on("/ajustes", HTTP_GET, pagina_ajustes_GET); 
   servidor.on("/ajustes", HTTP_POST, pagina_ajustes_POST); 
   servidor.begin(); 

   preferencias.begin("ajustesBMW");

tela.init(); 
  tela.setRotation(2); 
  tela.fillScreen(GxEPD_WHITE); 
  tela.display(true); 
   
  fontes.begin(tela); 
  fontes.setForegroundColor(GxEPD_BLACK);

  if (!LittleFS.begin()) {
    Serial.println("Erro ao montar LittleFS");
  }

} 
void loop() { 
   servidor.handleClient(); 
}
