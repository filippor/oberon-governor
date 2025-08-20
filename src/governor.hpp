#ifndef GOVERNOR_GOVERNOR_HPP
#define GOVERNOR_GOVERNOR_HPP

class Governor {
private:
	class Oberon& _gpu;
	bool r = true;
	int _opp = -1;
	int _opp_count;

	int polling_delay_ms;
	int up_threshold_high;
	int up_threshold_low;
	int down_threshold_high;
	int down_threshold_low;
	int gfx_temp_soft_lim;
	int gfx_temp_hard_lim;
	int soc_temp_hard_lim;
	int overheat_reset_ms;
	bool _verbose = false;
public:
	void run();
	void stop();
	Governor(class Oberon& gpu, bool verbose);
};

#endif // GOVERNOR_GOVERNOR_HPP
