/*
 * check_sum.c
 *	Контрольная сумма crc-16
 *  Created on: 5 апр. 2018 г.
 *      Author: jake
 */

unsigned short csum(unsigned short *iph,int size)
{
	/*
	1. Весь буфер разбивается на слова по два байта.
	2. Считается сумма. Арифметическая.
	*/
	register long cksum = 0;

	while (size > 1){
		cksum += *iph++;
	    size  -= sizeof(unsigned short);
	}

	/* 3. Если длина буфера нечетная - дополняется нулем */
	if (size)
		cksum += *(unsigned char*)iph;

	/* 4. Если сумма не вмещается в двухбайтное число, старшее слово складывается с младшим */
	cksum = (cksum >> 16) + (cksum & 0xffff);

	/*5. После операции 3 все равно может произойти переполнение 16р резултата, поэтому еще раз */
	cksum += (cksum >>16);

	/* тут сделана небольшая оптимизация - убрана одна операция & 0xffff,
	 * т.к. она уже не повлияет на результат

	6. Результат усекается до 16р целого  */
	return (unsigned short)(~cksum);

}
