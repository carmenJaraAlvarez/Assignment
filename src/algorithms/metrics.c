/*
 * metrics.c
 *
 *  Created on: 21 dic. 2020
 *      Author: practica
 */

#include "metrics.h"

int init_clock()
{
	startwtime = MPI_Wtime();
	if(print_all)
	{
		printf("\nmetrics.c		init_clock()		%f",startwtime);
	}
	return 0;
}

double end_clock()
{
	  endwtime = MPI_Wtime();
		if(print_all)
		{
			printf("\nmetrics.c		end_clock()		%f", endwtime);
		}
	  double res=endwtime-startwtime;
	  if(1)
	  {
		  printf("\nmetrics.c end_clock()->TIME = %f\n", res);
	  }

	  return res;
}

int describe_logs(){


	  /*  Get event ID from MPE, user should NOT assign event ID  */
    MPE_Describe_state(event1a, event1b, "Send work", "red");
    MPE_Describe_state(event2a, event2b, "Receive work",   "blue");
    MPE_Describe_state(event3a, event3b, "Send best",    "green");
    MPE_Describe_state(event4a, event4b, "Receive best",      "orange");
    MPE_Describe_state(event5a, event5b, "Send resolved",    "white");
    MPE_Describe_state(event6a, event6b, "Receive resolved",      "purple");
    MPE_Describe_state(event7a, event7b, "...",    "yellow");
    MPE_Describe_state(event8a, event8b, "receive work petition",      "red");
    MPE_Describe_state(event9a, event9b, "serial",      "blue");

    MPE_Describe_event( event1, "Ask for work", "white" );
    MPE_Describe_event( event2, "Scan petition", "orange" );
    MPE_Describe_event( event3, "Receive petition", "blue" );
    MPE_Describe_event( event4, "Receive best", "green" );
    MPE_Describe_event( event5, "Broadcast", "pink" );
    MPE_Describe_event( event6, "prune", "purple" );
    MPE_Describe_event( event7, "confirmed", "yellow" );
    MPE_Describe_event( event8, "Timeout", "red" );
    MPE_Describe_event( event9, "Tuple_prune", "orange" );

    return 0;
}
