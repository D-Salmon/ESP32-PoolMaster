

// WiFi credentials
#ifdef DEVT
  #define WIFI_NETWORK "<WIFI_DEVT_NETWORK>"
  #define WIFI_PASSWORD "<WIFI_DEVT_PASSWORD>"
#else
  #define WIFI_NETWORK "<WIFI_NETWORK>"
  #define WIFI_PASSWORD "<WIFI_PASSWORD>"
#endif 

// Mail parameters and credentials
#define SMTP  // define to activate SMTP email notifications
#ifdef SMTP
  #define SMTP_HOST "your smtp server"
  #define SMTP_PORT 587  // check the port number
  #define AUTHOR_EMAIL "your email address"
  #define AUTHOR_LOGIN "your user name"
  #define AUTHOR_PASSWORD "your password"
  #define RECIPIENT_EMAIL "your recipient email address"
#endif

// MQTT stuff including local broker/server IP address, login and pwd
//------------------------------------------------------------------
#define MQTT_SERVER_IP IPAddress(192, 168, 1, 57)
#define MQTT_SERVER_PORT 1883

// Uncomment if MQTT broker needs login/pwd
#define MQTT_LOGIN 				
#define MQTT_SERVER_ID    "ESP32Pool"		   // MQTT server ID
#define MQTT_SERVER_LOGIN "<MQTT_LOGIN>"
#define MQTT_SERVER_PWD   "<MQTT_PASSWORD>"




