/* For CPU_ZERO and CPU_SET macros */
#define _GNU_SOURCE

/*****************************************************************************/

#include "ecrt.h"

/*****************************************************************************/

#include <string.h>
#include <stdio.h>
/* For setting the process's priority (setpriority) */
#include <sys/resource.h>
/* For pid_t and getpid() */
#include <unistd.h>
#include <sys/types.h>
/* For locking the program in RAM (mlockall) to prevent swapping */
#include <sys/mman.h>
/* clock_gettime, struct timespec, etc. */
#include <time.h>
/* Header for handling signals (definition of SIGINT) */
#include <signal.h>
/* For using real-time scheduling policy (FIFO) and sched_setaffinity */
#include <sched.h>
/* For using uint32_t format specifier, PRIu32 */
#include <inttypes.h>
/* For msgget and IPC_NOWAIT */
#include <sys/msg.h>

#include <errno.h>

/*****************************************************************************/
/* Uncomment to execute the motion loop for a predetermined number of cycles (= NUMBER_OF_CYCLES). */
#define LOG

#ifdef LOG

/* Assuming an update rate of exactly 1 ms, number of cycles for 24h = 24*3600*1000. */
#define NUMBER_OF_CYCLES 86400000

#endif

/* Comment to disable configuring PDOs (i.e. in case the PDO configuration saved in EEPROM is our 
   desired configuration.)
*/
#define CONFIG_PDOS

/* Comment to disable distributed clocks. */
#define DC

/* Comment to disable interprocess communication with queues. */
#define IPC

/* Choose the syncronization method: The reference clock can be either master's, or the reference slave's (slave 0 by default) */
#ifdef DC

/* Slave0's clock is the reference: no drift. Algorithm from rtai_rtdm_dc example. Work in progress.*/
/*#define SYNC_MASTER_TO_REF
/* Master's clock (CPU) is the reference: lower overhead. */
#define SYNC_REF_TO_MASTER

#endif

#ifdef DC

/* Comment to disable configuring slave's DC specification (shift time & cycle time) */
#define CONFIG_DC

/* Uncomment to enable performance measurement. */
/* Measure the difference in reference slave's clock timstamp each cycle, and print the result,
   which should be as close to cycleTime as possible. */
/* Note: Only works with DC enabled. */
#define MEASURE_PERF

#endif

/*****************************************************************************/

/* One motor revolution increments the encoder by 2^19 -1. */
#define ENCODER_RES 524287
/* The maximum stack size which is guranteed safe to access without faulting. */       
#define MAX_SAFE_STACK (8 * 1024) 

/* Calculate the time it took to complete the loop. */
//#define MEASURE_TIMING 

#define SET_CPU_AFFINITY

#define NSEC_PER_SEC (1000000000L)
#define FREQUENCY 1000
/* Period of motion loop, in nanoseconds */
#define PERIOD_NS (NSEC_PER_SEC / FREQUENCY)

#ifdef DC

#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)

#endif

#ifdef CONFIG_DC

/* SYNC0 event happens halfway through the cycle */
#define SHIFT0 (PERIOD_NS/2)

#endif

/*****************************************************************************/

void print_config(void)
{

printf("**********\n");

#ifdef LOG
printf("LOG. NUMBER_OF_CYCLES = %d\n", NUMBER_OF_CYCLES);
#endif

#ifdef CONFIG_PDOS
printf("CONFIG_PDOS\n");
#endif

#ifdef DC


printf("\nDC. ");

#ifdef SYNC_MASTER_TO_REF
printf("Mode: SYNC_MASTER_TO_REF\n");
#endif

#ifdef SYNC_REF_TO_MASTER
printf("Mode: SYNC_REF_TO_MASTER\n");
#endif

#ifdef CONFIG_DC 
printf("CONFIG_DC\n");
#endif

#ifdef MEASURE_PERF 
printf("MEASURE_PERF\n");
#endif


#endif

#ifdef IPC
printf("\nIPC\n");
#endif

#ifdef SET_CPU_AFFINITY
printf("SET_CPU_AFFINITY\n");
#endif


#ifdef MEASURE_TIMING
printf("MEASURE_TIMING\n");
#endif

#ifdef FREQUENCY
printf("FREQUENCY = %d\n", FREQUENCY);
#endif

printf("**********\n");



}


