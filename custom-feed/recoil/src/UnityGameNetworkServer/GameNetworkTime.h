
#include <time.h>

#ifndef _GAME_NETWORK_TIME_H
#define _GAME_NETWORK_TIME_H

class GameNetworkTime {

public:
	GameNetworkTime() {
		timeStart = 0.0f;
		timeLastUpdate = 0.0f;
	}

	~GameNetworkTime() {

	}

	void Start() {
		timeStart = getCurrentTime();
	}

	// Gets the current time in seconds since 01/01/71
	time_t getCurrentTime() {
		time_t ltime;

		// Get UNIX-style time and display as number and string.   
		time(&ltime);

		return ltime;
	}

	time_t getTimeSinceLevelLoad() {
		return getCurrentTime() - timeStart;
	}


	time_t getDeltaTime() {
		return getCurrentTime() - timeLastUpdate;
	}

	void Update() {
		timeLastUpdate = getCurrentTime();
	}

private:

	time_t timeStart;

	time_t timeLastUpdate;

};
#endif