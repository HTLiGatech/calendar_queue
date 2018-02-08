#include <stdio.h>
#include <stdlib.h>
#include "sim.h"
#include <time.h>

// Data structure for an event; this is independent of the application domain

// Event is the element in calendar queue
typedef struct Event{
	double timeStamp;		// event time stamp
	void *appData;			// pointer to event parameters
	struct Event *next;		// pointer to next event in a same bucket, NULL if no next
} Event;

//function declaration (FEL related)
//void Schedule(double ts, void *data);	//push an event into FEL (has been included in sim.h)
Event* Remove();						//pop an event from FEL
void Resize(int newSize);				//change the size of FEL's bucketArray
void PrintList();						//print events in FEL

//function declaration (others)
//double CurrentTime();				//show current time (has been included in sim.h)
//void RunSim(double endTime);		//run simulation (has been included in sim.h)
//void Initialize();				//initialize FEL (has been included in sim.h)

//global variables in calendar queue: Future Event List (FEL)
int nBucket;				//number of buckets
double bucketCapacity;		//width of every bucket
int queueSize;				//number of events
int lastBucket;				//the bucket that has just delete an event from
double now;					//time stamp of last event
double roundTop;			//time stamp limit of current round of the whole bucket array
Event** bucketArray;		//array of bucket


void Initialize(){
	nBucket = 1;
	queueSize = 0;
	lastBucket = 0;
	now = 0.0;
	roundTop = 1.0;
	bucketCapacity = roundTop / nBucket;

	bucketArray = malloc(sizeof(Event*) * nBucket);
	if (bucketArray == NULL){
		printf("Cannot malloc bucketArray.\n"); 
		exit(1);
	}
	for (int i = 0; i < nBucket; i++){
		bucketArray[i] = (Event*)malloc(sizeof(Event));
		if (bucketArray[i] == NULL){
			printf("Cannot malloc bucketArray.\n");
			exit(1);
		}
	}
	for (int i = 0; i < nBucket; i++){
		bucketArray[i] -> timeStamp = -1.0;
		bucketArray[i] -> appData = NULL;
		bucketArray[i] -> next = NULL;
	}	
}

void Schedule(double ts, void *data){
	int i = ((int) (ts / bucketCapacity)) % nBucket;

	Event *e, *p, *q;
	e = malloc(sizeof(Event));
	if (e == NULL){
		printf("Cannot malloc e.\n");
		exit(1);
	}
	e -> timeStamp = ts;
	e -> appData = data;
	for (q = bucketArray[i], p = bucketArray[i] -> next; p != NULL; p = p -> next, q = q -> next){
		if (p -> timeStamp >= e -> timeStamp){
			break;
		}
	}
	e -> next = q -> next;
	q -> next = e;
	queueSize++;
	
	if (queueSize > (2 * nBucket)){
		Resize(2 * nBucket);
	}
}

Event* Remove(){
	if (queueSize == 0){
		return NULL;
	}
	int i = lastBucket;
	while (1){
		//if found item to remove
		if (bucketArray[i] -> next != NULL && bucketArray[i] -> next -> timeStamp < roundTop){
			//remove item
			Event* tempItem = bucketArray[i] -> next;
			bucketArray[i] -> next = tempItem -> next;
			//update FEL parameters
			queueSize--;
			lastBucket = i;
			now = tempItem -> timeStamp;
			if (queueSize < (nBucket / 2)){
				Resize(nBucket / 2);
			}
			return tempItem;
		}
		else{
			//check next bucket
			i++;
			if (i == nBucket){
				i = 0;
				roundTop += bucketCapacity * nBucket;
			}
			if (i == lastBucket){
				//go to direct search
				double nearestFuture;
				for (int j = 0; j < nBucket; j++){
					if (bucketArray[j] -> next != NULL){
						nearestFuture = bucketArray[j] -> next -> timeStamp;
						i = j;
					}
				}
				for (int k = 0; k < nBucket; k++){
					if (bucketArray[k] -> next != NULL){
						if (bucketArray[k] -> next -> timeStamp < nearestFuture){
							nearestFuture = bucketArray[k] -> next -> timeStamp;
							i = k;
						}
					}
				}
				roundTop =  (1 + (int) (bucketArray[i] -> next -> timeStamp / (bucketCapacity * nBucket))) * (bucketCapacity * nBucket);
			}
		}
	}
}