const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ";");
            tok && *tok;
            tok = strtok(NULL, ";\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}


/*
float motor0_points[] = { 50.62154919,  50.62137813,  50.62018069,  50.61693061,  50.61060191,  50.60016933,  50.58460876,  50.56289793,  50.53401722,  50.49695055,  50.45068645,  50.39456035,  50.32927654,  50.25588084,  50.17541779,  50.0889296,  49.9974552,  49.90202952,  49.80368285,  49.70344041,  49.60232207,  49.59220011,  49.48995549,  49.38276785,  49.26705232,  49.14019993,  49.00048699,  48.8469848,  48.67946978,  48.49833402,  48.304496,  48.09931117,  47.88448225,  47.66196894,  47.43389661,  47.20246391,  46.96984898,  46.73811417,  46.50910928,  46.28437328,  46.06503471,  45.85171099,  45.64440689,  45.44241236,  45.24420011,  45.13359654,  44.93248172,  44.71402765,  44.47582818,  44.21786575,  43.9401211,  43.64257312,  43.32519887,  42.98797346,  42.63087004,  42.25385971,  41.85691148,  41.43999225,  41.00306671,  40.54609737,  40.06904453,  39.57186622,  39.05451826,  38.51695426,  37.95912564,  37.38098503,  36.78687925,  36.18984927,  35.59230678,  34.99424918,  34.39567438,  33.79658092,  33.19696787,  32.59683492,  31.99618235,  31.39501102,  30.79332243,  30.19111867,  29.58840249,  28.98517725,  28.38144697,  27.77721632,  27.17249065,  26.56727596,  25.96157897,  25.35540706,  24.74876836,  24.14167168,  23.53412658,  22.92614337,  22.3177331,  21.7089076,  21.09967945,  20.49006207,  19.88006964,  19.26971717,  18.65902051,  18.04799635,  17.43666223,  16.82503655,  16.21313863,  15.60098865,  14.98860771,  14.37601785,  13.76324204,  13.15030421,  12.53722923,  11.92404299,  11.31077235,  10.6974452,  10.08409043,  9.470737988,  8.857418864,  8.244165119,  7.631009891,  7.017987412,  6.405133021,  5.792483176,  5.180075468,  4.567948633,  3.956142565,  3.344698325,  2.733658157,  2.123065495,  1.512964975,  0.903402446,  0.294424978,  -0.313919131,  -0.921580343,  -1.528507877,  -2.134649698,  -2.739952516,  -3.34436178,  -3.947821673,  -4.55027511,  -5.151663737,  -5.751927933,  -6.351006807,  -6.948838203,  -7.545358703,  -8.140503633,  -8.734207069,  -9.326401848,  -9.917019574,  -10.50599063,  -11.09324421,  -11.6787083,  -12.26230972,  -12.84397415,  -13.42362614,  -14.00118912,  -14.57658547,  -15.14973652,  -15.72056256,  -16.28898293,  -16.85491602,  -17.41827931,  -17.97898944,  -18.53696221,  -19.09211269,  -19.6443552,  -20.19360345,  -20.7397705,  -21.28276891,  -21.82251074,  -22.35890766,  -22.891871,  -23.4213118,  -23.94714094,  -24.46926918,  -24.98760724,  -25.5020659,  -26.0125561,  -26.51898897,  -27.02127601,  -27.5193291,  -28.01306065,  -28.50238367,  -28.9872119,  -29.46745987,  -29.94304304,  -30.41387788,  -30.87988199,  -31.34097418,  -31.79707463,  -32.24810491,  -32.69398817,  -33.13464919,  -33.5700145,  -34.00001249,  -34.42457348,  -34.84362987,  -35.25711619,  -35.66496918,  -36.06712795,  -36.463534,  -36.85413131,  -37.23886646,  -37.61768866,  -37.99054983,  -38.35740468,  -38.71821075,  -39.07292848,  -39.42152123,  -39.76395535,  -40.10020022,  -40.43022822,  -40.75401486,  -41.07153867,  -41.38278131,  -41.68772755,  -41.98636521,  -42.27868524,  -42.56468164,  -42.84435145,  -43.11769476,  -43.38471462,  -43.64541703,  -43.89981092,  -44.14790805,  -44.38972297,  -44.62527299,  -44.85457806,  -45.07766078,  -45.29454624,  -45.50526199,  -45.70983798,  -45.90830642,  -46.10070174,  -46.28706048,  -46.46742123,  -46.6418245,  -46.81031265,  -46.97292979,  -47.12972169,  -47.28073569,  -47.42602059,  -47.56562658,  -47.69960511,  -47.82800885,  -47.95089155,  -48.06830796,  -48.18031377,  -48.2869655,  -48.38832038,  -48.48443636,  -48.57537192,  -48.66118607,  -48.74193823,  -48.81768818,  -48.88849597,  -48.95442185,  -49.01552624,  -49.0718696,  -49.12351242,  -49.17051516,  -49.21293814,  -49.25084157,  -49.28428543,  -49.31332944,  -49.33803304,  -49.3584553,  -49.3746196,  -49.38640807,  -49.39409068,  -49.39809844,  -49.39883965,  -49.39669997,  -49.39204326,  -49.38521254,  -49.37653081,  -49.36630182,  -49.35481086,  -49.34232547,  -49.32909609,  -49.31535669,  -49.30132535,  -49.28720483,  -49.27318304,  -49.25943352,  -49.24611588,  -49.23337617,  -49.22125749,  -49.21845523,  -49.20495047,  -49.18393457,  -49.14958135,  -49.09739152,  -49.02408939,  -48.927517,  -48.80652439,  -48.66085592,  -48.49103354,  -48.29823803,  -48.08418975,  -47.85103039,  -47.60120716,  -47.33736042,  -47.06221579,  -46.77848102,  -46.48874797,  -46.19539947,  -45.90052078,  -45.60581498,  -45.31252185,  -45.02133931,  -44.73234678,  -44.57071044,  -44.28437146,  -44.00183743,  -43.72591421,  -43.45937347,  -43.20495582,  -42.96537483,  -42.74332161,  -42.54147015,  -42.36248319,  -42.20901881,  -42.08280677,  -41.98187216,  -41.90334273,  -41.84437466,  -41.80214572,  -41.7738495,  -41.75669055,  -41.74788047,  -41.74463486,  -41.74417122
};
float motor1_points[] = { -17.39188584,  -17.39229947,  -17.39519489,  -17.40305439,  -17.41836181,  -17.44360437,  -17.48127519,  -17.53387663,  -17.6039243,  -17.693952,  -17.80651745,  -17.94337633,  -18.10297919,  -18.28294863,  -18.4809034,  -18.69445292,  -18.9211923,  -19.15869784,  -19.40452312,  -19.6561956,  -19.9112139,  -19.9368034,  -20.19366797,  -20.45359724,  -20.71777492,  -20.98658311,  -21.25968895,  -21.53613285,  -21.81441878,  -22.09260736,  -22.36841204,  -22.63929906,  -22.90259116,  -23.15557533,  -23.39561426,  -23.6202613,  -23.827378,  -24.01525361,  -24.18272533,  -24.32929813,  -24.45526302,  -24.56181276,  -24.6511543,  -24.72661788,  -24.79276326,  -24.82799497,  -24.89158623,  -24.96023364,  -25.03457907,  -25.11449548,  -25.1998448,  -25.29047779,  -25.38623388,  -25.48694101,  -25.59241547,  -25.70246164,  -25.81687185,  -25.93542608,  -26.05789174,  -26.18402336,  -26.31356235,  -26.44623661,  -26.58176025,  -26.7198332,  -26.86014082,  -27.00235269,  -27.14507983,  -27.28501144,  -27.42153762,  -27.55463534,  -27.68428127,  -27.81045173,  -27.93312277,  -28.05227017,  -28.16786941,  -28.27989577,  -28.38832427,  -28.49312973,  -28.5942868,  -28.69176993,  -28.78555344,  -28.87561152,  -28.96191824,  -29.04444761,  -29.12317356,  -29.19806997,  -29.26911073,  -29.33626973,  -29.39952088,  -29.45883817,  -29.51419565,  -29.56556751,  -29.61292804,  -29.65625173,  -29.69551325,  -29.73068748,  -29.76174956,  -29.7886749,  -29.81143922,  -29.83001857,  -29.84438936,  -29.85452841,  -29.86041294,  -29.86202064,  -29.85932968,  -29.85231872,  -29.84096698,  -29.82525424,  -29.80516089,  -29.78066794,  -29.75175703,  -29.71841053,  -29.68061149,  -29.63834368,  -29.59159168,  -29.54034082,  -29.48457726,  -29.42428798,  -29.35946084,  -29.29008458,  -29.21614883,  -29.13764416,  -29.05456206,  -28.96689501,  -28.87463646,  -28.77778083,  -28.67632358,  -28.57026117,  -28.45959111,  -28.34431193,  -28.22442323,  -28.09992566,  -27.97082095,  -27.83711187,  -27.6988023,  -27.55589717,  -27.40840249,  -27.25632533,  -27.09967386,  -26.93845728,  -26.77268587,  -26.60237095,  -26.4275249,  -26.2481611,  -26.06429398,  -25.87593895,  -25.68311244,  -25.48583183,  -25.28411547,  -25.07798265,  -24.86745359,  -24.65254938,  -24.43329202,  -24.20970434,  -23.98181004,  -23.74963358,  -23.51320024,  -23.27253603,  -23.02766772,  -22.77862275,  -22.52542925,  -22.26811601,  -22.00671242,  -21.74124846,  -21.47175467,  -21.19826214,  -20.92080243,  -20.63940759,  -20.3541101,  -20.06494287,  -19.77193917,  -19.47513262,  -19.17455719,  -18.87024712,  -18.5622369,  -18.25056129,  -17.93525524,  -17.61635386,  -17.29389244,  -16.96790638,  -16.63843117,  -16.30550239,  -15.96915565,  -15.62942658,  -15.28635081,  -14.93996395,  -14.59030154,  -14.23739906,  -13.88129188,  -13.52201526,  -13.15960434,  -12.79409406,  -12.42551922,  -12.05391441,  -11.67931399,  -11.30175212,  -10.92126267,  -10.53787928,  -10.1516353,  -9.762563771,  -9.370697439,  -8.97606872,  -8.578709696,  -8.1786521,  -7.775927304,  -7.370566308,  -6.962599731,  -6.552057799,  -6.138970336,  -5.723366754,  -5.305276048,  -4.884726781,  -4.461747086,  -4.036364648,  -3.608606707,  -3.178500046,  -2.746070986,  -2.311345381,  -1.874348613,  -1.435105588,  -0.993640728,  -0.549977972,  -0.104140767,  0.343847931,  0.793965666,  1.246190479,  1.700500914,  2.15687602,  2.615295356,  3.075738989,  3.538187499,  4.00262198,  4.469024041,  4.937375811,  5.407659936,  5.879859585,  6.353958446,  6.829940735,  7.307791188,  7.787495072,  8.269038177,  8.752406825,  9.237587866,  9.724568682,  10.21333719,  10.70388183,  11.19619159,  11.69025598,  12.18606508,  12.68360946,  13.18288027,  13.68386919,  14.18656844,  14.6909708,  15.19706958,  15.70485867,  16.21433247,  16.72548598,  17.23831475,  17.75281486,  18.268983,  18.7868164,  19.30501023,  19.81278813,  20.30472836,  20.78063755,  21.24036506,  21.6837661,  22.11070161,  22.52103817,  22.91464791,  23.29140846,  23.65120286,  23.99391957,  24.3194524,  24.62770055,  24.91856854,  25.19196627,  25.44780902,  25.68601741,  25.90651753,  26.10924086,  26.29548199,  26.33770407,  26.51929168,  26.7047845,  26.89688977,  27.09731203,  27.30686165,  27.52556392,  27.75276866,  27.98726025,  28.22736757,  28.47107366,  28.71612452,  28.9601368,  29.20070408,  29.43550142,  29.66238829,  29.87950978,  30.08539609,  30.27906063,  30.46009686,  30.62877406,  30.78613241,  30.93407758,  31.07547513,  31.15348228,  31.29178566,  31.42867875,  31.56277922,  31.69270542,  31.81707617,  31.93451064,  32.04362808,  32.14304773,  32.23138869,  32.30726978,  32.36977074,  32.41981586,  32.45879006,  32.48807778,  32.50906318,  32.52313018,  32.53166257,  32.53604406,  32.53765829,  32.53788889
};
*/
float *motor0_points;
float *motor1_points;
float *motor2_points;
float *motor3_points;
float *motor4_points;
float *motor5_points;




int numberofpoint = 0;
int read_trajectory_from_csv()
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
	fp = fopen("point.csv", "r");
    if (fp == NULL){
		perror("Error");
        exit(EXIT_FAILURE);
		
	}

    int numberofline = 0 ;
    //read line by line for find out number of line in trajectory 
    while ((read = getline(&line, &len, fp)) != -1) {
        numberofline++;
    }
	printf("nu;mberof line %d" , numberofline);

	
    rewind(fp); // go to first line of file 
    motor0_points = malloc(numberofline * sizeof(float));
    motor1_points = malloc(numberofline * sizeof(float));
	motor2_points = malloc(numberofline * sizeof(float));
	motor3_points = malloc(numberofline * sizeof(float));
//	motor4_points = malloc(numberofline * sizeof(float));

	int h = 0;
    while ((read = getline(&line, &len, fp)) != -1) {
        
        //printf("csvline length:%zu\n", read);
        printf("%s", line);
        
        char *pt;
        pt = strtok (line,",");
		int i = 0;
        while (pt != NULL) {
           // printf(" %c \n " , pt[0] );
            float a = atof(pt);
			if(i == 0 ){
				motor0_points[h] = a;
				printf("this is read point %f \n " , motor0_points[h] );
			}
			if(i==1){
				motor1_points[h] = a;
			}
			if(i==2){
				motor2_points[h] = a;
			}
			if(i==3){
				motor3_points[h] = a;
			}
			i++;

            pt = strtok (NULL, ",");

       }

		h++;
    }

    fclose(fp);
    if (line)
        free(line);
 //   exit(EXIT_SUCCESS);
 	numberofpoint = numberofline;
}







