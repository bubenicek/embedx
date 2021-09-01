
#ifndef __LTC2357_H
#define __LTC2357_H

/** Number of channels */
#define LTC2357_NUM_CHANNELS    4

/** Sample type */
typedef struct
{
   uint16_t channel[LTC2357_NUM_CHANNELS];

} LTC2357_samples_t;

/** Initialize LTC2357 */
int LTC2357_init(void);

/** Read channels samples */
int LTC2357_read_samples(LTC2357_samples_t *samples);


#endif  // __LTC2357_H
