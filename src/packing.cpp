#include <packing.h>

#define SERIAL_EN true

const size_t package_size = 2 * sizeof(float) + 8 * sizeof(int16_t);
uint8_t packed[2 * package_size];  // without  closing character
uint8_t *p = packed;
//   USED FUNCTIONS



/** @brief Check whether the system is little endian or big endian.
 * @return 1 in case of little endian system, 0 otherwise
 */
int is_little_endian()
{
	float y = 16711938.0;
	if (memcmp(&y, "\x4b\x7f\x01\x02", 4) == 0)
		return 0;
	return 1;
}

/** @brief Pack data into an string.
 * @param x Memory address of the variable to pack
 * @param size Size of the variable to pack, in bytes
 * @param p Output string where the data should be packed in
 * @param le 1 if the data should be little endian packed, 0 otherwise
 */
void pack(void *x, size_t size, uint8_t *p, int le)
{
	int sysle = is_little_endian();
	const uint8_t *s = (uint8_t*)x;
	int i, incr = 1;

	if ((sysle && !le) ||
			(!sysle && le)) {
		p += size - 1;
		incr = -1;
	}

	for (i = 0; i < size; i++) {
		*p = *s++;
		p += incr;
	}
}

/** @brief hardcoded
 */
int16_t to_short(float x)
{
	return (int16_t)(x + 0.5f);
}

/** @brief Pack a float variable into an string.
 * @param x Variable to pack
 * @param p Output string where the data should be packed in
 * @param le 1 if the data should be little endian packed, 0 otherwise
 */
void pack_float(float x, uint8_t *p, int le)
{
	pack((void *)(&x), sizeof(x), p, le);
}

/** @brief Pack a float variable into an string, as a short int variable.
 *
 * This function can be useful to create decimal numbers, with a very limited
 * number of decimal positions. e.g. vel=1.2532f can be packed as a decimal number,
 * with 2 decimal digits (1.25), calling this function as:
 * pack_short(vel * 100, p, 1);
 * Of course, the unpacked value should be divided by 100 later.
 *
 * @param x Variable to pack
 * @param p Output string where the data should be packed in
 * @param le 1 if the data should be little endian packed, 0 otherwise
 * @see to_short
 */
void pack_short(float x, uint8_t* p, int le)
{
	int16_t s = to_short(x);
	pack((void *)(&s), sizeof(s), p, le);
}

//Devuelve un puntero al buffer

void packer(marineData Data) {



	pack_float(Data.new_data.lat, p, 1); p += sizeof(float);
	pack_float(Data.new_data.lon, p, 1); p += sizeof(float);
	if (SERIAL_EN)
		SerialUSB.print(" LAT: "); SerialUSB.print(Data.new_data.lat);
	SerialUSB.print(" Lng: "); SerialUSB.print(Data.new_data.lon);
	pack_float(Data.old_data.lat, p, 1); p += sizeof(float);
	pack_float(Data.old_data.lon, p, 1); p += sizeof(float);
	if (SERIAL_EN)
		SerialUSB.print(" LAT_old: "); SerialUSB.print(Data.old_data.lat);     // CLO USB y espacios
	SerialUSB.print(" Lng_old: "); SerialUSB.println(Data.old_data.lon);      // CLO USB y espacios y ln para que pase el carro.

	pack_short(Data.new_data.vel * 100, p, 1); p += sizeof(int16_t);
	pack_short((Data.new_data.heading - 180) * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.new_data.rollmax * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.new_data.pitchmax * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.new_data.rollmin * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.new_data.pitchmin * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.new_data.temp * 10, p, 1); p += sizeof(int16_t);
	pack_short(Data.new_data.vbatt * 10, p, 1); p += sizeof(int16_t);


	pack_short(Data.old_data.vel * 100, p, 1); p += sizeof(int16_t);
	pack_short((Data.old_data.heading - 180) * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.old_data.rollmax * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.old_data.pitchmax * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.old_data.rollmin * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.old_data.pitchmin * 180, p, 1); p += sizeof(int16_t);
	pack_short(Data.old_data.temp * 10, p, 1); p += sizeof(int16_t);
	pack_short(Data.old_data.vbatt * 10, p, 1); p += sizeof(int16_t);

	unsigned int ii;
	for (ii = 0; ii < 2 * package_size; ii++)
	{
	
		if (packed[ii] < 16) {     // for some reason the HEX between 1 to 16 only write one digit, not the 0 that shuold be before those numbers, so we add manually.
			SerialUSB.print("0");     // CLO ponerle el USB a Serial.print
		}
		SerialUSB.print(packed[ii], HEX);
	}

}