#define NUMBER_OF_motorpuls 524288


int32_t degree_to_motor_position(float point) {
	return (int32_t)(point * NUMBER_OF_motorpuls / 360);
}
float motor_to_degree_position(int32_t point) {
	return (float)((float)point * (float)360 /  (float)NUMBER_OF_motorpuls);
}


/*****************************************************************************/
/* Note: Anything relying on definition of SYNC_MASTER_TO_REF is essentially copy-pasted from /rtdm_rtai_dc/main.c */

#ifdef SYNC_MASTER_TO_REF

/* First used in system_time_ns() */
static int64_t  system_time_base = 0LL;
/* First used in sync_distributed_clocks() */
static uint64_t dc_time_ns = 0;
static int32_t  prev_dc_diff_ns = 0;
/* First used in update_master_clock() */
static int32_t  dc_diff_ns = 0;
static unsigned int cycle_ns = PERIOD_NS;
static uint8_t  dc_started = 0;
static int64_t  dc_diff_total_ns = 0LL;
static int64_t  dc_delta_total_ns = 0LL;
static int      dc_filter_idx = 0;
static int64_t  dc_adjust_ns;
#define DC_FILTER_CNT          1024
/** Return the sign of a number
 *
 * ie -1 for -ve value, 0 for 0, +1 for +ve value
 *
 * \retval the sign of the value
 */
#define sign(val) \
    ({ typeof (val) _val = (val); \
    ((_val > 0) - (_val < 0)); })

static uint64_t dc_start_time_ns = 0LL;

#endif

ec_master_t* master;

/*****************************************************************************/