void Resize(int newSize){
	bucketCapacity = bucketCapacity * nBucket / newSize;
	Event** newArray;
	newArray = (Event**)malloc(sizeof(Event*) * newSize);
	if (newArray == NULL){
		printf("Cannot malloc newArray.\n");
		exit(1);
	}
	for (int i = 0; i < newSize; i++){
		newArray[i] = (Event*)malloc(sizeof(Event));
		if (newArray[i] == NULL){
			printf("Cannot malloc newArray.\n");
			exit(1);
		}
	}

	for (int i = 0; i < newSize; i++){
		newArray[i] -> timeStamp = -1.0;
		newArray[i] -> appData = NULL;
		newArray[i] -> next = NULL;
	}

	double ts;
	void* data;
	int i;
	Event *e, *p, *q;
	e = malloc(sizeof(Event));
	if (e == NULL){
		printf("Cannot malloc e.\n");
		exit(1);
	}
	
	Event *f;
	for (int j = 0; j < nBucket; j++){
		for (e = bucketArray[j] -> next; e != NULL; e = e -> next){
			ts = e -> timeStamp;
			data = e -> appData;
			i = ((int) (ts / bucketCapacity)) % newSize;

			for (q = newArray[i], p = newArray[i] -> next; p != NULL; p = p -> next, q = q -> next){
				if (p -> timeStamp >= ts){
					break;
				}
			}
			f = (Event*)malloc(sizeof(Event));
			if (f == NULL){
				printf("Cannot malloc f.\n");
				exit(1);
			}
			f -> timeStamp = ts;
			f -> appData = data;
			f -> next = q -> next;
			q -> next = f;
		}
	}

	for (int i = 0; i < nBucket; i++){
		free(bucketArray[i]);
	}
	free(bucketArray);
	bucketArray = malloc(sizeof(Event*) * newSize);
	if (bucketArray == NULL){
		printf("Cannot malloc bucketArray.\n"); 
		return;
	}
	for (int i = 0; i < newSize; i++){
		bucketArray[i] = (Event*)malloc(sizeof(Event));
		if (bucketArray[i] == NULL){
			printf("Cannot malloc bucketArray.\n");
			return;
		}
	}
	bucketArray = newArray;
	newArray = NULL;
	nBucket = newSize;
	for (int i = 0; i < nBucket; i++){
		if (bucketArray[i] -> next != NULL){
			lastBucket = i;
			break;
		}
	}
	for (int i = 0; i < nBucket; i++){
		if (bucketArray[i] -> next == NULL){
			continue;
		}
		if (bucketArray[i] -> next -> timeStamp < bucketArray[lastBucket] -> next -> timeStamp){
			lastBucket = i;
		}
	}
}

void PrintList(){
	printf("==========\n");
	printf("nBucket = %d\n", nBucket);
	printf("bucketCapacity = %f\n", bucketCapacity);
	printf("queueSize = %d\n", queueSize);
	printf("lastBucket = %d\n",lastBucket);
	printf("now = %f\n", now);
	printf("roundTop = %f\n", roundTop);
	Event* p;
	printf("bucketArray:\n");
	for (int i = 0; i < nBucket; i++){
		printf("bucketArray[%d]: ",i);
		for (p = bucketArray[i] -> next; p != NULL; p = p -> next) {
			printf("%f ", p -> timeStamp);
		}
		printf("\n");
	}

}

double CurrentTime(){
	return now;
}

void RunSim(double endTime, FILE* ofp){
	Event* e;
	clock_t start, finish;
	
	//printf ("Initial event list:\n");
	//PrintList ();
	
	//event processing loop
	start = clock();
	while ((e = Remove()) != NULL){
		now = e -> timeStamp;
		if (now > endTime){
			break;
		}
		EventHandler(e -> appData, ofp);
	}
	finish = clock();
	//printf("duration=%lf, time = %lf\n", endTime, 1000*1000*((double)(finish-start))/CLOCKS_PER_SEC/endTime);
	//fprintf(ofp, "duration=%lf, time = %lf\n", endTime, 1000*1000*((double)(finish-start))/CLOCKS_PER_SEC/endTime);
	//PrintList();
	for (int i = 0; i < nBucket; i++){
		free(bucketArray[i]);
		bucketArray[i] = NULL;
	}
	free(bucketArray);
	bucketArray = NULL;
}


