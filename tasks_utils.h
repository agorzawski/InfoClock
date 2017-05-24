/*
 * tasks_utils.h
 *
 *  Created on: 28.12.2016
 *      Author: Bartosz Bielawski
 */

#ifndef TASKS_UTILS_H_
#define TASKS_UTILS_H_

#include "task.hpp"

#include "WebServerTask.h"
#include "DisplayTask.hpp"
#include "AsyncLoggerTask.h"

#include <initializer_list>


struct TaskDescriptor
{
		const static uint8_t ENABLED = 1;
		const static uint8_t CONNECTED = 2;
		const static uint8_t SLOW = 3;

		TaskDescriptor(Tasks::Task* task, uint8_t flags):
			task(task), flags(flags){}

		Tasks::Task* task;
		uint8_t flags;
};

struct RegisterTask
{
		RegisterTask(Tasks::Task* t, uint8_t flags = 0);
		Tasks::Task* task;
};


using PageCallback = std::function<void(ESP8266WebServer&, void*)>;

struct PageDescriptor
{
		PageDescriptor(const char* url, const char* label, PageCallback callback):
			url(url), label(label), callback(callback) {}

		const char* url;
		const char* label;
		PageCallback callback;
};

struct RegisterPage
{
		RegisterPage(const String& url, const String& label, std::function<void(ESP8266WebServer&)> ph);
};


struct RegisterPackage
{
		RegisterPackage(const char* name, Tasks::Task* t, uint8_t flags,
				std::initializer_list<PageDescriptor> pages);
};

void setupTasks();

Tasks::Task* addTask(Tasks::Task* t, uint8_t flags = 0);
void addTask(const TaskDescriptor& td);
void scheduleTasks();

WebServerTask& getWebServerTask();
DisplayTask& getDisplayTask();
AsyncLoggerTask& getLoggerTask();

extern bool slowTaskCanExecute;


#endif /* TASKS_UTILS_H_ */
