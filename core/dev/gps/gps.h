
#ifndef __GPS_H
#define __GPS_H


/** GPS data */
typedef struct
{
   time_t time;

   double latitude;
   double longitude;
   double speed;
   double course;   

} gps_data_t;


typedef void (*gps_receive_data_callback_t)(gps_data_t *data);


/** Initialize GPS */
int gps_init(gps_receive_data_callback_t cb);

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