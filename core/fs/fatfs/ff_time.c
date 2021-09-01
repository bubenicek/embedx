
#include "system.h"
#include "integer.h"


/**
 * The get_fattime function gets current time.
 *
 * Return Value
 * Currnet time is returned with packed into a DWORD value. The bit field is as follows:
 *
 * bit31:25
 * Year from 1980 (0..127)
 *
 * bit24:21
 * Month (1..12)
 *
 * bit20:16
 * Date (1..31)
 *
 * bit15:11
 * Hour (0..23)
 *
 * bit10:5
 * Minute (0..59)
 *
 * bit4:0
 * Second/2 (0..29)
 *
 * Description
 * The get_fattime function must return any valid time even if the system does not support a real time clock. This fucntion is not required in read only configuration.
 *
 */
DWORD get_fattime(void)
{
/*   
	DWORD res;
	LIBWRAP_TM_S tm;

    libwrap_rtc_get_time(&tm);

	res = (
		(( tm.sec/2 )       << 0UL  ) |
		(( tm.min )         << 5UL  ) |
		(( tm.hour)         << 11UL ) |
		(( tm.day )         << 16UL ) |
		(( tm.month )       << 21UL ) |
		(( tm.year-1980)    << 25UL )
	);

	return res;
*/
   return 0;
}
