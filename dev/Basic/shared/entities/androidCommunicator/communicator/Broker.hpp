namespace sim_mob
{
class Broker  : public sim_mob::Agent
{
	static Broker instance;
public:
	explicit Broker(const MutexStrategy& mtxStrat, int id=-1);
	Entity::UpdateStatus update(timeslice now);
	void load(const std::map<std::string, std::string>& configProps);
	bool frame_init(timeslice now);
	Entity::UpdateStatus frame_tick(timeslice now);
	void frame_output(timeslice now);

};


}
