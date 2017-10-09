

#ifndef __Tranche__
#define __Tranche__

#include "fee.hpp"
#include "Asset.hpp"
#include "Liability.hpp"


class Waterfall {

protected:

	vector<double> *ForwardCurve;
	vector<Asset *> * assets;
	vector<fee *> * fees;

	map<double, vector<Liability *> *> * tranches;
	list<double>* trancheNums;
	COleDateTime*  ClosingDate, *FirstPaymentDate, *NonCallPeriod, *ReinvestmentPeriod, *MaturityDate;

	double AmountUnscheduledAmortizationReinvestedDuringRP, AmountRecoveriesReinvestedDuringRP, AmountScheduledAmortizationReinvestedDuringRP; // 1
	double AmountUnscheduledAmortizationReinvestedAfterRP, AmountRecoveriesReinvestedAfterRP, AmountScheduledAmortizationReinvestedAfterRP; //0
	double LIBORfloor;
	vector<Asset *> * RP;
	int NumberPeriods;
	vector<double> *yearFracs;

	void 	timeAdvance(COleDateTime *now);
	int FirstDefaultPeriod;

public:

	bool IncrementInMonths; // defaulttrue
	int TimeBetweenPeriods;
	vector<string > * LogIndex;
	map<string, list<string> * > * Log;
	Waterfall (
			vector<Asset*> * setRP,
			string setClosingDate,
			string setFirstPaymentDate,
			string setNonCallPeriod,
			string setReinvestmentPeriod,
			string setMaturityDate,
			vector<double> *setForwardCurve,
			vector<fee *> *setFees,   // ADD FEES
			double setAmountUnscheduledAmortizationReinvestedDuringRP=1,
			double setAmountRecoveriesReinvestedDuringRP=1,
			double setAmountScheduledAmortizationReinvestedDuringRP=1,
			double setAmountUnscheduledAmortizationReinvestedAfterRP=0,
			double setAmountRecoveriesReinvestedAfterRP=0,
			double setAmountScheduledAmortizationReinvestedAfterRP=0,
			int setFirstDefaultPeriod=1,
			bool setIncrementInMonths=true) :
				FirstDefaultPeriod(setFirstDefaultPeriod),
				AmountUnscheduledAmortizationReinvestedDuringRP(setAmountUnscheduledAmortizationReinvestedDuringRP),
				AmountRecoveriesReinvestedDuringRP(setAmountRecoveriesReinvestedDuringRP),
				AmountScheduledAmortizationReinvestedDuringRP(setAmountScheduledAmortizationReinvestedDuringRP),
				AmountUnscheduledAmortizationReinvestedAfterRP(setAmountUnscheduledAmortizationReinvestedAfterRP),
				AmountRecoveriesReinvestedAfterRP(setAmountRecoveriesReinvestedAfterRP),
				AmountScheduledAmortizationReinvestedAfterRP(setAmountScheduledAmortizationReinvestedAfterRP),
				IncrementInMonths(setIncrementInMonths) {

		//Setup the reinvestment portfolio
		RP=new vector<Asset*> ;         // faces need to add up to 1
		assets=new vector<Asset*> ;
		double RPtotalFace=0;
		for (auto i=setRP->begin(); i!=setRP->end(); ) {
			RPtotalFace+=(**i).face;
			Asset * tmp =(**i).clone();

			assets->push_back(tmp);
			RP->push_back(tmp);
			i++;
		}
		assert (RPtotalFace==1);

		fees=setFees;
		ClosingDate=ToTime(setClosingDate);

		FirstPaymentDate=ToTime(setFirstPaymentDate);

		NonCallPeriod=ToTime(setNonCallPeriod);
		ReinvestmentPeriod=ToTime(setReinvestmentPeriod);
		MaturityDate=ToTime(setMaturityDate);
		tranches = new map<double, vector<Liability *> *>;
		trancheNums=new list<double>;
		Log= new map<string, list<string> * >;
		LogIndex= new vector<string>;

		if (setIncrementInMonths){

			TimeBetweenPeriods=FirstPaymentDate->GetMonth() - ClosingDate->GetMonth();
		} else {
			TimeBetweenPeriods= daydiff(FirstPaymentDate,ClosingDate);
		}

		//How much time is between each period?
		auto now=ToTime(setClosingDate);
		auto lastperiod=ToTime(setClosingDate);
		yearFracs= new vector<double>;

		while (daydiff(now, MaturityDate)>=0){
			timeAdvance(now); // check for accuracy
			yearFracs->push_back(yearfrac(lastperiod,now));
			timeAdvance(lastperiod);
		}


		ForwardCurve=setForwardCurve; //Later we will change this so that dates can be entered and it auto adjusts

		NumberPeriods = ForwardCurve->size();
		assert(ForwardCurve->size()>=yearFracs->size());

	};

