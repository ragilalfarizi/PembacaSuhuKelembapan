#include <WString.h>

String SendHTML(float Temperature,float Humidity, String IP, float chipId){
 String ptr = "<!DOCTYPE html> <html>\n";

 ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
 ptr +="<title>Tugas Besar Kelompok C</title>\n";
 //ptr +="<link rel= "stylesheet" href="include/Style.css">\n";
 ptr +="<style>html { font-family: 'Raleway'; display: inline-block; margin: 0px auto; text-align: center;}\n";
 ptr +="body{margin-top: 50px; font-family: 'Railway'; font-size: 16px; text-align: center; padding: 0;} h1 {font-size: 150%; color: #874356;margin: 50px auto 30px;}\n";
 ptr +="p {font-size: 20px;color: #874356;margin-bottom: 10px;}\n";
 ptr +="jumbotron{ padding: 60px; text-align: center; font-size: 20px; background-color: #874356; color: #874356;}\n";
 ptr +="</style>\n";
 ptr +="</head>\n";

 ptr +="<body>\n";

//gakebaca
 ptr +="<header>\n";
 ptr +="<div id=\"jumbotron\">\n";
 ptr +="<h1>Tugas Besar Kelompok C ^^</h1>\n";
 ptr +="<p>Internet of Things Final Project</p>\n\n";
 ptr +="</div>\n";
 ptr +="</header>\n";

 ptr +="<div id=\"identitas\">\n";
 ptr +="<h1>Group Member</h1>\n";
 ptr +="<p>Yumna Salma Nabila (H1A020002)</p>\n";
 ptr +="<p>Ragil Auliya Alfarizi (H1A020080)</p>\n";
 ptr +="<p>Maria Kusuma Wardani (H1A020090)</p>\n";
 //ptr +="<p>NIM: H1A020090 </p>";
 
 ptr +="</div>\n";
 ptr +="<div id=\"identitas_perangkat\">\n";
 ptr +="<h1>Device Identity</h1>\n";
 ptr +="<p>IP Address</p>";
 ptr +=IP;
 //ptr +="<p>Chip Model</p>";
 //ptr +=chipModel;
 ptr +="<p>Chip ID</p>";
 ptr +=chipId;
 ptr +="</div>\n";
 ptr +="<div id=\"webpage\">\n";
 ptr +="<h1>Temperature and Humidity Read by DHT11</h1>\n";
 ptr +="<p>Temperature: ";
 ptr +=Temperature;
 ptr +=" <sup>o</sup>C</p>";
 ptr +="<p>Humidity: ";
 ptr +=Humidity;
 ptr +=" %</p>";
 ptr +="</div>\n";
 ptr +="<div id=\"saklar\">\n";
 ptr +="<h1>LED Control</h1>\n";
 ptr +="<a href=\"/led_hidup\"><button>ON</button></a>";
 ptr +="<a href=\"/led_mati\"><button>OFF</button></a>";
 ptr +="<h1>Update Firmware</h1>\n";
 ptr +="<a href=\"/Update\"><button>Update</button></a>";
 ptr +="</div>\n";
 ptr +="</body>\n";
 ptr +="</html>\n";
 return ptr; 
}
