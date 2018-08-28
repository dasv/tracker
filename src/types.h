#ifndef TYPES_H
#define TYPES_H

#include "Arduino.h"

struct dataIridium
{
	// Latitude
	float lat;
	// Longitude
	float lon;
	// Velocity
	float vel;
	// Heading angle
	float heading;
	// Maximum roll angle
	float rollmax;
	// Minimum roll angle
	float rollmin;
	// Maximum pitch angle
	float pitchmax;
	// Minimum pitch angle
	float pitchmin;
	// Temperature
	float temp;
	// VatVol
	float vbatt;
	// Nº de satelites   CLO
	int sat;
};

struct marineData {
	uint16_t msg_cnt;
	uint8_t hour;
	uint8_t mins;
	uint8_t secs;
	uint8_t cc;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	double hhmmsscc;
	double ddmmaa;
	dataIridium new_data;
	dataIridium old_data;
};

#endif