#include <SPI.h>
#include <PN532.h>
#include <PN532_SPI.h>
#include <Pn532NfcReader.h>
#include <NfcAdapter.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <PubSubClient.h>

const int BUFFER_SIZE = 512; // Define um tamanho máximo para sua string. Ajuste conforme necessário.

const char *FILE_NAME = "/pnrd_reader.txt";

uint32_t tagId1 = 0xFF;

uint8_t fire = 1; // Dynamically loaded from file
uint8_t n_places = 2;
uint8_t n_transitions = 2;

uint16_t *tokenVector = new uint16_t[n_places];
String stringToken;

// WiFi
String readerId = "";
String ssid = "";
String password = "";
// MQTT Broker
String mqtt_server = "";
int mqtt_port = 1883;
String mqtt_user = "";
String mqtt_password = "";

PN532_SPI pn532spi1(SPI, 2);
NfcAdapter nfc1 = NfcAdapter(pn532spi1);
Pn532NfcReader *reader1 = new Pn532NfcReader(&nfc1);
Pnrd pnrd1 = Pnrd(reader1, n_places, n_transitions, false, false, false);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi(String ssid, String password)
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void connect_mqtt(String mqtt_server, String topic, String mqtt_user, String mqtt_password)
{
  while (!client.connected())
  {
    String clientId = "client_";
    clientId += topic;

    Serial.print("Attempting MQTT connection...");
    Serial.print("MQTT Server: ");
    Serial.println(mqtt_server);
    Serial.print("Client ID: ");
    Serial.println(clientId);

    if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str()))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600);

  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  File file = SPIFFS.open(FILE_NAME);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return;
  }

  String line = file.readStringUntil('\n'); // Read the first line (int_places)
  n_places = line.toInt();
  line = file.readStringUntil('\n'); // Read the second line (int_transitions)
  n_transitions = line.toInt();
  line = file.readStringUntil('\n'); // Read the third line (fire)
  fire = line.toInt();
  line = file.readStringUntil('\n'); // Read the forth line (readerId)
  readerId = line;
  line = file.readStringUntil('\n'); // Read the fifth line (ssid)
  ssid = line;
  line = file.readStringUntil('\n'); // Read the sixth line (password)
  password = line;
  line = file.readStringUntil('\n'); // Read the seventh line (mqtt_server)
  mqtt_server = line;
  line = file.readStringUntil('\n'); // Read the eighth line (mqtt_port)
  mqtt_port = line.toInt();
  line = file.readStringUntil('\n'); // Read the ninth line (mqtt_user)
  mqtt_user = line;
  line = file.readStringUntil('\n'); // Read the tenth line (mqtt_password)
  mqtt_password = line;

  file.close();

  setup_wifi(ssid, password);
  client.setServer(mqtt_server.c_str(), mqtt_port);
  client.setCallback(callback);

  pnrd1.reconfigure(n_places, n_transitions, false, false);
  reader1->initialize();

  pnrd1.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
  pnrd1.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);
}
void palms_comm(
    String topic,
    uint32_t tagId,
    String readerId,
    int antena,
    const char *ErrorType,
    String tokenVector,
    uint8_t fire)
{
  String mqtt_message = String("I") + String(tagId, HEX) + String("-A") + String(1) + String("-R") + String(readerId) + String("-P") + String(ErrorType) + String("-T[") + String(tokenVector) + String("]") + String("-F") + String(fire) + String("-EE");
  client.publish(topic.c_str(), mqtt_message.c_str());
}

void loop()
{
  delay(50);
  if (!client.connected())
  {
    connect_mqtt(mqtt_server, readerId, mqtt_user, mqtt_password);
  }
  client.loop();
  ReadError readError1 = pnrd1.getData();
  delay(50);
  switch (readError1)
  {
  case ReadError::NO_ERROR:
    FireError fireError1;
    if (tagId1 != pnrd1.getTagId())
    {
      tagId1 = pnrd1.getTagId();
      FireError fireError1 = pnrd1.fire(fire);
      pnrd1.printIncidenceMatrix();
      pnrd1.printTokenVector();
      pnrd1.getTokenVector(tokenVector);
      char stringToken[BUFFER_SIZE] = {0};
      char temp[6]; // Espaço suficiente para um número uint16_t e um caractere nulo.

      for (int32_t place = 0; place < n_places; place++)
      {
        snprintf(temp, sizeof(temp), "%u", tokenVector[fire]); // Converte o número para string.
        strcat(stringToken, temp);

        if (place != n_places - 1)
        {
          strcat(stringToken, ",");
        }
      }
      switch (fireError1)
      {
      case FireError::NO_ERROR:
        if (pnrd1.saveData() == WriteError::NO_ERROR)
        {
          palms_comm(readerId, tagId1, readerId, 0, "NO_ERROR", stringToken, fire);
          return;
        }
        else
        {
          palms_comm(readerId, tagId1, readerId, 0, "ERROR_TAG_UPDATE", stringToken, fire);
          return;
        }

      case FireError::PRODUCE_EXCEPTION:
        palms_comm(readerId, tagId1, readerId, 0, "PRODUCE_EXCEPTION", stringToken, fire);
        return;

      case FireError::CONDITIONS_ARE_NOT_APPLIED:
        palms_comm(readerId, tagId1, readerId, 0, "CONDITIONS_ARE_NOT_APPLIED", stringToken, fire);
        break;

      case FireError::NOT_KNOWN:
        palms_comm(readerId, tagId1, readerId, 0, "NOT_KNOWN", stringToken, fire);
        break;
      }
    }
    break;
  case ReadError::ERROR_UNKNOWN:
    Serial.println("ERROR_UNKNOWN");
    break;
  case ReadError::TAG_NOT_PRESENT:
    Serial.println("TAG_NOT_PRESENT");
    break;
  case ReadError::INFORMATION_NOT_PRESENT:
    Serial.println("INFORMATION_NOT_PRESENT");
    break;
  case ReadError::DATA_SIZE_NOT_COMPATIBLE:
    Serial.println(pnrd1.getNumberOfPlaces());
    Serial.println(pnrd1.getNumberOfTransitions());

    Serial.println("DATA_SIZE_NOT_COMPATIBLE");
    break;
  case ReadError::NOT_AUTORIZED:
    Serial.println("NOT_AUTORIZED");
    break;
  }

  Serial.flush();
}