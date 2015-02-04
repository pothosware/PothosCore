//source modified from https://github.com/mfukar/lfsr

/*
 * Galois LFSR software implementation header file
 */

typedef	long long lfsr_data_t;

typedef struct {
        lfsr_data_t data,
                    polynomial,
                    mask;
} lfsr_t;

static inline
void          GLFSR_init(lfsr_t *, lfsr_data_t, lfsr_data_t);
static inline
unsigned char GLFSR_next(lfsr_t *);

/*
 * @file	lfsr.c
 * @author	Michael Foukarakis
 * @version	1.1
 * @date
 * 	Created:     Wed Feb 02, 2011 18:47 EET
 * 	Last Update: Tue Mar 26, 2013 12:40 GTB Standard Time
 *------------------------------------------------------------------------
 * Description:	Galois LFSR software implementation
 * 		WARNING:
 * 		Polynomial representation: x^4 + x^3 + 1 = 11001 = 0x19
 *
 * Well-known polynomials:
 * CRC-12	: x^12 + x^11 + x^3 + x^2 + x + 1
 * CRC-16-IBM	: x^16 + x^15 + x^2 + 1
 * CRC-16-DECT	: x^16 + x^10 + x^8 + x^7 + x^3 + 1
 * CCITT	: x^16 + x^12 + x^5 + 1
 * CRC-32-IEEE	: x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1
 *
 * Spread-spectrum sequences:
 * 7-bit	: x^7 + x + 1
 * 13-bit	: x^13 + x^4 + x^3 + 1
 * 19-bit	: x^19 + x^5 + x^2 + x + 1
 *
 * Used in A5:
 * x^19 + x^5 + x^2 + x + 1
 * x^22 + x + 1
 * x^23 + x^15 + x^2 + x + 1
 * x^17 + x^5 + 1
 *
 * GPS satellites:
 * x^10 + x^3 + 1
 * x^10 + x^9 + x^8 + x^6 + x^3 + x^2 + 1
 *
 * Source of polynomials:
 * http://homepage.mac.com/afj/taplist.html
 * http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
 *------------------------------------------------------------------------
 * History:	None yet
 * TODO:	Nothing yet
 *------------------------------------------------------------------------
 */

static inline
void GLFSR_init(lfsr_t *glfsr, lfsr_data_t polynom, lfsr_data_t seed_value)
{
    lfsr_data_t seed_mask;
    unsigned int shift = 8 * sizeof(lfsr_data_t) - 1;

    glfsr->polynomial = polynom | 1;
    glfsr->data = seed_value;

    seed_mask = 1;
    seed_mask <<= shift;

    while(shift--) {
        if(polynom & seed_mask) {
            glfsr->mask = seed_mask;
            break;
        }
        seed_mask >>= 1;
    }
    return;
}

static inline
unsigned char GLFSR_next(lfsr_t *glfsr)
{
	unsigned char retval = 0;

	glfsr->data <<= 1;

	if(glfsr->data & glfsr->mask) {
		retval = 1;
		glfsr->data ^= glfsr->polynomial;
	}

	return(retval);
}
