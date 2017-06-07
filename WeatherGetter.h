/*
 * WeatherGetter.h
 *
 *  Created on: 04.01.2017
 *      Author: Bartosz Bielawski
 */

#ifndef WEATHERGETTER_H_
#define WEATHERGETTER_H_

#include <task.hpp>
#include "ESP8266HTTPClient.h"
#include "CounterCRTP.hpp"

class ESP8266WebServer;

class WeatherGetter: public Tasks::Task, public CounterCRTP<WeatherGetter>
{
	public:
		WeatherGetter();
		virtual ~WeatherGetter() {}
		virtual void reset();
		virtual void run();
	private:
		int16_t pressure;
		float temperature;
		String localization;

		String taskName;

		//page handling
		void handleConfig(ESP8266WebServer& ws);
		void handleStatus(ESP8266WebServer& ws);

		//display function
		String getWeatherDescription();
};

#endif /* WEATHERGETTER_H_ */