#ifdef SYNC_MASTER_TO_REF

/** Get the time in ns for the current cpu, adjusted by system_time_base.
 *
 * \attention Rather than calling rt_get_time_ns() directly, all application
 * time calls should use this method instead.
 *
 * \ret The time in ns.
 */
uint64_t system_time_ns(void)
{
	struct timespec time;
	int64_t time_ns;
	clock_gettime(CLOCK_MONOTONIC, &time);
	time_ns = TIMESPEC2NS(time);

	if (system_time_base > time_ns) 
	{
		printf("%s() error: system_time_base greater than"
		       " system time (system_time_base: %ld, time: %lu\n",
			__func__, system_time_base, time_ns);
		return time_ns;
	}
	else 
	{
		return time_ns - system_time_base;
	}
}


/** Synchronise the distributed clocks
 */

/* If SYNC_MASTER_TO_REF and MEASURE_PERF are both defined. */
#ifdef MEASURE_PERF
void sync_distributed_clocks(uint32_t* t_cur)
/* If only SYNC_MASTER_TO_REF is defined. */
#else
void sync_distributed_clocks(void)	
#endif
{

	uint32_t ref_time = 0;
	uint64_t prev_app_time = dc_time_ns;

	dc_time_ns = system_time_ns();

	// set master time in nano-seconds
	ecrt_master_application_time(master, dc_time_ns);

	// get reference clock time to synchronize master cycle
	ecrt_master_reference_clock_time(master, &ref_time);
	#ifdef MEASURE_PERF
	*t_cur = ref_time;
	#endif
	dc_diff_ns = (uint32_t) prev_app_time - ref_time;

	// call to sync slaves to ref slave
	ecrt_master_sync_slave_clocks(master);
}


/** Update the master time based on ref slaves time diff
 *
 * called after the ethercat frame is sent to avoid time jitter in
 * sync_distributed_clocks()
 */
void update_master_clock(void)
{

	// calc drift (via un-normalised time diff)
	int32_t delta = dc_diff_ns - prev_dc_diff_ns;
	//printf("%d\n", (int) delta);
	prev_dc_diff_ns = dc_diff_ns;

	// normalise the time diff
	dc_diff_ns = ((dc_diff_ns + (cycle_ns / 2)) % cycle_ns) - (cycle_ns / 2);
        
	// only update if primary master
	if (dc_started) 
	{

		// add to totals
		dc_diff_total_ns += dc_diff_ns;
		dc_delta_total_ns += delta;
		dc_filter_idx++;

		if (dc_filter_idx >= DC_FILTER_CNT) 
		{
			// add rounded delta average
			dc_adjust_ns += ((dc_delta_total_ns + (DC_FILTER_CNT / 2)) / DC_FILTER_CNT);
                
			// and add adjustment for general diff (to pull in drift)
			dc_adjust_ns += sign(dc_diff_total_ns / DC_FILTER_CNT);

			// limit crazy numbers (0.1% of std cycle time)
			if (dc_adjust_ns < -1000) 
			{
				dc_adjust_ns = -1000;
			}
			if (dc_adjust_ns > 1000) 
			{
				dc_adjust_ns =  1000;
			}
		
			// reset
			dc_diff_total_ns = 0LL;
			dc_delta_total_ns = 0LL;
			dc_filter_idx = 0;
		}

		// add cycles adjustment to time base (including a spot adjustment)
		system_time_base += dc_adjust_ns + sign(dc_diff_ns);
	}
	else 
	{
		dc_started = (dc_diff_ns != 0);

		if (dc_started) 
		{
			// output first diff
			printf("First master diff: %d.\n", dc_diff_ns);

			// record the time of this initial cycle
			dc_start_time_ns = dc_time_ns;
		}
	}
}

#endif

/*****************************************************************************/

void ODwrite(ec_master_t* master, uint16_t slavePos, uint16_t index, uint8_t subIndex, uint8_t objectValue)
{
	/* Blocks until a reponse is received */
	uint8_t retVal = ecrt_master_sdo_download(master, slavePos, index, subIndex, &objectValue, sizeof(objectValue), NULL);
	/* retVal != 0: Failure */
	if (retVal)
		printf("OD write unsuccessful\n");
}

void initDrive(ec_master_t* master, uint16_t slavePos)
{
	/* Reset alarm */
	ODwrite(master, slavePos, 0x6040, 0x00, 128);
	/* Servo on and operational */
	ODwrite(master, slavePos, 0x6040, 0x00, 0xF);
	/* Mode of operation, CSP */
	ODwrite(master, slavePos, 0x6060, 0x00, 0x8);
	/* velocity */
	//ODwrite(master, slavePos, 0x606B, 0x00, 41000000000);
	//ODwrite(master, slavePos, 0x606B, 0x01, 5000);
//	ODwrite(master, slavePos, 0x606B, 0x02, 5000);

}

/*****************************************************************************/

/* Add two timespec structures (time1 and time2), store the the result in result. */
/* result = time1 + time2 */
inline void timespec_add(struct timespec* result, struct timespec* time1, struct timespec* time2)
{

	if ((time1->tv_nsec + time2->tv_nsec) >= NSEC_PER_SEC) 
	{
		result->tv_sec  = time1->tv_sec + time2->tv_sec + 1;
		result->tv_nsec = time1->tv_nsec + time2->tv_nsec - NSEC_PER_SEC;
	} 
	else 
	{
		result->tv_sec  = time1->tv_sec + time2->tv_sec;
		result->tv_nsec = time1->tv_nsec + time2->tv_nsec;
	}

}

#ifdef MEASURE_TIMING
/* Substract two timespec structures (time1 and time2), store the the result in result. */
/* result = time1 - time2 */
inline void timespec_sub(struct timespec* result, struct timespec* time1, struct timespec* time2)
{

	if ((time1->tv_nsec - time2->tv_nsec) < 0) 
	{
		result->tv_sec  = time1->tv_sec - time2->tv_sec - 1;
		result->tv_nsec = NSEC_PER_SEC - (time1->tv_nsec - time2->tv_nsec);
	} 
	else 
	{
		result->tv_sec  = time1->tv_sec - time2->tv_sec;
		result->tv_nsec = time1->tv_nsec - time2->tv_nsec;
	}

}
#endif

/*****************************************************************************/

/* We have to pass "master" to ecrt_release_master in signal_handler, but it is not possible
   to define one with more than one argument. Therefore, master should be a global variable. 
*/
void signal_handler(int sig)
{
	printf("\nReleasing master...\n");
	ecrt_release_master(master);
	pid_t pid = getpid();
	kill(pid, SIGKILL);
}

/*****************************************************************************/

