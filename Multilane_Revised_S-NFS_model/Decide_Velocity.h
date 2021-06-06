#ifndef DECIDE_VELOCITY.H
#define DECIDE_VELOCITY.H

#include "Initialize.h"

class Decide_Velocity :public Initialize {
private:
	int _rule1(int ID, int temporaryvelocity);
	int _rule2(int ID, int temporaryvelocity);
	int _rule3(int ID, int temporaryvelocity);
	int _rule4(int ID, int temporaryvelocity);
	double p = 0;
public:
	void Decide_targetvelocity();
};
#endif // !DECIDE_VELOCITY.H

