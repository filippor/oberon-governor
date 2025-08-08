#ifndef GOVERNOR_GOVERNOR_HPP
#define GOVERNOR_GOVERNOR_HPP

class Governor {
private:
	class Oberon& gpu;
	bool r = true;
	int opp = 0;
	int opp_c;

	int polling_delay_ms;
	int up_threshold_high;
	int up_threshold_low;
	int down_threshold_high;
	int down_threshold_low;
	int gfx_temp_soft_lim;
	int gfx_temp_hard_lim;
	int soc_temp_hard_lim;
	int overheat_reset_ms;
public:
	void run();
	void stop();
	Governor(class Oberon& gpu);
};

#endif // GOVERNOR_GOVERNOR_HPP
