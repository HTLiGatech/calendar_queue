#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "sim.h"


/////////////////////////////////////////////////////////////////////////////////////////////
//
// State variables  and other global information
//
/////////////////////////////////////////////////////////////////////////////////////////////
int Service=500;	// # of sales agents available
int	Sales=300;		// # of sales agents available
int Waitlist=0;		// # of callers waiting on the line


int Dials=0;
int Count_Abondoned=0;
int Count_Success=0;

double Sguess=.4;
double Strue=.6;

// Event types
#define	CALL_IN     1
#define	CALLS_OUTP1 2
#define CALLS_OUTP2	3
#define SALES_HU	4
#define SERVICE_HU	5


/////////////////////////////////////////////////////////////////////////////////////////////
//
// Data structures for event data
//
/////////////////////////////////////////////////////////////////////////////////////////////

// Events only have one parameter, the event type
struct EventData {
	int EventType;
};

/////////////////////////////////////////////////////////////////////////////////////////////
//
// Function prototypes
//
/////////////////////////////////////////////////////////////////////////////////////////////

// prototypes for event handlers
void CallIn (struct EventData *e);
void CallsOutp1 (struct EventData *e);
void CallsOutp2 (struct EventData *e);
void SalesHU (struct EventData *e);
void ServiceHU (struct EventData *e);
void Sample		(FILE* ofp);

double urand(void);
double randexp(double u);



/////////////////////////////////////////////////////////////////////////////////////////////
//
// Event Handlers
// Parameter is a pointer to the data portion of the event
//
/////////////////////////////////////////////////////////////////////////////////////////////

// General Event Handler Procedure define in simulation engine interface
void EventHandler (void *data, FILE* ofp)
{
    struct EventData *d;
    
    // coerce type
    d = (struct EventData *) data;
    // call an event handler based on the type of event
    if (d->EventType == CALL_IN) CallIn (d);
    else if (d->EventType == CALLS_OUTP1){ CallsOutp1 (d); Sample(ofp);}
    else if (d->EventType == CALLS_OUTP2) CallsOutp2 (d);
    else if (d->EventType == SALES_HU) SalesHU (d);
    else if (d->EventType == SERVICE_HU) ServiceHU (d);
    else {fprintf (stderr, "Illegal event found\n"); exit(1); }
}

// event handler for arrival events
void CallIn (struct EventData *e)
{
	struct EventData *d;
	double ts;

    //printf ("Processing Incoming Call event at time %f, Waitlist=%d\n", CurrentTime(), Waitlist);
	if (e->EventType != CALL_IN) {fprintf (stderr, "Unexpected event type\n"); exit(1);}

	// schedule next call in using randexp() to assign the next call in event
    if((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    d->EventType = CALL_IN;
    ts = CurrentTime() + randexp(.9);
    Schedule (ts, d);

    // if there are service agents free, put one on the call and schedule a hang up
	if (Service > 0) {

		Service--;
		if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = SERVICE_HU;
		ts = CurrentTime() + 400 + rand() % 300;
		Schedule (ts, d);

	}

	// else if there are sales agents free, put one on the call and schedule a hang up

	else if (Sales>0){

		Sales--;

		if (Sales<0){

		}
		if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = SALES_HU;
		ts = CurrentTime() + 400 + rand() % 300;
		Schedule (ts, d);

	}

	// If no one could pick up the phone, add caller to wait list

	else if( urand() > .75){ Waitlist++; }



	free (e);	// don't forget to free storage for event!
}

// event handler for departure events
void CallsOutp1 (struct EventData *e)
{
    struct EventData *d;
    double ts;
    
    //printf ("Processing Dial Out event at time %f, Available Sales Agents=%d\n", CurrentTime(), Sales);
	if (e->EventType != CALLS_OUTP1) {fprintf (stderr, "Unexpected event type\n"); exit(1);}

	// schedule the answering of call out if there are available sales agents, save the number of sales agents
	// and attempted calls to global variables

	if (Sales>0){

		if((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = CALLS_OUTP2;
		ts = CurrentTime() + 10.0;
		Schedule (ts, d);

		Dials=Sales/Sguess;

	}






	// schedule the next automated call out
    if((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
    d->EventType = CALLS_OUTP1;
    ts = CurrentTime() + 60.0;
    Schedule (ts, d);




	free (e);	// release memory for data part of event
}

void CallsOutp2(struct EventData *e){

    struct EventData *d;
    double ts;

    //printf ("Processing Dial Out event at time %f, Available Sales Agents=%d\n", CurrentTime(), Sales);
	if (e->EventType != CALLS_OUTP2) {fprintf (stderr, "Unexpected event type\n"); exit(1);}

	// check to see if each call was picked up and schedule hang ups for those that were
	int i;
	int g=0;
	for(i=0; i<Dials; i++){

		if (urand()<=Strue){

			if(Sales>0){
				Sales--;
				if((d=malloc(sizeof(struct EventData)))==NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
				d->EventType = SALES_HU;
				ts = CurrentTime() + 200 + rand() % 200;
				Schedule (ts, d);
				g++;

				Count_Success++;
			}
			else{ Count_Abondoned++;}

		}



	}
	if(Dials>20){
		Sguess=(double) g / (double) Dials;
	}

	Dials=0;
}

void ServiceHU (struct EventData *e){

	if (e->EventType != SERVICE_HU) {fprintf (stderr, "Unexpected event type\n"); exit(1);}
	if(Waitlist>0){Waitlist--;}
	else{Service++;}

}

void SalesHU (struct EventData *e){

	if (e->EventType != SALES_HU) {fprintf (stderr, "Unexpected event type\n"); exit(1);}
	if(Waitlist>0){Waitlist--;}
	else{Sales++;}


}

double urand(void)
{
	int a = rand() % 100;

	double b = (double) a / 100;

	return b;
}

double randexp(double u)
{

	double a = -u * (log(1-urand()));

	return a;

}

void Sample(FILE* ofp){

	int a= (int) (CurrentTime()/60.0);

	//fprintf(ofp, "%d \t %d \t %d \t %d \t %d \t %f \t %d \t %d \n", a, Service, Sales, Waitlist, Dials, Sguess, Count_Success, Count_Abondoned);


}

///////////////////////////////////////////////////////////////////////////////////////
//////////// MAIN PROGRAM
///////////////////////////////////////////////////////////////////////////////////////

int main (void)
{
	struct EventData *d;
	double ts;
	FILE* ofp;

	ofp = fopen("./data/Sampling.txt", "w");


	//fprintf(ofp, "Time(m) \t Service \t Sales \t WaitList \t Dials \t S_est \t TotSuccess \t TotAbandoned \n");
	
	//for (int i = 360; i <= 3600; i += 10){

		Initialize();
		// initialize event list with first arrival
		if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = CALL_IN;
		ts = 1.0;
		Schedule (ts, d);
	
	
		if ((d=malloc (sizeof(struct EventData))) == NULL) {fprintf(stderr, "malloc error\n"); exit(1);}
		d->EventType = CALLS_OUTP1;
		ts = 60;
		Schedule (ts, d);
	
	
		RunSim(100.0, ofp);
	//}

	fclose(ofp);
}
