
#ifndef __GPS_H
#define __GPS_H


/** GPS data */
typedef struct
{
   bool valid;

   float latitude;
   float longitude;
   float speed;
   float course;   
   
   time_t time;

} gps_data_t;


/** Initialize GPS */
int gps_init(void);

/** Get GPS data */
int gps_get_data(gps_data_t *data);

// returns distance in meters between two positions, both specified
// as signed decimal-degrees latitude and longitude. Uses great-circle
// distance computation for hypothetical sphere of radius 6372795 meters.
// Because Earth is no exact sphere, rounding errors may be up to 0.5%.
// Courtesy of Maarten Lamers
double gps_distance_between(double lat1, double long1, double lat2, double long2);

// returns course in degrees (North=0, West=270) from position 1 to position 2,
// both specified as signed decimal-degrees latitude and longitude.
// Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
// Courtesy of Maarten Lamers
double gps_course_to(double lat1, double long1, double lat2, double long2);


#endif   // __GPS_H