/**
 * G711 A-law audio codec
 */

#ifndef __alaw_h
#define __alaw_h


#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */

#define ALAW_AMI_MASK       0x55


/**
 * Decode an A-law value to 16-bit linear PCM
 */
static inline int alaw_to_linear(unsigned char a_val)
{
	int		t;
	int		seg;

	a_val ^= 0x55;

	t = (a_val & QUANT_MASK) << 4;
	seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
	switch (seg) {
	case 0:
		t += 8;
		break;
	case 1:
		t += 0x108;
		break;
	default:
		t += 0x108;
		t <<= seg - 1;
	}
	return ((a_val & SIGN_BIT) ? t : -t);
}


/*! \brief Find the bit position of the highest set bit in a word
    \param bits The word to be searched
    \return The bit number of the highest set bit, or -1 if the word is zero. */
static inline int top_bit(unsigned int bits)
{
    int res;

    if (bits == 0)
        return -1;
    res = 0;
    if (bits & 0xFFFF0000)
    {
        bits &= 0xFFFF0000;
        res += 16;
    }
    if (bits & 0xFF00FF00)
    {
        bits &= 0xFF00FF00;
        res += 8;
    }
    if (bits & 0xF0F0F0F0)
    {
        bits &= 0xF0F0F0F0;
        res += 4;
    }
    if (bits & 0xCCCCCCCC)
    {
        bits &= 0xCCCCCCCC;
        res += 2;
    }
    if (bits & 0xAAAAAAAA)
    {
        bits &= 0xAAAAAAAA;
        res += 1;
    }
    return res;
}

/**
 * Encode a linear sample to A-law
 * @param linear The sample to encode.
 * @return The A-law value.
*/
static inline unsigned char linear_to_alaw(int linear)
{
    int mask;
    int seg;

    if (linear >= 0)
    {
        /* Sign (bit 7) bit = 1 */
        mask = ALAW_AMI_MASK | 0x80;
    }
    else
    {
        /* Sign (bit 7) bit = 0 */
        mask = ALAW_AMI_MASK;
        linear = -linear - 1;
    }

    /* Convert the scaled magnitude to segment number. */
    seg = top_bit(linear | 0xFF) - 7;
    if (seg >= 8)
    {
        if (linear >= 0)
        {
            /* Out of range. Return maximum value. */
            return (unsigned char) (0x7F ^ mask);
        }
        /* We must be just a tiny step below zero */
        return (unsigned char) (0x00 ^ mask);
    }
    /* Combine the sign, segment, and quantization bits. */
    return (unsigned char) (((seg << 4) | ((linear >> ((seg)  ?  (seg + 3)  :  4)) & 0x0F)) ^ mask);
}


#endif   // alaw.h


