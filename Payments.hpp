#ifndef __payment__
#define __payment__

#include "CBTime.hpp"

class Payment {
protected:
	static map<int, map<string, string> * > * PaymentLog;
	static int period;

public:
	double Interest, Principal,  Enforcement, Delay;
	string name, parent;
	static bool LogPayment;

	static void seeLog();
	void addToLog();
	Payment * closeLog(Payment *);;
	void closeSelf(string);;
	string id;

	static void ResetLog(){
		period=-1;
		delete PaymentLog;
		PaymentLog =new map<int, map<string, string> * >;
	}

	static int AdvanceTime(){
		period++;
		if (LogPayment) (* PaymentLog)[period] = new map<string, string> ;
		return period;
	};

	Payment(string to, string from){
		Interest=0;
		Principal=0;
		Enforcement=0;
		Delay=0;
		name=to;
		parent=from;
		if (LogPayment) addToLog();

	}

	Payment( double newPrincipal,double newInterest, string to, string from){
		Interest=newInterest;
		Principal=newPrincipal;
		Enforcement=0;
		Delay=0;
		name=to;
		parent=from;

		if (LogPayment) addToLog();

	}

	Payment( double newPrincipal,double newInterest,  double newEnforcement, double newDelay, string to, string from){
		Interest=newInterest;
		Principal=newPrincipal;
		Enforcement=newEnforcement;
		Delay=newDelay;
		name=to;
		parent=from;

		if (LogPayment) addToLog();
	}

protected: Payment * addup(Payment *newPayment){

	assert(newPayment->Delay==0);
	Principal+=newPayment->Principal+newPayment->Interest+ newPayment->Enforcement; // everything goes into the same account when we add it up
	Interest+=newPayment->Interest; // we want to keep track of interest seperately for some metrics;

	if (LogPayment) closeLog(newPayment);

	return this;
}
public : map<string, double> * addAll(list<Payment*> * payments){

	map<string, double> * summary = new map<string, double>;

	for (list<Payment*>::iterator i=payments->begin();
			i!=payments->end(); ) {

		//add to summary
		if (summary->count((**i).name)==0) (*summary)[(**i).name]==0;

		// add value
		if (((*i)->Delay) >0){
			(**i).Delay--;
			i++;
		}else{
			(*summary)[(**i).name]+=(**i).Principal+(**i).Interest+ (**i).Enforcement;

			addup(*i);
			i=payments->erase(i);

		}

	}
	return summary;
}

};


void Payment :: addToLog(){
	std::stringstream ss;
	ss << this;
	id = parent + " " + name + " @ " + ss.str();
	(*(*PaymentLog)[period])[id]="Open $"+str(Interest+Principal);
};
Payment * Payment :: closeLog(Payment *newPayment){
	(*(*PaymentLog)[period])[newPayment->id]=id+" $"+str(newPayment->Interest+newPayment->Principal);
	return this;
};

void Payment :: closeSelf(string newPayment){
	(*(*PaymentLog)[period])[id]=newPayment+" $"+str(Principal);
};

void Payment :: seeLog(){
	for (const auto &t : (*PaymentLog)){
		cout << "Period " << t.first << endl;
		for (const auto &i : *((*PaymentLog)[t.first])){
			cout << "\t" << i.first << " \t -> \t" <<i.second << endl;
		}}
};

int Payment :: period=-1;
map<int, map<string, string> * > * Payment :: PaymentLog =new map<int, map<string, string> * >;
bool Payment :: LogPayment=true;

#endif



