//#include <Arduino.h>
//#if defined(ESP32)
#include <WiFi.h>
//#elif defined(ESP8266)
//#include <ESP8266WiFi.h>
//#endif
#include <Firebase_ESP_Client.h>
#include <Keypad.h>

// Proporciona información sobre el proceso de generación de tokens.
#include <addons/TokenHelper.h>
//#include "addons/TokenHelper.h"
// Proporciona información sobre la impresión de carga útil RTDB y otras funciones auxiliares.
#include "addons/RTDBHelper.h"

// Pines RX y TX para Serial2
#define RXD2 16
#define TXD2 17

// Pines para el teclado matricial
#define ROW_NUM 4 // 4 filas
#define COLUMN_NUM 4 // 4 columnas
char keys[ROW_NUM][COLUMN_NUM] = {
  {'D', 'C', 'B', 'A'},  
  {'#', '9', '6', '7'},
  {'0', '8', '5', '2'},
  {'*', '7', '4', '1'}
};
byte pin_rows[ROW_NUM]      = {12,14,27,26}; // GPIO19, GPIO18, GPIO5, GPIO17 connect to the row pins
byte pin_column[COLUMN_NUM] = {25,33,32,35};   // GPIO16, GPIO4, GPIO0, GPIO2 connect to the column pins
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);

// Credenciales Wi-Fi
#define WIFI_SSID "ERIKG_0923"
#define WIFI_PASSWORD "RudeusGreyrat"

// API Key y URL de la base de datos de Firebase
#define API_KEY "AIzaSyCTyFbOb-bNkkk-3hhzdNt6MqHGGibOkGQ"
#define DATABASE_URL "https://sistemas-en-chips-default-rtdb.firebaseio.com/"

// Define el objeto de datos Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

char lastValidKey = ' '; // Variable para almacenar el último valor válido
char key;
String estado2Value;
String teststr;
//-------------------------------------------definición de funciones----------
// Función para escribir una cadena de caracteres en la base de datos de Firebase
void writeStringToFirebase(const String &data, const String &path) {
  // Escribir la cadena de caracteres en la base de datos en la ruta especificada
  if (Firebase.RTDB.setString(&fbdo, path.c_str(), data.c_str())) {
    printFirebaseSuccess(); // Imprimir mensaje de éxito en la operación de Firebase
  } else {
    printFirebaseError(); // Imprimir mensaje de error en la operación de Firebase
  }
}

// Función para imprimir mensaje de éxito en la operación de Firebase
void printFirebaseSuccess() {
  Serial.println("TYPE: " + fbdo.dataType());
}
// Función para imprimir mensaje de error en la operación de Firebase
void printFirebaseError() {
  Serial.println("FAILED");
  Serial.println("REASON: " + fbdo.errorReason());
}

// Función para leer y comparar datos del puerto Serial 2 y enviar a firebase. 
void CompareFirebase(String str){
if (teststr == "play" || teststr == "pause" || teststr == "siguiente" || teststr == "anterior")
      {
        if (Firebase.RTDB.setString(&fbdo, "test/key", teststr))
        {
          Serial.println("PASSED");
          Serial.println("PATH: " + fbdo.dataPath());
          Serial.println("TYPE: " + fbdo.dataType());
          Serial.println("VALUE: " + teststr);
        }
        else
        {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }
      } 
}

// Función para comparar la tecla presionada en el teclado matricial y enviar a Firebase
void TecMatri_Compare_Firebase(char key) {
  String firebaseString;

  switch (key) {
    case '4':
      firebaseString = "play";
      break;
    case '5':
      firebaseString = "pause";
      break;
    case '6':
      firebaseString = "siguiente";
      break;
    case 'B':
      firebaseString = "anterior";
      break;
    default:
    firebaseString = " ";// Enviar un string vacío si no corresponde a ninguna tecla
  }

  // Enviar el string a Firebase
  if (firebaseString != "") {
    writeStringToFirebase(firebaseString, "test/key");
  }
}
//.-----función que lee firebase y manda  hacia arduino. 
void readStringFirebase(){
  if (Firebase.RTDB.getString(&fbdo, "/test/Serial2")) {
            if (fbdo.dataType() == "string") {
                estado2Value = fbdo.stringData();
                Serial.println(estado2Value);

                // Dependiendo del valor del string, enviar un string específico a Serial2
                if (estado2Value == "adelante") {
                    Serial2.println('7');
                } else if (estado2Value == "atras") {
                    Serial2.println('8');  
                } else if (estado2Value == "derecha") {
                    Serial2.println('9');
                } else if (estado2Value == "izquierda") {
                    Serial2.println('C');
                }else {
                //no se envía nada. 
                }
            }
        } else {
            Serial.println(fbdo.errorReason());
        }
  }
//-------------------------------------------------------------------setup
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  //Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // Inicializar Serial2
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.ready() && Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  
  config.token_status_callback = tokenStatusCallback; // Ver addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
   key = keypad.getKey(); // Leer el dato del teclado matricial

  if (key != NO_KEY) { // Si se ha presionado una tecla
    // Comparar el carácter leído y enviar el valor al puerto Serial2
    if (key == '7' || key == '8' || key == '9' || key == 'C' || key == '*') {
      lastValidKey = key;
      Serial2.println(String(key)); // Enviar la tecla válida al puerto Serial2
    } else if (lastValidKey != ' ') {
      //no se envía nada.
    }
    // Llamar a la función para comparar la tecla presionada y enviar a Firebase
    TecMatri_Compare_Firebase(key);
    
  }
  // Leer y comparar datos del puerto Serial2
  String teststr = Serial2.readString();
    CompareFirebase(teststr);

}
