

#ifndef __Fee__
#define __Fee__

#include "Payments.hpp"

class fee {
public:
	double  rate;
	bool isFixed;
	string name;

	fee ( double setrate,
			bool setisFixed,
			string setname) :  isFixed(setisFixed), name(setname), rate(setrate){;};

	double adjust( Payment * money,double face,  double yearFracs){

		double cost;
		if (isFixed) {cost =rate* yearFracs;}
		else { cost= face*(rate* yearFracs);}

		money->Principal-=cost;
		return cost;
	};

};

#endif




