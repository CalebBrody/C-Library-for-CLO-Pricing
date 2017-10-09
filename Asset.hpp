
#ifndef __Asset__
#define __Asset__

#include "SecurityBase.hpp"


class Asset : public Security{
public:
	double price;  // not fully implimented, only used in reinvestment
	list<Payment*> * Amortize(double LIBOR, double);
	using Security::Security;

	Asset * clone(){ // make a new asset with 0 face
		vector<double> * setDefaultRate = new vector<double>(*defaultRate);
		vector<double> * setPrepay= new vector<double>(*Prepay);
		Asset * tmp =new Asset(seniority,0, couponSpread, setDefaultRate, setPrepay,type, recoveryRate, recoveryDelay, isFloating, LIBORfloor);
		tmp->price=face/ price;
		return tmp;
	};

	static double defaulted;


};

double Asset::defaulted=0;
list<Payment*> * Asset::Amortize(double LIBOR, double yearfrac){
	list<Payment*> * payments = new list<Payment*> ;

	//pay interest on whatever is left
	double pmt;

	pmt=getPmt(LIBOR, yearfrac);
	//payments->push_back (new Payment(pmt-min(pmt, Interest),min(pmt, Interest), "Interest", seniority));
	payments->push_back (new Payment(0,pmt, "Interest", seniority));


	//prepayment comes before default
	double PrepayValue=face* ((*Prepay)[period])* yearfrac;
	face-=PrepayValue;
	payments->push_back (new Payment(PrepayValue,0, "Prepayment", seniority));

	if (period==life){
		payments->push_back (new Payment(face,0, "Final Payment", seniority));
		face=0;
	}

	//adjust for default
	assert((*defaultRate)[period]<=1);
	double DefaultValue=face*(*defaultRate)[period]*yearfrac;

	payments->push_back (new Payment(DefaultValue*recoveryRate,0,0,recoveryDelay, "Recovery", seniority+ "  ("+str(period) +")"));
	face-=DefaultValue;
	defaulted+=DefaultValue;
	// we need the unchanged face for interest calculation
	face+=Interest-pmt;




	period++; //go to the next period
	return payments;
};



#endif
