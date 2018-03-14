#include "../include/Exception.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace Lib;

Exception::Exception(const char* message) {
	m_message = strdup(message);
	m_location = NULL;
}

Exception::Exception(const char* message, const char* file, int line) {
	m_message = strdup(message);

	if (file != NULL) {
		char buffer[16] = { 0 };
		sprintf(buffer,"%d",line);

		// m_location = new char[strlen(file) + strlen(buffer) + 2];
		// m_location = (char*)::operator new(strlen(file) + strlen(buffer) + 2);
		m_location = (char*)malloc(sizeof(char) * (strlen(file) + strlen(buffer) + 2));

		if (m_location) {
			strcpy(m_location, file);
			strcat(m_location, ":");
			strcat(m_location, buffer);
		}
	}
	else {
		m_location = NULL;
	}
}

Exception::Exception(const Exception& e) {
	m_message = strdup(e.m_message);
	m_location = strdup(e.m_location);
}

Exception& Exception::operator=(const Exception& e) {
	if (this != &e) {
		free(m_message);
		free(m_location);
		m_message = strdup(e.m_message);
		m_location = strdup(e.m_location);
	}
	return *this;
}

Exception::~Exception() {
	if (m_message)
		free(m_message);

	if (m_location)
		free(m_location);
}

const char* Exception::message() {
	return (m_message ? m_message : getName());
}

const char* Exception::location() {
	return (m_location ? m_location : "");
}

const char* Exception::getName() {
	return "unknown exception";
}

const char* IndexOutOfBoundsException::getName() {
	return "index out of bounds exception";
}

const char* NullPointerException::getName() {
	return "null pointer exception";
}

const char* InvalidParameterException::getName() {
	return "invalid parameter exception";
}

const char* NoMemoryException::getName() {
	return "no enough memory exception";
}

const char* InvalidOperationException::getName() {
	return "invalid operation exception";
}

const char* ErrorStateException::getName() {
	return "error state exception";
}