	static Asset * ReinvestmentAsset(double PercentageReinvestments, double WeightedAveragePrice, double WeightedAverageSpread, double WeightedAverageLife, double WeightedAverageLIBORFloor, string name,
			double PortfolioCPR,double PortfolioCDR,double PortfolioRecoveryRate, int sze){  // Still need to limit life


		Asset * tmp = new Asset(name , PercentageReinvestments, WeightedAverageSpread,

				new vector<double>(sze,PortfolioCDR),new vector<double>(sze,PortfolioCPR),
				Coupon,PortfolioRecoveryRate, 0,true,WeightedAverageLIBORFloor );


		tmp->price=WeightedAveragePrice;

		return tmp;
	};

	void addAssets(double TargetPortfolioSize,double PortfolioSpread,double PortfolioLIBORFloor,double PercentageAssetswithLIBORFloor
			,double PercentageFixedRateAssets,double PortfolioFixedRate,double PortfolioCPR,double PortfolioCDR,
			int FirstDefaultPeriod,double PortfolioRecoveryRate){
		// note that double PortfolioPrice is unused

		// Add the fixed assets first
		addAsset(new Asset("Initial Fixed Assets",TargetPortfolioSize*PercentageFixedRateAssets, PortfolioFixedRate ,
				new vector<double>(yearFracs->size(),PortfolioCDR),new vector<double>(yearFracs->size(),PortfolioCPR), Coupon,PortfolioRecoveryRate, 0,false, -1));

		addAsset(new Asset("Initial Floating Assets No Floor",TargetPortfolioSize*(1-PercentageFixedRateAssets)* (1-PercentageAssetswithLIBORFloor), PortfolioSpread ,
				new vector<double>(yearFracs->size(),PortfolioCDR),new vector<double>(yearFracs->size(),PortfolioCPR), Coupon,PortfolioRecoveryRate, 0,true, -1));

		addAsset(new Asset("Initial Floating Assets With Floor",TargetPortfolioSize*(1-PercentageFixedRateAssets)* PercentageAssetswithLIBORFloor, PortfolioSpread ,
				new vector<double>(yearFracs->size(),PortfolioCDR),new vector<double>(yearFracs->size(),PortfolioCPR), Coupon,PortfolioRecoveryRate, 0,true, PortfolioLIBORFloor ));


	};


	void addAsset(Asset* newAsset){

		(*newAsset->defaultRate)[0]=0;
		(*newAsset->defaultRate)[1]=0;

		assets->push_back(newAsset);
	};

	void addToTranches(string ClassName, double Amount,double setCoupon,string SnPRating, string MoodyRating,string FitchRating,
			double Subordination,double setOCTrigger=0, double setOCCushion=0, double setIC=0, bool isfloating=true ){

		Liability*  tmp= new Liability(ClassName,Amount, setCoupon ,  NULL, NULL, Coupon,0, 0,isfloating, -1); // setRecoveryRate, int setRecoveryDelay na

		addToTranches(1-Subordination,tmp);
	};


	void addToTranches(double seniority, Liability * newLib){

		if (tranches->count(seniority) == 0) {
			(*tranches)[ seniority] = new vector<Liability *>;
			trancheNums->push_back(seniority);trancheNums->sort();
		}
		(*tranches)[ seniority]->push_back(newLib);
	};

