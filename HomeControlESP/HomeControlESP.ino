/*
* Copyright (c) 2015, Majenko Technologies
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice, this
*   list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* * Neither the name of Majenko Technologies nor the names of its
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char *ssid = "UPC2633536";//"AndroidT";//"UPC2633536";
const char *password = "niezgadnieszmnie"; //"1234567890";//"niezgadnieszmnie";
MDNSResponder mdns;


const char* host = "api.thingspeak.com";
const char* privateKey = "O6HQMA20AE4YQ3UO";

ESP8266WebServer server(80);

const int led = 13;


void handleNotFound() {
	digitalWrite(led, 1);
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}

	//Serial.println(message);
	server.send(404, "text/plain", message);
	digitalWrite(led, 0);
}

void execute() {
	String msg = "MSG=";
	msg += server.arg("command");
	Serial.println(msg);
	String response = "OK";

	server.send(200, "text/plain", response);
}

void setup(void) {
	pinMode(led, OUTPUT);
	digitalWrite(led, 0);
	Serial.begin(9600);
	WiFi.begin(ssid, password);
	//Serial.println("");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	
	if (mdns.begin("esp8266", WiFi.localIP())) {
		Serial.println("MDNS responder started");
	}

	server.on("/android", execute);
	server.on("/settings", settings);
	server.on("/light", light);
	server.on("/test", ala);
	server.on("/inline", []() {
		server.send(200, "text/plain", "this works as well");
		sendData("20.20");
	});
	server.onNotFound(handleNotFound);
	server.begin();
	Serial.println("HTTP server started");

}

void loop(void) {
	mdns.update();
	server.handleClient();
	if (Serial.available() > 2) {
		String message = Serial.readString();
		if (message.substring(0, 3).equals("III")) {
			Serial.println(WiFi.localIP());
		}
		else if (message.substring(0, 3).equals("TTT")) {
			sendData(message.substring(3));
		}
		/*
		while (Serial.available()) {
			char c = Serial.read();
			message += c;
		}

		if (message.equals("TTT")) {
			message = "";
			
			while (Serial.available()) {
				char c = Serial.read();
				message += c;
				Serial.println(c);
			}
			

			for (int i = 0; i < 3; i++) {
				char c = Serial.read();
					message += c;
			}
			Serial.println(message);
			sendData(message);
		}
		
		if (message.equals("III")) {
			Serial.println(WiFi.localIP());
		}
		
		*/
		while (Serial.available()) {
			Serial.read();
		}
	}

}

void ala() {
	//Serial.println("ala");
	String message = "{\"STATUS\":\"OK\", \"LIGHT\":1, \"BRIGTHNESS\":50, \"TEMP_IN\":22.5, \"TEMP_OUT\":10.5}";
	server.send(200, "text/plain", message);
}

void light() {
	String msg = "MSG=";
	msg += server.arg("state");
	Serial.println(msg);
	String message = "OK";

	server.send(200, "text/plain", message);
}

void settings() {

	int stack = 0;
	while (Serial.available())
		Serial.read();
	Serial.println("MSG=D");
	long temp = millis();
	String data;
	
	while (temp + 1000 > millis()) {
		data += Serial.readString();
		/*
		String str = Serial.readString();
		Serial.println(str);
		if (str.equals("{"))
			stack++;
		
		if (stack > 0) {
			data += str;
			if (str.equals("}"))
				stack--;
			if (stack == 0)
				break;
		}		
		*/		
	}
	server.send(200, "text/plain", data);
}


void sendData(String value) {
	//Serial.println("Sending data to think speak...");

	//Serial.print("connecting to ");
	//Serial.println(host);
	//Serial.print("IP: ");
	//Serial.println(WiFi.localIP());
	// Use WiFiClient class to create TCP connections
	WiFiClient client;
	const int httpPort = 80;
	if (!client.connect(host, httpPort)) {
		Serial.println("connection failed");
		return;
	}

	// We now create a URI for the request
	String url = "/update";
	url += "?api_key=";
	url += privateKey;
	url += "&field1=";
	url += value;

	//Serial.print("Requesting URL: ");
	//Serial.println(url);
	String str = String("GET ") + url + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"Connection: close\r\n\r\n";
	//Serial.println();
	// This will send the request to the server
	client.print(str);
	delay(10);

	// Read all the lines of the reply from server and print them to Serial
	while (client.available()) {
		String line = client.readStringUntil('\r');
		//Serial.print(line);
	}

	//Serial.println();
	//Serial.println("closing connection");
}