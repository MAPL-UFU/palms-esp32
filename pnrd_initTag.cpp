#include <Wire.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <Pn532NfcReader.h>
#include <NfcAdapter.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <PubSubClient.h>

const char *FILE_NAME = "/pnrd_initData.txt";

uint8_t n_places = 2;
uint8_t n_transitions = 2;
uint32_t tagId1 = 0xFF;

// WiFi
String readerId = "initTag";
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

    n_places = file.parseInt();
    n_transitions = file.parseInt();
    int size_mIncidenceMatrix = file.parseInt();
    int size_mStartingTokenVector = file.parseInt();

    int8_t *mIncidenceMatrix = new int8_t[size_mIncidenceMatrix];
    uint16_t *mStartingTokenVector = new uint16_t[size_mStartingTokenVector];

    for (int i = 0; i < size_mIncidenceMatrix; i++)
    {
        mIncidenceMatrix[i] = file.parseInt();
    }

    for (int i = 0; i < size_mStartingTokenVector; i++)
    {
        mStartingTokenVector[i] = file.parseInt();
    }
    String line = file.readStringUntil('\n'); // Read the third line (space)
    ssid = file.readStringUntil('\n');
    password = file.readStringUntil('\n');    // Read the sixth line (password)
    mqtt_server = file.readStringUntil('\n'); // Read the seventh line (mqtt_server)
    mqtt_port = file.parseInt();
    line = file.readStringUntil('\n');          // Read the third line (space)
    mqtt_user = file.readStringUntil('\n');     // Read the ninth line (mqtt_user)
    mqtt_password = file.readStringUntil('\n'); // Read the tenth line (mqtt_password)
    file.close();

    setup_wifi(ssid, password);
    client.setServer(mqtt_server.c_str(), mqtt_port);
    client.setCallback(callback);

    // set this to redefine the pnrd object with the new values
    pnrd1.reconfigure(n_places, n_transitions, false, false);

    reader1->initialize();

    pnrd1.setIncidenceMatrix(mIncidenceMatrix);
    pnrd1.setTokenVector(mStartingTokenVector);
    pnrd1.setAsTagInformation(PetriNetInformation::TOKEN_VECTOR);
    pnrd1.setAsTagInformation(PetriNetInformation::ADJACENCY_LIST);

    Serial.print("\n\nInitial recording of PNRD tags.");
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
    if (Serial.available() > 0)
    {
        int incomingByte = Serial.read();

        Serial.print("I received: ");
        Serial.println(incomingByte, DEC);
    }
    if (!client.connected())
    {
        connect_mqtt(mqtt_server, readerId, mqtt_user, mqtt_password);
    }
    client.loop();
    delay(1000);
    Serial.println("Place a tag near the reader.");
    delay(1000);
    if (pnrd1.saveData() == WriteError::NO_ERROR)
    {
        Serial.println("Tag configurated successfully.");
        pnrd1.printIncidenceMatrix();
        pnrd1.printTokenVector();
        palms_comm(readerId, pnrd1.getTagId(), readerId, 0, "INIT_TAG", "", 0);
    }

    Serial.print("\n\n");
    delay(1000);
}