/* We make sure 8kB (maximum stack size) is allocated and locked by mlockall(MCL_CURRENT | MCL_FUTURE). */
void stack_prefault(void)
{
    unsigned char dummy[MAX_SAFE_STACK];
    memset(dummy, 0, MAX_SAFE_STACK);
}

/*****************************************************************************/

int main(int argc, char **argv)
{

		read_trajectory_from_csv();
	   



	print_config();
	
	#ifdef SET_CPU_AFFINITY
	cpu_set_t set;
	/* Clear set, so that it contains no CPUs. */
	CPU_ZERO(&set);
	/* Add CPU (core) 1 to the CPU set. */
	CPU_SET(1, &set);
	#endif
	
	/* 0 for the first argument means set the affinity of the current process. */
	/* Returns 0 on success. */
	if (sched_setaffinity(0, sizeof(set), &set))
	{
		printf("Setting CPU affinity failed!\n");
		return -1;
	}
	
	/* SCHED_FIFO tasks are allowed to run until they have completed their work or voluntarily yield. */
	/* Note that even the lowest priority realtime thread will be scheduled ahead of any thread with a non-realtime policy; 
	   if only one realtime thread exists, the SCHED_FIFO priority value does not matter.
	*/  
	struct sched_param param = {};
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	printf("Using priority %i.\n", param.sched_priority);
	if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) 
	{
		printf("sched_setscheduler failed\n");
	}
	
	/* Lock the program into RAM to prevent page faults and swapping */
	/* MCL_CURRENT: Lock in all current pages.
	   MCL_FUTURE:  Lock in pages for heap and stack and shared memory.
	*/
	if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
	{
		printf("mlockall failed\n");
		return -1;
	}
	
	/* Allocate the entire stack, locked by mlockall(MCL_FUTURE). */
	stack_prefault();
	
	/* Register the signal handler function. */
	signal(SIGINT, signal_handler);
	
	/***************************************************/
	#ifdef IPC
	
	int qID;
	/* key is specified by the process which creates the queue (receiver). */
	key_t qKey = 1234;
	
	/* When qFlag is zero, msgget obtains the identifier of a previously created message queue. */
	int qFlag = 0;
	
	/* msgget returns the System V message queue identifier associated with the value of the key argument. */
	if ((qID = msgget(qKey, qFlag)) < 0) 
	{
		printf("Failed to access the queue with key = %d : %s\n", qKey, strerror(errno));
		return -1;
	}
	
	
	typedef struct myMsgType 
	{
		/* Mandatory, must be a positive number. */
		long       mtype;
		/* Data */
		#ifdef MEASURE_PERF
		long       updatePeriod;
		#endif
		int32_t    actPos[4];
		int32_t    targetPos[4];
       	} myMsg;
	
	myMsg msg;
	
	size_t msgSize;
	/* size of data = size of structure - size of mtype */
	msgSize = sizeof(struct myMsgType) - sizeof(long);
	    
	/* mtype must be a positive number. The receiver picks messages with a specific mtype.*/
	msg.mtype = 1;
	
	#endif
	/***************************************************/
	
	/* Reserve the first master (0) (/etc/init.d/ethercat start) for this program */
	master = ecrt_request_master(0);
	if (!master)
		printf("Requesting master failed\n");
	
	initDrive(master, 0);
	initDrive(master, 1);
	initDrive(master, 2);
	initDrive(master, 3);
	
	uint16_t alias = 0;
	uint16_t position0 = 0;
	uint16_t position1 = 1;
	uint16_t position2 = 2;
	uint16_t position3 = 3;

	uint32_t vendor_id = 0x00007595;
	uint32_t product_code = 0x00000000;
	
	/* Creates and returns a slave configuration object, ec_slave_config_t*, for the given alias and position. */
	/* Returns NULL (0) in case of error and pointer to the configuration struct otherwise */
	ec_slave_config_t* drive0 = ecrt_master_slave_config(master, alias, position0, vendor_id, product_code);
	ec_slave_config_t* drive1 = ecrt_master_slave_config(master, alias, position1, vendor_id, product_code);
	
	ec_slave_config_t* drive2 = ecrt_master_slave_config(master, alias, position2, vendor_id, product_code);
	ec_slave_config_t* drive3 = ecrt_master_slave_config(master, alias, position3, vendor_id, product_code);
	
	
	/* If the drive0 = NULL or drive1 = NULL */
	if (!drive0 || !drive1 || !drive2 || !drive3)
	{
		printf("Failed to get slave configuration\n");
		return -1;
	}
	
	
	/***************************************************/
	#ifdef CONFIG_PDOS
	
	/* Slave 0's structures, obtained from $ethercat cstruct -p 0 */ 
	ec_pdo_entry_info_t slave_0_pdo_entries[] = 
	{
	{0x6040, 0x00, 16}, /* Controlword */
	{0x607a, 0x00, 32}, /* Target Position */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
	};
	
	ec_pdo_info_t slave_0_pdos[] =
	{
	{0x1601, 2, slave_0_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
	{0x1a01, 2, slave_0_pdo_entries + 2}, /* 2nd Transmit PDO Mapping */
	};
	
	ec_sync_info_t slave_0_syncs[] =
	{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_0_pdos + 0, EC_WD_DISABLE},
	{3, EC_DIR_INPUT , 1, slave_0_pdos + 1, EC_WD_DISABLE},
	{0xFF}
	};
	
	/* Slave 1's structures, obtained from $ethercat cstruct -p 1 */ 
	ec_pdo_entry_info_t slave_1_pdo_entries[] = 
	{
	{0x6040, 0x00, 16}, /* Controlword */
	{0x607a, 0x00, 32}, /* Target Position */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
	};
	
	ec_pdo_info_t slave_1_pdos[] =
	{
	{0x1601, 2, slave_1_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
	{0x1a01, 2, slave_1_pdo_entries + 2}, /* 2nd Transmit PDO Mapping */
	};
	
	ec_sync_info_t slave_1_syncs[] =
	{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_1_pdos + 0, EC_WD_DISABLE},
	{3, EC_DIR_INPUT , 1, slave_1_pdos + 1, EC_WD_DISABLE},
	{0xFF}
	};
	
	


	/* Slave 2's structures, obtained from $ethercat cstruct -p 1 */ 
	ec_pdo_entry_info_t slave_2_pdo_entries[] = 
	{
	{0x6040, 0x00, 16}, /* Controlword */
	{0x607a, 0x00, 32}, /* Target Position */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
	};
	
	ec_pdo_info_t slave_2_pdos[] =
	{
	{0x1601, 2, slave_2_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
	{0x1a01, 2, slave_2_pdo_entries + 2}, /* 2nd Transmit PDO Mapping */
	};
	
	ec_sync_info_t slave_2_syncs[] =
	{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_2_pdos + 0, EC_WD_DISABLE},
	{3, EC_DIR_INPUT , 1, slave_2_pdos + 1, EC_WD_DISABLE},
	{0xFF}
	};
	
	/* Slave 3's structures, obtained from $ethercat cstruct -p 1 */ 
	ec_pdo_entry_info_t slave_3_pdo_entries[] = 
	{
	{0x6040, 0x00, 16}, /* Controlword */
	{0x607a, 0x00, 32}, /* Target Position */
	{0x6041, 0x00, 16}, /* Statusword */
	{0x6064, 0x00, 32}, /* Position Actual Value */
	};
	
	ec_pdo_info_t slave_3_pdos[] =
	{
	{0x1601, 2, slave_3_pdo_entries + 0}, /* 2nd Receive PDO Mapping */
	{0x1a01, 2, slave_3_pdo_entries + 2}, /* 2nd Transmit PDO Mapping */
	};
	
	ec_sync_info_t slave_3_syncs[] =
	{
	{0, EC_DIR_OUTPUT, 0, NULL            , EC_WD_DISABLE},
	{1, EC_DIR_INPUT , 0, NULL            , EC_WD_DISABLE},
	{2, EC_DIR_OUTPUT, 1, slave_3_pdos + 0, EC_WD_DISABLE},
	{3, EC_DIR_INPUT , 1, slave_3_pdos + 1, EC_WD_DISABLE},
	{0xFF}
	};
	

	
	if (ecrt_slave_config_pdos(drive0, EC_END, slave_0_syncs))
	{
		printf("Failed to configure slave 0 PDOs\n");
		return -1;
	}
	
	if (ecrt_slave_config_pdos(drive1, EC_END, slave_1_syncs))
	{
		printf("Failed to configure slave 1 PDOs\n");
		return -1;
	}
	
	if (ecrt_slave_config_pdos(drive2, EC_END, slave_2_syncs))
	{
		printf("Failed to configure slave 2 PDOs\n");
		return -1;
	}

	if (ecrt_slave_config_pdos(drive3, EC_END, slave_3_syncs))
	{
		printf("Failed to configure slave 3 PDOs\n");
		return -1;
	}
	#endif
	/***************************************************/

	unsigned int offset_controlWord0, offset_targetPos0, offset_statusWord0, offset_actPos0;
	unsigned int offset_controlWord1, offset_targetPos1, offset_statusWord1, offset_actPos1;
	
	unsigned int offset_controlWord2, offset_targetPos2, offset_statusWord2, offset_actPos2;
	unsigned int offset_controlWord3, offset_targetPos3, offset_statusWord3, offset_actPos3;


	
	ec_pdo_entry_reg_t domain1_regs[] =
	{
	{0, 0, 0x00007595, 0x00000000, 0x6040, 0x00, &offset_controlWord0},
	{0, 0, 0x00007595, 0x00000000, 0x607a, 0x00, &offset_targetPos0  },
	{0, 0, 0x00007595, 0x00000000, 0x6041, 0x00, &offset_statusWord0 },
	{0, 0, 0x00007595, 0x00000000, 0x6064, 0x00, &offset_actPos0     },
	
	{0, 1, 0x00007595, 0x00000000, 0x6040, 0x00, &offset_controlWord1},
	{0, 1, 0x00007595, 0x00000000, 0x607a, 0x00, &offset_targetPos1  },
	{0, 1, 0x00007595, 0x00000000, 0x6041, 0x00, &offset_statusWord1 },
	{0, 1, 0x00007595, 0x00000000, 0x6064, 0x00, &offset_actPos1     },

	{0, 2, 0x00007595, 0x00000000, 0x6040, 0x00, &offset_controlWord2},
	{0, 2, 0x00007595, 0x00000000, 0x607a, 0x00, &offset_targetPos2  },
	{0, 2, 0x00007595, 0x00000000, 0x6041, 0x00, &offset_statusWord2 },
	{0, 2, 0x00007595, 0x00000000, 0x6064, 0x00, &offset_actPos2     },

	{0, 3, 0x00007595, 0x00000000, 0x6040, 0x00, &offset_controlWord3},
	{0, 3, 0x00007595, 0x00000000, 0x607a, 0x00, &offset_targetPos3  },
	{0, 3, 0x00007595, 0x00000000, 0x6041, 0x00, &offset_statusWord3 },
	{0, 3, 0x00007595, 0x00000000, 0x6064, 0x00, &offset_actPos3     },
	{}
	};
	
	/* Creates a new process data domain. */
	/* For process data exchange, at least one process data domain is needed. */
	ec_domain_t* domain1 = ecrt_master_create_domain(master);
	
	/* Registers PDOs for a domain. */
	/* Returns 0 on success. */
	if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs))
	{
		printf("PDO entry registration failed\n");
		return -1;
	}
	
	#ifdef CONFIG_DC
	/* Do not enable Sync1 */
	ecrt_slave_config_dc(drive0, 0x0300, PERIOD_NS, SHIFT0, 0, 0);
	ecrt_slave_config_dc(drive1, 0x0300, PERIOD_NS, SHIFT0, 0, 0);

	ecrt_slave_config_dc(drive2, 0x0300, PERIOD_NS, SHIFT0, 0, 0);
	ecrt_slave_config_dc(drive3, 0x0300, PERIOD_NS, SHIFT0, 0, 0);

	#endif
	
	#ifdef SYNC_REF_TO_MASTER
	/* Initialize master application time. */
	struct timespec masterInitTime;
	clock_gettime(CLOCK_MONOTONIC, &masterInitTime);
	ecrt_master_application_time(master, TIMESPEC2NS(masterInitTime));
	#endif
	
	#ifdef SYNC_MASTER_TO_REF
	/* Initialize master application time. */
	dc_start_time_ns = system_time_ns();
	dc_time_ns = dc_start_time_ns;
	ecrt_master_application_time(master, dc_start_time_ns);
	
	/* If this method is not called for a certain master, then the first slave with DC functionality
	   will provide the reference clock.
	*/
	/* But we call this function anyway, just for emphasis. */
	if (ecrt_master_select_reference_clock(master, drive0))
	{
		printf("Selecting slave 0 as reference clock failed!\n");
		return -1;
	}
	#endif
	
	
	/* Up to this point, we have only requested the master. See log messages */
	printf("Activating master...\n");
	/* Important points from ecrt.h 
	   - This function tells the master that the configuration phase is finished and
	     the real-time operation will begin. 
	   - It tells the master state machine that the bus configuration is now to be applied.
	   - By calling the ecrt master activate() method, all slaves are configured according to
             the prior method calls and are brought into OP state.
	   - After this function has been called, the real-time application is in charge of cylically
	     calling ecrt_master_send() and ecrt_master_receive(). Before calling this function, the 
	     master thread is responsible for that. 
	   - This method allocated memory and should not be called in real-time context.
	*/
	     
	if (ecrt_master_activate(master))
		return -1;

	
	uint8_t* domain1_pd;
	/* Returns a pointer to (I think) the first byte of PDO data of the domain */
	if (!(domain1_pd = ecrt_domain_data(domain1)))
		return -1;
	
	ec_slave_config_state_t slaveState0;
	ec_slave_config_state_t slaveState1;

	ec_slave_config_state_t slaveState2;
	ec_slave_config_state_t slaveState3;

	struct timespec wakeupTime;
	
	#ifdef SYNC_REF_TO_MASTER
	struct timespec	time;
	#endif

	#ifdef MEASURE_PERF
	/* The slave time received in the current and the previous cycle */
	uint32_t t_cur, t_prev;
	#endif

	
	/* Return value of msgsnd. */
	int retVal;
	
	struct timespec cycleTime = {0, PERIOD_NS};
	struct timespec recvTime = {0, PERIOD_NS/50};
	
	clock_gettime(CLOCK_MONOTONIC, &wakeupTime);

	
	
	/* The slaves (drives) enter OP mode after exchanging a few frames. */
	/* We exchange frames with no RPDOs (targetPos) untill all slaves have 
	   reached OP state, and then we break out of the loop.
	*/
	while (1)
	{
		
		timespec_add(&wakeupTime, &wakeupTime, &cycleTime);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);
		
		ecrt_master_receive(master);
		ecrt_domain_process(domain1);
		
		ecrt_slave_config_state(drive0, &slaveState0);
		ecrt_slave_config_state(drive1, &slaveState1);
		ecrt_slave_config_state(drive2, &slaveState2);
		ecrt_slave_config_state(drive3, &slaveState3);

		if (slaveState0.operational && slaveState1.operational && slaveState2.operational && slaveState3.operational)
		{
			printf("All slaves have reached OP state\n");
			break;
		}
	
		ecrt_domain_queue(domain1);
		

		#ifdef SYNC_REF_TO_MASTER
		/* Syncing reference slave to master:
                   1- The master's (PC) clock is the reference.
		   2- Sync the reference slave's clock to the master's.
		   3- Sync the other slave clocks to the reference slave's.
		*/
		
		clock_gettime(CLOCK_MONOTONIC, &time);
		ecrt_master_application_time(master, TIMESPEC2NS(time));
		/* Queues the DC reference clock drift compensation datagram for sending.
		   The reference clock will by synchronized to the **application (PC)** time provided
		   by the last call off ecrt_master_application_time().
		*/
		ecrt_master_sync_reference_clock(master);
		/* Queues the DC clock drift compensation datagram for sending.
		   All slave clocks will be synchronized to the reference slave clock.
		*/
		ecrt_master_sync_slave_clocks(master);
		#endif
		
		#ifdef SYNC_MASTER_TO_REF
		
		// sync distributed clock just before master_send to set
     	        // most accurate master clock time
		#ifdef MEASURE_PERF
                sync_distributed_clocks(&t_cur);
		#else
		sync_distributed_clocks();
		#endif
		
		#endif
		
		
		ecrt_master_send(master);
		
		#ifdef SYNC_MASTER_TO_REF
		// update the master clock
     		// Note: called after ecrt_master_send() to reduce time
                // jitter in the sync_distributed_clocks() call
                update_master_clock();
		#endif
	
	}
	
	int32_t actPos0, targetPos0;
	int32_t actPos1, targetPos1;

	int32_t actPos2, targetPos2;
	int32_t actPos3, targetPos3;
	
	/* Sleep is how long we should sleep each loop to keep the cycle's frequency as close to cycleTime as possible. */ 
	struct timespec sleepTime;
	#ifdef MEASURE_TIMING 
	struct timespec execTime, endTime;
	#endif 	
	
	#ifdef LOG
	/* Cycle number. */
	int i = 0;
	#endif
	int test =100;
	int counter =   0 ;
	/* Wake up 1 msec after the start of the previous loop. */
	sleepTime = cycleTime;
	/* Update wakeupTime = current time */
	clock_gettime(CLOCK_MONOTONIC, &wakeupTime);
	int position_number = 0;
	int rev_position_number = sizeof(motor0_points) / sizeof(motor0_points[0]) - 1 ;
	#ifdef LOG
	while (i != NUMBER_OF_CYCLES)
	#else
	while (1)
	#endif
	{
		#ifdef MEASURE_TIMING
		clock_gettime(CLOCK_MONOTONIC, &endTime);
		/* wakeupTime is also start time of the loop. */
		/* execTime = endTime - wakeupTime */
		timespec_sub(&execTime, &endTime, &wakeupTime);
		printf("Execution time: %lu ns\n", execTime.tv_nsec);
		#endif

		clock_nanosleep(CLOCK_MONOTONIC, 0, &recvTime, NULL);
		

		/* Fetches received frames from the newtork device and processes the datagrams. */
		ecrt_master_receive(master);
		/* Evaluates the working counters of the received datagrams and outputs statistics,
		   if necessary.
		   This function is NOT essential to the receive/process/send procedure and can be 
		   commented out 
		*/
		ecrt_domain_process(domain1);
		
		
		#if defined(MEASURE_PERF) && defined(SYNC_REF_TO_MASTER)
		ecrt_master_reference_clock_time(master, &t_cur);	
		#endif
		
		/********************************************************************************/
		
		/* Read PDOs from the datagram */
		actPos0 = EC_READ_S32(domain1_pd + offset_actPos0);
		actPos1 = EC_READ_S32(domain1_pd + offset_actPos1);
		
		actPos2 = EC_READ_S32(domain1_pd + offset_actPos2);
		actPos3 = EC_READ_S32(domain1_pd + offset_actPos3);

		/*
			calculate the position 
		*/

		
		
		//targetPos0 = degree_to_motor_position(motor0_points[position_number]);
		//targetPos0 =motor1_points[position_number];	
		//position_number++;
		//printf("actpos %ld \n" , actPos0);
		/* Process the received data 524288*/
		if(counter <= 0){
			counter = counter + 1 ;
			targetPos0 = 0 ;
		}else {
		//if(position_number < sizeof(motor0_points) / sizeof(motor0_points[0])){
			if(position_number < numberofpoint){
				//position_number = position_number %  (sizeof(motor0_points) / sizeof(motor0_points[0]));
				//printf("pos value : %d \t actpos0 %ld\t dpos: %ld\n" , position_number , actPos0 , degree_to_motor_position(motor0_points[position_number]));
				targetPos0 = degree_to_motor_position(motor0_points[position_number]);
				targetPos1 = degree_to_motor_position(motor1_points[position_number]);
				targetPos2 = degree_to_motor_position(motor0_points[position_number]);
				targetPos3 = degree_to_motor_position(motor0_points[position_number]);
				position_number++;
				if(position_number == sizeof(motor0_points) / sizeof(motor0_points[0] )){
					rev_position_number =  sizeof(motor0_points) / sizeof(motor0_points[0]) - 1 ; 
				}
			}else{
				//position_number = position_number %  (sizeof(motor0_points) / sizeof(motor0_points[0]));
				//printf("rev value : %d \t actpos0 %ld\t dpos: %ld\n" , rev_position_number , actPos0 , degree_to_motor_position(motor0_points[rev_position_number]));
				targetPos0 = degree_to_motor_position(motor0_points[rev_position_number]);
				targetPos1 = degree_to_motor_position(motor1_points[rev_position_number]);
				targetPos2 = degree_to_motor_position(motor0_points[rev_position_number]);
				targetPos3 = degree_to_motor_position(motor0_points[rev_position_number]);
				rev_position_number -- ;
				if(rev_position_number < 0 ){
					position_number = 0 ;
				}
				
			}
			
			
		}	
		
		//targetPos0= 0 ;
		//test = 0;
		//targetPos0 = degree_to_motor_position();

			//printf("pos val: %f\n" ,motor_to_degree_position(targetPos0));
			printf("act pos: %f\n" ,motor_to_degree_position(actPos0));
			printf("act pos1: %f\n" ,motor_to_degree_position(actPos1));
			printf("act pos2: %f\n" ,motor_to_degree_position(actPos2));
			printf("act pos3: %f\n" ,motor_to_degree_position(actPos3));
		/*
		if(actPos1 > -82400 && actPos1 < 82400)
			targetPos1 = (actPos1  + 52400 ) ; // / NUMBER_OF_motorpuls )  * NUMBER_OF_motorpuls - degree_to_motor_position(motor1_points[position_number])
		else
			targetPos1 = (actPos1) ;
		if(actPos2 > -82400 && actPos2 < 82400)
			targetPos2 = (actPos2  - 52400 ) ; // / NUMBER_OF_motorpuls )  * NUMBER_OF_motorpuls - degree_to_motor_position(motor1_points[position_number]);
		else
			targetPos2 = (actPos2) ;
		if(actPos3 > -82400 && actPos3 < 82400)
			targetPos3 = (actPos3  + 52400 ) ; // / NUMBER_OF_motorpuls )  * NUMBER_OF_motorpuls - degree_to_motor_position(motor1_points[position_number]);
		else
			targetPos3 = (actPos3) ;
		*/

		
		 //targetPos1 = (actPos1  + 524000 * 3) ; // / NUMBER_OF_motorpuls )  * NUMBER_OF_motorpuls - degree_to_motor_position(motor1_points[position_number])
		// targetPos2 = (actPos2  + 120000 * 10   ) ; // / NUMBER_OF_motorpuls )  * NUMBER_OF_motorpuls - degree_to_motor_position(motor1_points[position_number]);
		// targetPos3 = (actPos3  + 120000  ) ; // / NUMBER_OF_motorpuls )  * NUMBER_OF_motorpuls - degree_to_motor_position(motor1_points[position_number]);


		//position_number++;
		/* Write PDOs to the datagram */
		EC_WRITE_U8  (domain1_pd + offset_controlWord0, 0xF );
		EC_WRITE_S32 (domain1_pd + offset_targetPos0  , targetPos0);
		
		EC_WRITE_U8  (domain1_pd + offset_controlWord1, 0xF );
		EC_WRITE_S32 (domain1_pd + offset_targetPos1  , targetPos1);

		EC_WRITE_U8  (domain1_pd + offset_controlWord2, 0xF );
		EC_WRITE_S32 (domain1_pd + offset_targetPos2  , targetPos2);

		EC_WRITE_U8  (domain1_pd + offset_controlWord3, 0xF );
		EC_WRITE_S32 (domain1_pd + offset_targetPos3  , targetPos3);
		
		/********************************************************************************/
		
		/* Queues all domain datagrams in the master's datagram queue. 
		   Call this function to mark the domain's datagrams for exchanging at the
		   next call of ecrt_master_send() 
		*/
		ecrt_domain_queue(domain1);
		
		#ifdef SYNC_REF_TO_MASTER
		/* Distributed clocks */
		clock_gettime(CLOCK_MONOTONIC, &time);
		ecrt_master_application_time(master, TIMESPEC2NS(time));
		ecrt_master_sync_reference_clock(master);
		ecrt_master_sync_slave_clocks(master);
		#endif
		
		#ifdef SYNC_MASTER_TO_REF
		
		// sync distributed clock just before master_send to set
     	        // most accurate master clock time
		#ifdef MEASURE_PERF
                sync_distributed_clocks(&t_cur);
		#else
		sync_distributed_clocks();
		#endif
		
		#endif

		/* wakeupTime = wakeupTime + sleepTime */
		timespec_add(&wakeupTime, &wakeupTime, &sleepTime);
		/* Sleep to adjust the update frequency */
		/* Note: TIMER_ABSTIME flag is key in ensuring the execution with the desired frequency.
		   We don't have to conider the loop's execution time (as long as it doesn't get too close to 1 ms), 
		   as the sleep ends cycleTime (=1 msecs) *after the start of the previous loop*.
		*/
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeupTime, NULL);

		
		/* Sends all datagrams in the queue.
		   This method takes all datagrams that have been queued for transmission,
		   puts them into frames, and passes them to the Ethernet device for sending. 
		*/
		ecrt_master_send(master);
		
		#ifdef SYNC_MASTER_TO_REF
		// update the master clock
     		// Note: called after ecrt_master_send() to reduce time
                // jitter in the sync_distributed_clocks() call
                update_master_clock();
		#endif
		
		#ifdef IPC
		msg.actPos[0] = actPos0;
		msg.actPos[1] = actPos1;
		msg.actPos[2] = actPos2;
		msg.actPos[3] = actPos3;
		msg.targetPos[0] = targetPos0;
		msg.targetPos[1] = targetPos1;
		msg.targetPos[2] = targetPos2;
		msg.targetPos[3] = targetPos3;
		
		#ifdef MEASURE_PERF
		msg.updatePeriod = t_cur - t_prev;
		t_prev = t_cur;
		#endif
		
		/*  msgsnd appends a copy of the message pointed to by msg to the message queue 
		    whose identifier is specified by msqid.
		*/
		if (msgsnd(qID, &msg, msgSize, IPC_NOWAIT)) 
		{
			printf("Error sending message to the queue: %s\n", strerror(errno));
			printf("Terminating the process...\n");
			return -1;
		}
		#endif
		
		#ifdef LOG
		i = i + 1;
		#endif
	
	}
	
	ecrt_release_master(master);

	return 0;
}
