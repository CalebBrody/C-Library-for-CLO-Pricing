
#ifndef __Liability__
#define __Liability__

#include "SecurityBase.hpp"

class Liability : public Security{
public:
	using Security::Security;
	double PayInterestFrom(Payment*, double, double) ;
	void SetupTests(double setOCTrigger, double setOCCushion, double setIC);
	double PayPrincipalFrom(Payment*) ;
	double PayPrincipalFrom(Payment*, double );

	double OCTrigger,OCCushion, IC;
};

void Liability :: SetupTests(double setOCTrigger=0, double setOCCushion=0, double setIC=0){
	OCTrigger=setOCTrigger;
	OCCushion=setOCCushion;
	IC=setIC;
};


double Liability :: PayInterestFrom(Payment* Money, double LIBOR, double yearfrac){
	double pmt=getPmt(LIBOR, yearfrac);
	if (Money->Principal<pmt){

		//cout << "Default - cant pay"<< endl;
		//throw runtime_error("Default - cant pay");
	}

	Money->Principal-=pmt;
	pmt-= Interest;
	face -= pmt;  // use any money leftover from payment to pay donwn the bonds

	period++;

	return Interest;
}

double Liability :: PayPrincipalFrom(Payment* Money, double amount){ // return true if we are out of money
	if (Money->Principal < amount){


		amount= Money->Principal;
		if (Payment :: LogPayment) {
			Money->closeSelf("Face to Tranche: "+ seniority +" New Bal="+str(face) +" -");
		};
	};

	Money->Principal-= amount;
	face-=amount;
	return amount;
}

double Liability :: PayPrincipalFrom(Payment* Money){ // return true if we are out of money
	return PayPrincipalFrom(Money, face);
}

#endif