	void appendLog(string id){

		(*Log)[id]=new list<string>;
		LogIndex->push_back(id);
		(*Log)[id]->push_back(id);
	}


	void UpdateLog(){


		Payment :: ResetLog();  						// Fix for second run
		delete Log;
		delete LogIndex;
		Log= new map<string, list<string> * >;
		LogIndex= new vector<string>;

		//get the log ready
		appendLog("Payment Date");
		appendLog("Is In Reinvestment Period");
		appendLog("Is In Default Period");
		appendLog("Accrual Period");  // yearfrac
		appendLog("Libor Forward Rates");

		appendLog("Assets");  // Total sum only
		appendLog("Beginning Iniital Portfolio Balance");


		//	appendLog("Amortization Schedule");
		appendLog("Scheduled Prepayment (%)");
		appendLog("Scheduled Prepayments ($)");
		appendLog("Constant Prepayment Rate");
		appendLog("Unscheduled Prepayments ($)");
		appendLog("Defaulted Amount");
		appendLog("Recovery Amount ($)");
		appendLog("Net Losses on Defaults");
		appendLog("Portfolio Collection Account");

		appendLog("Portfolio LIBOR Floor");
		//appendLog("Portfolio Spread");
		appendLog("Interest Reserve");
		appendLog("Interest Collection Account");

		for (auto i=assets->begin(); i!=assets->end(); i++ ) { appendLog("\t" + (**i).seniority+" Balance"); };

		appendLog("Liabilities");
		appendLog("Beginning Principal Balance"); // sum
		for (auto s=trancheNums->begin(); s!=trancheNums->end(); s++ ) {
			for (auto i=(*tranches)[*s]->begin(); i!=(*tranches)[*s]->end(); i++ ) {
				appendLog("\t" + (**i).seniority+" Balance"); }};


		appendLog("Amounts Due Before Interest Waterall");
		appendLog("Fees");
		for (auto i=fees->begin(); i!=fees->end(); i++ ) { appendLog("\t" + (**i).name); };


		appendLog("Interest Waterall");
		for (auto s=trancheNums->begin(); s!=trancheNums->end(); s++ ) {

			for (auto i=(*tranches)[*s]->begin(); i!=(*tranches)[*s]->end(); i++ ) {
				appendLog("\t" + (**i).seniority+" Interest"); }};


		appendLog("Principal Waterfall");
		for (auto s=trancheNums->begin(); s!=trancheNums->end(); s++ ) {
			for (auto i=(*tranches)[*s]->begin(); i!=(*tranches)[*s]->end(); i++ ) {
				appendLog("\t" + (**i).seniority+" Principal Repayment"); }};

		appendLog("Reinvestments");
		appendLog("Amounts of Portfolio Proceeds after Interest");
		appendLog("Amounts of Unscheduled Portfolio Proceeds Reinvested");
		appendLog("Amounts of Scheduled Portfolio Proceeds Reinvested");
		appendLog("Amounts of Recovery Portfolio Proceeds Reinvested");
		appendLog("Total Reinvestments Due");
		appendLog("Actual Reinvestments");
		for (auto i=RP->begin(); i!=RP->end(); i++ ) { appendLog("\t"+(**i).seniority+" Purchased"); 

		};



		int period = Payment :: AdvanceTime();
		Payment * spv=new Payment(" ", "SPV");
		list<Payment*> * payments = new list<Payment*>;
		double Reinvestment;

		COleDateTime * now= new COleDateTime (*ClosingDate);
		char MY_TIME[50];
		bool InReinvestmentPeriod, InDefaultPeriod;;

		double LiabBalence, AssetBalence, cdr, cpr, lfloor=0, CollectionAccount;
		map<string, double> * summary ;

		for (auto Accrual=yearFracs->begin(); Accrual!=yearFracs->end();Accrual++ ) {
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////// for each period

			Reinvestment=0;
			timeAdvance(now); // use the ending date of the period.  If we want the starting date put this at the bottom of the loop
			(*Log)["Payment Date"]->push_back(str(now->Format(_T("%A, %B %d, %Y")   ) ));

			InReinvestmentPeriod=(daydiff(ReinvestmentPeriod, now )<0);
			InDefaultPeriod=(period>=FirstDefaultPeriod);

			(*Log)["Is In Reinvestment Period"]->push_back(str(InReinvestmentPeriod));
			(*Log)["Is In Default Period"]->push_back(str(InDefaultPeriod));
			(*Log)["Accrual Period"]->push_back(str(*Accrual));  // yearfrac
			(*Log)["Libor Forward Rates"]->push_back(str( (*ForwardCurve)[period] ));


			////                                     Assets
			AssetBalence=0;
			Asset :: defaulted=0; // reset asset summary
			summary = NULL;
			for (auto i=assets->begin(); i!=assets->end(); i++ ) {

				(*Log)["\t" + (**i).seniority+" Balance"]->push_back(str((*i)->face));
				AssetBalence+=(*i)->face;
				cdr=(*(**i).defaultRate)[period];
				cpr=(*(**i).Prepay)[period];
				if (LIBORfloor>-.99 ) lfloor= (**i).LIBORfloor;

				payments->splice(payments->begin(),  *(*i)->Amortize( (*ForwardCurve)[period], *Accrual) );

				summary=spv->addAll(payments); // why are we getting 20 M, should be 5M 
			};

			(*Log)["Beginning Iniital Portfolio Balance"]->push_back(str(AssetBalence)); // total

			//(*Log)["Amortization Schedule"]->push_back();
			(*Log)["Scheduled Prepayment (%)"]->push_back(str((*summary)["Prepayment"]/AssetBalence));
			(*Log)["Scheduled Prepayments ($)"]->push_back(str((*summary)["Prepayment"]));
			(*Log)["Constant Prepayment Rate"]->push_back(str(cdr* (*Accrual) ));

			(*Log)["Defaulted Amount"]->push_back(str(Asset :: defaulted));
			(*Log)["Recovery Amount ($)"]->push_back(str((*summary)["Recovery"]));

			(*Log)["Net Losses on Defaults"]->push_back(str( Asset :: defaulted - (*summary)["Recovery"]  ));

			(*Log)["Portfolio Collection Account"]->push_back(str(spv->Principal - spv->Interest ));

			(*Log)["Portfolio LIBOR Floor"]->push_back(str(lfloor));

			(*Log)["Interest Reserve"]->push_back("0");                            // add later

			(*Log)["Interest Collection Account"]->push_back(str((*summary)["Interest"]));

			//pay fees
			for (auto i=fees->begin(); i!=fees->end(); i++ ) {
				(*Log)["\t"+(**i).name]->push_back(str((**i).adjust(spv,AssetBalence, *Accrual )));
			};




			//	Liabilities;

			LiabBalence=0;
			double totalInterest=0;
			for (auto s=trancheNums->begin(); s!=trancheNums->end(); s++ ) {
				// check for pari passu
				for (auto i=(*tranches)[*s]->begin(); i!=(*tranches)[*s]->end(); i++ ) {

					(*Log)["\t"+(**i).seniority+" Balance"]->push_back(str((*i)->face));
					LiabBalence+=(*i)->face;


					double tmp=(**i).PayInterestFrom(spv, (*ForwardCurve)[period], *Accrual);
					totalInterest+=tmp;


					(*Log)["\t"+(**i).seniority+" Interest"]->push_back(str(  tmp   ));

				}};

			(*Log)["Interest Waterall"]->push_back(str(totalInterest));

			(*Log)["Beginning Principal Balance"]->push_back(str(LiabBalence)); // sum

			(*Log)["Amounts of Portfolio Proceeds after Interest"]->push_back(str(spv->Principal));

			//	should we reinvest?
			if (InReinvestmentPeriod){

				Reinvestment+=AmountUnscheduledAmortizationReinvestedDuringRP* (*summary)["Prepayment"];


				(*Log)["Amounts of Unscheduled Portfolio Proceeds Reinvested"]->push_back(
						str(AmountUnscheduledAmortizationReinvestedDuringRP* (*summary)["Prepayment"]));

				Reinvestment+=AmountRecoveriesReinvestedDuringRP* (*summary)["Recovery"];
				(*Log)["Amounts of Recovery Portfolio Proceeds Reinvested"]->push_back(str
						(AmountRecoveriesReinvestedDuringRP* (*summary)["Recovery"]));


				(*Log)["Amounts of Scheduled Portfolio Proceeds Reinvested"]->push_back("0");


			} else {


				Reinvestment+=AmountUnscheduledAmortizationReinvestedAfterRP* (*summary)["Prepayment"];
				(*Log)["Amounts of Unscheduled Portfolio Proceeds Reinvested"]->push_back(
						str(AmountUnscheduledAmortizationReinvestedAfterRP* (*summary)["Prepayment"]));

				Reinvestment+=AmountRecoveriesReinvestedAfterRP* (*summary)["Recovery"];
				(*Log)["Amounts of Recovery Portfolio Proceeds Reinvested"]->push_back(str
						(AmountRecoveriesReinvestedAfterRP* (*summary)["Recovery"]));


				(*Log)["Amounts of Scheduled Portfolio Proceeds Reinvested"]->push_back("0");


				Reinvestment=spv->Principal;
			}


			(*Log)["Total Reinvestments Due"]->push_back(str(Reinvestment));
			Reinvestment=min(Reinvestment, spv->Principal);
			(*Log)["Actual Reinvestments"]->push_back(str(Reinvestment));



			for (auto i=RP->begin(); i!=RP->end(); i++ ) {

				(*Log)[("\t"+(**i).seniority+" Purchased")]->push_back(str(Reinvestment* (**i).price ));
				(**i).face+=Reinvestment* (**i).price;

			};

			spv->Principal-=Reinvestment;


			// Pay out any money left over


			for (auto s=trancheNums->begin(); s!=trancheNums->end(); s++ ) {
				// check for pari passu
				double principalOutstanding=0;
				int LiabsOutstanding=0;

				for (auto i=(*tranches)[*s]->begin(); i!=(*tranches)[*s]->end(); i++ ) {
					principalOutstanding+=(**i).face;
					LiabsOutstanding++;

				}

				for (auto i=(*tranches)[*s]->begin(); i!=(*tranches)[*s]->end(); i++ ) {
					if (principalOutstanding>0.01){
						(*Log)["\t"+(**i).seniority+" Principal Repayment"]->push_back(str((**i).PayPrincipalFrom(spv,

								(**i).face * (**i).face/ principalOutstanding)));
					}

				}


			}





			delete summary;

			period = Payment :: AdvanceTime();

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		};
		delete now;
		delete spv;



	};


