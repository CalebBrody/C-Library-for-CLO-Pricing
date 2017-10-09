#ifndef __SecurityBase__
#define __SecurityBase__

#include "Payments.hpp"
#include "CBTime.hpp"
#include "bondtype.hpp"

class Security{

public:
	Security(string setName,double setFace, double setCouponSpread,   vector<double> * setDefaultRate, vector<double> * setPrepay, BondType setType,double setRecoveryRate, int setRecoveryDelay, bool setIsFloating, double setFloor=-1 ) :
		seniority(setName),
		face(setFace),
		type(setType),
		couponSpread(setCouponSpread),
		recoveryRate(setRecoveryRate),
		recoveryDelay(setRecoveryDelay),
		LIBORfloor(setFloor),
		isFloating(setIsFloating)
{period=0;

defaultRate=setDefaultRate;
Prepay=setPrepay;

if (defaultRate) life= (defaultRate->size());
}; // this creates a new Security

	double getPmt(double, double), face;
	double Interest;
	int period =0, life;

	string seniority;
	bool isFloating;
	double couponSpread, LIBORfloor;
	vector<double> *defaultRate, *AmortizationProfile, *Prepay; // americanizationProfile is in dollars
	double recoveryRate;
	int recoveryDelay;
	double FixedPayment; // only for self admer
	BondType type;

};


double Security::getPmt(double LIBOR, double yearfrac){
	double pmt;
	if (isFloating)
		Interest=face*(max(LIBORfloor, LIBOR)+couponSpread)* yearfrac;
	else
		Interest=face*couponSpread* yearfrac;

	switch (type){
	case(SelfAmortizing): pmt=FixedPayment* yearfrac;  break;;
	case(Coupon): pmt=Interest; break;
	case(Bullet): pmt=0; break;
	default: pmt=(*AmortizationProfile)[period]* yearfrac; break;
	};

	return pmt;
};



#endif


