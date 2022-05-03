
#include <math.h>

#include "system.h"
#include "minmea.h"
#include "gps.h"
#include "gpsmon.h"
#include "gps_driver.h"

TRACE_TAG(gps);
#if !ENABLE_TRACE_GPS
#undef TRACE
#define TRACE(...)
#endif


// Constants:
#define CFG_GPS_THREAD_STACK_SIZE        4096
#define CFG_GPS_THREAD_PRIORITY          osPriorityNormal
#define CFG_GPS_LINEBUFSIZE              255


#define TWO_PI                           6.283185307179586476925286766559
#define DEG_TO_RAD                       0.017453292519943295769236907684886
#define RAD_TO_DEG                       57.295779513082320876798154814105


// Prototypes:
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

static char *gps_readline(void);
static void gps_thread(void *arg);


// Locals:
static const osThreadDef(GPS, gps_thread, CFG_GPS_THREAD_PRIORITY, 0, CFG_GPS_THREAD_STACK_SIZE);
static gps_receive_data_callback_t data_cb = NULL;
static struct minmea_sentence_rmc rmc_frame;
static struct minmea_sentence_gsv gsv_frame;


int gps_init(gps_receive_data_callback_t cb)
{
    data_cb = cb;

#if defined(CFG_GPS_ENABLE_UBLOX_MONITOR) && (CFG_GPS_ENABLE_UBLOX_MONITOR == 1)

    // Initialize GPS socket monitor
    if (gpsmon_init() != 0)
    {
        TRACE_ERROR("GPSMON init failed");
        return -1;
    }

#else

    // Initialize GPS driver
    if (gps_driver_init() != 0)
    {
        TRACE_ERROR("Init GPS driver failed");
        return -1;
    }

    // Start monitor thread
    if (osThreadCreate(osThread(GPS), NULL) == 0)
    {
        TRACE_ERROR("Start GPS thread failed");
        return -1;
    }

#endif   // CFG_GPS_ENABLE_UBLOX_MONITOR

    TRACE("Init");

    return 0;
}

// returns distance in meters between two positions, both specified
// as signed decimal-degrees latitude and longitude. Uses great-circle
// distance computation for hypothetical sphere of radius 6372795 meters.
// Because Earth is no exact sphere, rounding errors may be up to 0.5%.
// Courtesy of Maarten Lamers
double gps_distance_between(double lat1, double long1, double lat2, double long2)
{
    TRACE("%s  lat1: %f  lat1: %f  lat2: %f  lon2: %f", __FUNCTION__, lat1, long1, lat2, long2);

    double delta = radians(long1-long2);
    double sdlong = sin(delta);
    double cdlong = cos(delta);
    lat1 = radians(lat1);
    lat2 = radians(lat2);
    double slat1 = sin(lat1);
    double clat1 = cos(lat1);
    double slat2 = sin(lat2);
    double clat2 = cos(lat2);
    delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
    delta = sq(delta);
    delta += sq(clat2 * sdlong);
    delta = sqrt(delta);
    double denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
    delta = atan2(delta, denom);
    return delta * 6372795;
}


// returns course in degrees (North=0, West=270) from position 1 to position 2,
// both specified as signed decimal-degrees latitude and longitude.
// Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
// Courtesy of Maarten Lamers
double gps_course_to(double lat1, double long1, double lat2, double long2)
{
  double dlon = radians(long2-long1);
  lat1 = radians(lat1);
  lat2 = radians(lat2);
  double a1 = sin(dlon) * cos(lat2);
  double a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  return degrees(a2);
}


static char *gps_readline(void) 
{
	static char line[CFG_GPS_LINEBUFSIZE];
	int size, linesize = 0;
	char *ptr = line;

	while (linesize < CFG_GPS_LINEBUFSIZE-1) 
    {
        size = gps_driver_read(ptr, 1);
		if (size == 1) 
        {
			if (*ptr == '\n') 
            {
				ptr++;
				*ptr = 0;
				return line;
			}
			ptr++;
            linesize++;
		}
        else
        {
            osDelay(100);
        } 
	} 

    return NULL;
} 

static void gps_thread(void *arg)
{
    enum minmea_sentence_id sentence_id;
    char *line;

    while(1) 
    {
		if ((line = gps_readline()) == NULL)
        {
            //TRACE_ERROR("Read gps line failed");
            continue;
        }

#if defined(ENABLE_TRACE_GPS_NMEA) && (ENABLE_TRACE_GPS_NMEA == 1)
        TRACE_PRINTFF("%s", line);
#endif        

		switch((sentence_id = minmea_sentence_id(line, false))) 
        {
		    case MINMEA_SENTENCE_RMC:
            {
                if (minmea_parse_rmc(&rmc_frame, line)) 
                {
#if defined(ENABLE_TRACE_GPS_NMEA) && (ENABLE_TRACE_GPS_NMEA == 1)                    
                TRACE("$xxRMC coordinates: (%d,%d)   speed: %d   %2.2d/%2.2d/%d  %2.2d:%2.2d:%2.2d",
                    rmc_frame.latitude.value,
                    rmc_frame.longitude.value,
                    rmc_frame.speed.value,
                    rmc_frame.date.day,
                    rmc_frame.date.month,
                    rmc_frame.date.year,
                    rmc_frame.time.hours,
                    rmc_frame.time.minutes,
                    rmc_frame.time.seconds);
#endif
                   if (rmc_frame.valid)
                   {
                        struct tm tm;
                        gps_data_t data;

                        data.latitude = minmea_tocoord(&rmc_frame.latitude);
                        data.longitude = minmea_tocoord(&rmc_frame.longitude);
                        data.speed = minmea_tofloat(&rmc_frame.speed);
                        data.course = minmea_tofloat(&rmc_frame.course);

                        memset(&tm, 0, sizeof(tm));
                        tm.tm_sec = rmc_frame.time.seconds;
                        tm.tm_min = rmc_frame.time.minutes;
                        tm.tm_hour = rmc_frame.time.hours;
                        tm.tm_mday = rmc_frame.date.day;
                        tm.tm_mon = rmc_frame.date.month - 1;
                        tm.tm_year = rmc_frame.date.year + 2000 - 1900;

                        data.time = mktime(&tm);

                        if (data_cb != NULL)
                            data_cb(&data);
                   }
                }
                else 
                {
      	            TRACE_ERROR("Parse MINMEA_SENTENCE_RMC failed");
                }               
            }
			break;

            case MINMEA_SENTENCE_GSV:
            {
                if (minmea_parse_gsv(&gsv_frame, line)) 
                {               
#if defined(ENABLE_TRACE_GPS_NMEA) && (ENABLE_TRACE_GPS_NMEA == 1)
                    TRACE("$GSV: message %d of %d", gsv_frame.msg_nr, gsv_frame.total_msgs);
                    TRACE("  $GSV: sattelites in view: %d", gsv_frame.total_sats);
#endif
                    if (gsv_frame.total_sats > 0)
                    {
                        for (int i = 0; i < 4; i++)
                        {
#if defined(ENABLE_TRACE_GPS_NMEA) && (ENABLE_TRACE_GPS_NMEA == 1)                            
                            TRACE("  $GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm",
                                gsv_frame.sats[i].nr,
                                gsv_frame.sats[i].elevation,
                                gsv_frame.sats[i].azimuth,
                                gsv_frame.sats[i].snr);
#endif                                
                        }                  
                    }
                }
                else
                {
                    TRACE_ERROR("Parse MINMEA_SENTENCE_GSV failed");
                }
            }
            break;

            default:
                //TRACE_ERROR("Unsoported msg: %d", sentence_id);
                break;
	    }
    }
}