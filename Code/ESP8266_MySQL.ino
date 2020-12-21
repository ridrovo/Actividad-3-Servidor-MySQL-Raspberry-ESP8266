#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <SimpleDHT.h>

uint8_t timeout, intentos;
bool errorconexion=false;
char ssid[] = "dragino-1c0ad8";   // SSID de la red o router donde se conecta el módulo
char pass[] = "dragino-dragino";   // Password de la red o router
byte mac[6];

int temperature = 0;
int humidity = 0;
int pinDHT11 = 2;
SimpleDHT11 dht11;

WiFiServer server(80);
// IPAddress ip(192, 168, 1, 144);  // Define IP del módulo
// IPAddress gateway(192, 168, 1, 1); // Define Puerta de enlace del módulo
// IPAddress subnet(255, 255, 255, 0); // Define máscara del módulo
// IPAddress DNS1(8, 8, 8, 8); //Opcional
// IPAddress DNS2(8, 8, 4, 4); //Opcional
WiFiClient client;  //Crea cliente WIfi
MySQL_Connection conn((Client *)&client); //Crea un objeto de conexión entre el cliente y MySQL
// Prepara la trama para la consulta SQL
char BASE_SQL[] = "INSERT INTO IoTBD.DHT11 (Temperatura, Humedad) VALUES (%f, %f)";
char consulta[128];

IPAddress server_addr(10, 130 , 1, 231);          // IP del servidor MySQL (IP de raspberry)
char user[] = "aballada";           // Usuario con permisos para MySQL
char password[] = "raspberry";       // Contraseña del usuario para MySQL

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando conexión");
// Serial.print("Configurando IP estática a : ");
// Serial.print(F("Setting static ip to : "));
// Serial.println(ip);
// Serial.println("");
  Serial.print("Conectando a la red Wifi con SSID : ");
  Serial.println(ssid); // opcional
  // WiFi.config(ip, gateway, subnet);  // Configura los datos de conexión del módulo Wifi para una IP estatica
  //WiFi.config(ip, gateway, subnet, DNS1, DNS2):  //Opcional
  WiFi.begin(ssid, pass);           //Inicia la conexión con la red o router, en este caso con el dragino
 timeout=0;
  while ((WiFi.status() != WL_CONNECTED) && (timeout++ < 150)) { //Espera a conectarse a la red hasta unos 30s
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  if (timeout>=151){
    Serial.println("Error al conectar a la red Wifi");
    errorconexion=true;
  } else {
    Serial.println("Módulo conectado a WiFi");
 // muestra_mac();          //Muestra la MAC asignada
    Serial.print("IP asignada: ");
    Serial.print(WiFi.localIP());   //Muestra la IP asignada real
    Serial.println("");
    Serial.println("Conectando a base de datos");
    intentos=0;
    while ((conn.connect(server_addr, 3306, user, password) != true)&& (intentos++ < 3)) {  //Realiza varios intentos de conexión a MySQL
      delay(200);
      Serial.print ( "." );
    }
    if (intentos>=4){
    Serial.println("Error al conectar al servidor");
    errorconexion=true;
     }else {
      Serial.println("");
      Serial.println("Conectado al servidor SQL!");  
      }
    }  
}
void loop() {

  byte temperature = 0;
  byte humidity = 0;
  byte data[40] = {0};
  dht11.read(pinDHT11, &temperature, &humidity, data);
  Serial.print((int)temperature); Serial.print(","); Serial.println((int)humidity);
   // DHT11 sampling rate is 1HZ.
  delay(2000);
  if (!errorconexion){
    delay(1000);
    Serial.println("Guardando datos");
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);  // Inicializa el cursor para la consulta SQL
    sprintf(consulta,BASE_SQL, millis(), temperature, humidity); // Completa la trama de la consulta SQL
    Serial.println(consulta);
    cur_mem->execute(consulta);  // Ejecuta la consulta SQL y escribe el valor de los milisegundos
    // sprintf(consulta,BASE_SQL,"'microsegundos'",micros()); // Completa la trama de la consulta SQL
    // Serial.println(consulta);  
    // cur_mem->execute(consulta);  // Ejecuta la consulta SQL y escribe el valor de los microsegundos
    delete cur_mem;  // Al no haber resultados, elimina el cursor y libera memoria 
  }
}
