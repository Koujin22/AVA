#pragma once
#include <cstdint>

class IMicrophoneService{
public:
	virtual ~IMicrophoneService() {};
	virtual void GetPcm(int16_t*) = 0;
	virtual void FlushPcm(int16_t*) = 0;

};