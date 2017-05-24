/*
 * WebServerTask.cpp
 *
 *  Created on: 10.01.2017
 *      Author: Bartosz Bielawski
 */


#include "Arduino.h"
#include "time.h"
#include "FS.h"


#include "WebServerTask.h"
#include "ESP8266WebServer.h"
#include "ESP8266HTTPUpdateServer.h"
#include "ArduinoOTA.h"

#include "MacroStringReplace.h"
#include "config.h"
#include "DataStore.h"

#include "html/webpage.h"

#include "utils.h"
#include "tasks_utils.h"
#include "web_utils.h"
#include "LambdaTask.hpp"

FlashStream statusFS(statusPage);

void handleStatus(ESP8266WebServer& webServer)
{
	StringStream ss(2048);

	macroStringReplace(pageHeaderFS, constString("Status"), ss);
	macroStringReplace(statusFS, dataSource, ss);
	webServer.send(200, textHtml, ss.buffer);
}


void handleReset(ESP8266WebServer& webServer)
{
	if (!handleAuth(webServer)) return;

	getDisplayTask().pushMessage("Rebooting...", 5_s, false);
	LambdaTask* lt = new LambdaTask([](){ESP.restart();});
	addTask(lt, TaskDescriptor::ENABLED);
	logPrintfA(F("WS"), F("Rebooting in 5 seconds..."));
	lt->sleep(5_s);

	webServer.sendHeader("Location", String("/"), true);
	webServer.send(302, "text/plain", "");
}


//TODO: remove
void handleReadParams(ESP8266WebServer& webServer)
{
	if (!handleAuth(webServer)) return;

	logPrintfX(F("WST"), F("WST: Reading params..."));
	auto dir = SPIFFS.openDir(F("/config"));

	String response;

	while (dir.next())
	{
		response += dir.fileName();
		response += " -> ";
		auto f = dir.openFile("r");
		response += f.readStringUntil('\n');
		response += '\n';
	}

	for (auto& key: DataStore::availableKeys())
	{
		response += key;
		response += " -> ";
		response += DataStore::value(key);
		response += '\n';
	}

	webServer.send(200, textPlain, response);
}

FlashStream webmessageFS(webmessagePage);

void handleWebMessage(ESP8266WebServer& webServer)
{
	if (!handleAuth(webServer))
		return;

	auto wm = webServer.arg(F("webmessage"));
	if (wm.length())
	{
		DataStore::value(F("webmessage")) = wm;
		DataStore::value(F("webmessageIP")) = webServer.client().remoteIP().toString();
	}

	StringStream ss(2048);

	macroStringReplace(pageHeaderFS, constString("Webmessage"), ss);
	macroStringReplace(webmessageFS, dataSource, ss);

	webServer.send(200, textHtml, ss.buffer);
}


FlashStream generalSettingsFS(generalSettingsPage);

void handleGeneralSettings(ESP8266WebServer& webServer)
{
	if (!handleAuth(webServer)) return;

	auto submitted = webServer.arg(F("submitted"));
	if (submitted.length())
	{
		logPrintfX(F("WST"), F("Saving data..."));
		writeConfig(F("essid"), webServer.arg(F("essid")));

		//we don't send the current value so we have to check if it is present
		auto pwd = webServer.arg(F("wifiPassword"));
		if (pwd.length())
			writeConfig(F("wifiPassword"), pwd);

		writeConfig(F("timezone"), webServer.arg(F("timezone")));

		auto ss = webServer.arg(F("syslogServer"));
		writeConfig(F("syslogServer"), ss);
		//the new syslog settings will be used after reboot
	}

	StringStream ss(2048);

	macroStringReplace(pageHeaderFS, constString("WiFi Settings"), ss);
	macroStringReplace(generalSettingsFS, dataSource, ss);

	webServer.send(200, textHtml, ss.buffer);
}

WebServerTask::WebServerTask():
		webServer(80)
{
	reset();
}

String WebServerTask::generateLinks()
{
	String results;

	for (auto& p: registeredPages)
	{
		results += "<a href=\"";
		results += p.first;
		results += "\">";
		results += p.second;
		results += "</a>\n";
	}

	return results;
}

FlashStream mainPageFS(mainPage);

void WebServerTask::handleMainPage()
{
	StringStream ss(2048);
	macroStringReplace(mainPageFS, [this] (const char*) {return generateLinks();}, ss);
	webServer.send(200, textHtml, ss.buffer);
}

void WebServerTask::registerPage(const String& url, const String& label,
		std::function<void(ESP8266WebServer& ws)> ph)
{
	registeredPages.emplace_back(url, label);
	String newUrl("/");
	newUrl += url;
	std::function<void(void)> f = [this, ph] () {ph(webServer);};
	webServer.on(newUrl.c_str(), f);
}

void WebServerTask::reset()
{
	webServer.stop();
	sleep(5_s);
	started = false;
}


void WebServerTask::run()
{
	if (!started)
	{
		started = true;
		logPrintfX(F("WST"), F("Configuring server"));

		webServer.onNotFound([this](){
			webServer.send(404, "text/plain", "Not found... :/");
		});

		webServer.on("/", [this](){
			this->handleMainPage();
		});

		//webServer.on("/owm", [this](){handleWeatherServiceConfig(webServer);});
		webServer.on("/webmessage", [this](){handleWebMessage(webServer);});
		webServer.on("/settings", [this] (){handleGeneralSettings(webServer);});

		webServer.on("/readParams", [this](){handleReadParams(webServer);});
		webServer.on("/status", 	[this](){handleStatus(webServer);});
		webServer.on("/reset", [this]{handleReset(webServer);});

		webServer.begin();

		logPrintfX(F("WST"), F("Ready!"));
		return;
	}

	webServer.handleClient();
	delay(0);
}

WebServerTask::~WebServerTask()
{
}

