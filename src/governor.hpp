#ifndef GOVERNOR_GOVERNOR_HPP
#define GOVERNOR_GOVERNOR_HPP

#define GOV_POLLING_DELAY_MS	50

#define GOV_UP_THRESHOLD	40
#define GOV_UP_DELAY_MS		GOV_POLLING_DELAY_MS
#define GOV_DOWN_THRESHOLD	20
#define GOV_DOWN_DELAY_MS	1000
#define GOV_GFX_TEMP_LIM	95
#define GOV_SOC_TEMP_LIM	90
#define GOV_OVERHEAT_RESET_MS	5000

class Governor {
private:
	class Oberon& gpu;
	bool r = true;
	int opp = 0;
	int opp_c;
public:
	void run();
	void stop();
	Governor(class Oberon& gpu);
};

#endif // GOVERNOR_GOVERNOR_HPP
