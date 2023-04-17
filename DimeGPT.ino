#include <UniversalTelegramBot.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "wifi name";
const char* password = "wifi password";
const char* botToken = "Telegram bot token looks like: 123123123:f43dffsdgedgdfgdfgerertrt";
const char* openaiApiKey =  "sk-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

BearSSL::WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

String poraka;
String chatID;
String completion;
String chatId;
String text;

HTTPClient http;
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Ignore SSL certificate validation
  client.setInsecure();
   
}

void loop() {
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

  while (numNewMessages) {
    Serial.println("New message(s) received:");
    
    HTTPClient http;
    http.setTimeout(10000);
    http.begin(client, "https://api.openai.com/v1/engines/text-davinci-003/completions");

    for (int i = 0; i < numNewMessages; i++) {
      chatId = String(bot.messages[i].chat_id);
      text = bot.messages[i].text;

      // Use GPT-3 to generate a completion     
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", "Bearer " + String(openaiApiKey));

      DynamicJsonDocument doc(5000);
      doc["prompt"] = text;
      doc["max_tokens"] = 3500;
      doc["temperature"] = 0.9;

      String requestBody;
      serializeJson(doc, requestBody);
      int httpResponseCode = http.POST(requestBody);

      if (httpResponseCode == HTTP_CODE_OK) {
        String response = http.getString();

        DynamicJsonDocument responseDoc(5000);
        deserializeJson(responseDoc, response);
        
        completion = responseDoc["choices"][0]["text"].as<String>();
        Serial.println("Completion: " + completion);
        poraka = completion;
        chatID = chatId;

        // Send the completion back to the user
        bot.sendMessage(chatID, poraka);
      } else {
        Serial.println("Error in HTTP request");
        poraka = "[Error] Error in HTTP request or the message is too long.";
      }

      bot.sendMessage(chatID, poraka);
    }
    
    bot.last_message_received = bot.messages[numNewMessages - 1].update_id;
    
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    http.end();
  }
  
  delay(1000);
}