	void Log2csv(){

		string *tmp= new string[LogIndex->size()*yearFracs->size()+1];

		int x=0, y=0,my=0;
		for (auto s=LogIndex->begin(); s!=LogIndex->end(); s++ ) {
			y=0;
			for (auto i=(*Log)[*s]->begin(); i!=(*Log)[*s]->end(); i++ ) {
				tmp[x*yearFracs->size() +y]=*i;
				y++;my=max(my,y);}
			x++;};

		cout << x << " " << y << endl;
		ofstream myfile;
		myfile.open ("example.csv");
		for (x=0;x<LogIndex->size();x++) {
			for (y=0;y<my-1;y++) {


				myfile << tmp[x*yearFracs->size() + y] << ",";
			}
			myfile << endl;
		};


		cout << x << " " << y << endl;

		myfile.close();

		cout << "closed" << endl;

	};

};

void Waterfall :: timeAdvance(COleDateTime *now){	
	if (IncrementInMonths){

		int tmp=(TimeBetweenPeriods+ now->GetMonth());
		int year = now->GetYear();

		if (tmp>=12){
			tmp-=12;
			year++;
		}
		now->SetDate(year, tmp, now->GetDay());

	} else {
		now=dayadd(now, TimeBetweenPeriods);
	}
};


#endif




