#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "i2c-master.h"
#include "i2c-rtc.h"

#define DefaultR (0xff)
#define DefaultG (0xef)
#define DefaultB (0x60)

void uartPutc(char c) {
	while (!(UCSRA & _BV(UDRE)))
		;
	UDR = c;
}

void uartPuts(char *s) {
	int x = 0;
	while (s[x]) {
		uartPutc(s[x]);
		x++;
	}
}
#define UART_MAXSTRLEN 100

volatile uint8_t uart_str_complete = 0;
volatile uint8_t uart_str_count = 0;
volatile char uart_string[UART_MAXSTRLEN + 1] = "";

ISR(SIG_UART_RECV) {
	unsigned char nextChar;

	nextChar = UDR;
	if (uart_str_complete == 0) {
		if (nextChar != '\n' && nextChar != '\r'
				&& uart_str_count < UART_MAXSTRLEN - 1) {
			uart_string[uart_str_count] = nextChar;
			uart_str_count++;
		} else {
			uart_string[uart_str_count] = '\0';
			uart_str_count = 0;
			uart_str_complete = 1;
		}
	}
}

int hex2dez_c(char h) {
	int res = -1;
	if (h >= '0' && h <= '9') {
		res = (h - '0');
	} else if (h >= 'A' && h <= 'F') {
		res = (h - 'A' + 10);
	} else if (h >= 'a' && h <= 'f') {
		res = (h - 'a' + 10);
	}
	return res;
}

int hex2dez(char *h) {
	int res1 = hex2dez_c(h[0]);
	int res2 = hex2dez_c(h[1]);
	if (res1 < 0 || res2 < 0)
		return -1;
	return (res1 << 4) + res2;
}

void InitPWM() {
	DDRB = (1 << PB1) | (1 << PB2) | (1 << PB3);
	TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
	TCCR1B = (1 << CS10);
	TCCR2 = (1 << CS20) | (1 << WGM20) | (1 << COM21);
}

uint8_t LinearizeForEye(uint8_t x) {
	//	if (x >= 0 && x < 5) {
	return (x);
	//	} else if (x >= 5 && x < 50) {
	//		return (x / 5);
	//	}
	//	return (((uint16_t) x) * x) >> 8;
}

char g_cPWMr = 0;
char g_cPWMg = 0;
char g_cPWMb = 0;

void SetColor(uint8_t uiR, uint8_t uiG, uint8_t uiB) {
	g_cPWMr = uiR; //LinearizeForEye(uiR) * 0.85;
	g_cPWMg = uiG; //LinearizeForEye(uiG) * 1;
	g_cPWMb = uiB; //LinearizeForEye(uiB) * 0.75;
	OCR1AL = g_cPWMr;
	OCR1BL = g_cPWMg;
	OCR2 = g_cPWMb;
}

uint8_t read_distance(int addr, int *puiLightCmd, int *puiTempLight) {
	uint8_t i2c_rtc_status;
	if (i2c_master_start_wait(addr + I2C_WRITE, 100) == 0) {
		uartPuts("\r\nError: Read Distance\r\n");
		return (0);
	}
	if (i2c_master_write(0, &i2c_rtc_status) == 0) {
		if (i2c_master_rep_start(addr + I2C_READ, &i2c_rtc_status) == 0) {
			*puiLightCmd = i2c_master_read_ack();
			if (*puiTempLight == 0) {
				*puiTempLight = i2c_master_read_nak();
			} else {
				i2c_master_read_nak();
			}
			i2c_master_stop();

			_delay_ms(10);

			if (*puiLightCmd > 0) {
				if (i2c_master_start_wait(addr + I2C_WRITE, 100) == 0) {
					return (0);
				}
				i2c_master_write(3, &i2c_rtc_status);
				i2c_master_write(0xaa, &i2c_rtc_status);
				i2c_master_stop();
			}
		}
	}
	return (1);
}

#define         SLAVE_ADDR_IR         0b00110100
#define         SLAVE_ADDR_DISTANCE1  0b00101100
#define         SLAVE_ADDR_DISTANCE2  0b00100100

#define 	I2C_ADDR_R       0x10
#define 	I2C_ADDR_G       0x12
#define 	I2C_ADDR_B       0x14
#define 	I2C_ADDR_LIGHT   0x16
#define 	I2C_ADDR_PERCENT 0x18

uint8_t readIr(int16_t *puiLightPercent, int16_t *puiLight,
		int16_t *puiLightFunction, int16_t *puiR, int16_t *puiG, int16_t *puiB) {
	uint8_t i2c_rtc_status;
	if (i2c_master_start_wait(SLAVE_ADDR_IR + I2C_WRITE, 100) == 0) {
		uartPuts("\r\nError: Wait for IR\r\n");
		return (0);
	}
	if (i2c_master_write(0, &i2c_rtc_status) == 0) {
		if (i2c_master_rep_start(SLAVE_ADDR_IR + I2C_READ, &i2c_rtc_status)
				== 0) {
			int command = (i2c_master_read_ack() << 8) | i2c_master_read_ack();
			int addr = (i2c_master_read_ack() << 8) | i2c_master_read_nak();
			i2c_master_stop();

			if (addr == 0b1110111100000000) {
				if (i2c_master_start_wait(SLAVE_ADDR_IR + I2C_WRITE, 100)
						== 0) {
					uartPuts("\r\nError: Wait for IR2\r\n");
					return (0);
				}
				i2c_master_write(3, &i2c_rtc_status);
				i2c_master_write(0xaa, &i2c_rtc_status);
				i2c_master_stop();
				switch (command) {
				case 0:
					(*puiLightPercent) += 2;
					if ((*puiLightPercent) > 100) {
						(*puiLightPercent) = 100;
					}
					break;
				case 1:
					(*puiLightPercent) -= 2;
					if ((*puiLightPercent) < 5) {
						(*puiLightPercent) = 5;
					}
					break;
				case 2:
					(*puiLight) = 0;
					break;
				case 3:
					(*puiLightFunction) = 0;
					(*puiLight) = 1;
					break;
				case 4:
					(*puiLightFunction) = 0;
					(*puiLightPercent) = 100;
					(*puiR) = 0xff;
					(*puiG) = 0x0;
					(*puiB) = 0x0;
					break;
				case 5:
					(*puiLightFunction) = 0;
					(*puiLightPercent) = 100;
					(*puiR) = 0x0;
					(*puiG) = 0xff;
					(*puiB) = 0x0;
					break;
				case 6:
					(*puiLightFunction) = 0;
					(*puiLightPercent) = 100;
					(*puiR) = 0x0;
					(*puiG) = 0x0;
					(*puiB) = 0xff;
					break;
				case 7:
					(*puiLightFunction) = 0;
					(*puiLightPercent) = 100;
					(*puiR) = DefaultR;
					(*puiG) = DefaultG;
					(*puiB) = DefaultB;
					break;
				case 8:
					(*puiR) = (*puiR) + 0x08;
					if ((*puiR) > 0xff) {
						(*puiR) = 0xff;
					}
					break;
				case 9:
					(*puiG) = (*puiG) + 0x08;
					if ((*puiG) > 0xff) {
						(*puiG) = 0xff;
					}
					break;
				case 10:
					(*puiB) = (*puiB) + 0x08;
					if ((*puiB) > 0xff) {
						(*puiB) = 0xff;
					}
					break;
				case 11:
					(*puiLightFunction) = 1;
					break;
				case 12:
					(*puiR) = (*puiR) - 0x08;
					if ((*puiR) < 0) {
						(*puiR) = 0;
					}
					break;
				case 13:
					(*puiG) = (*puiG) - 0x08;
					if ((*puiG) < 0) {
						(*puiG) = 0;
					}
					break;
				case 14:
					(*puiB) = (*puiB) - 0x08;
					if ((*puiB) < 0) {
						(*puiB) = 0;
					}
					break;
				case 15:
					(*puiLightFunction) = 2;
					break;
				case 19:
					(*puiLightFunction) = 3;
					break;
				case 23:
					(*puiLightFunction) = 4;
					break;
				}
//				i2c_rtc_sram_write(I2C_ADDR_R, puiR, 2);
//				i2c_rtc_sram_write(I2C_ADDR_G, puiG, 2);
//				i2c_rtc_sram_write(I2C_ADDR_B, puiB, 2);
//				i2c_rtc_sram_write(I2C_ADDR_PERCENT, puiLightPercent, 2);
//				i2c_rtc_sram_write(I2C_ADDR_LIGHT, puiLight, 2);
			}
		}
	}
	return (1);
}

int uiLastCmd = 0;
int uiLastCmdTimer = 0;

int main() {

	UCSRB |= _BV(TXEN) | _BV(RXEN) | _BV(RXCIE);
	UCSRC |= _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
	UBRRH = 0x00;
	UBRRL = 0x07;

	wdt_reset();
	wdt_enable(WDTO_2S);
	wdt_reset();

	uartPuts("\r\nReset-Cause:");
	switch (MCUCSR & 0x1f) {
	case 1:
		uartPuts("Power-On Reset\r\n");
		break;
	case 2:
		uartPuts("External Reset\r\n");
		break;
	case 4:
		uartPuts("Brown-Out Reset\r\n");
		break;
	case 8:
		uartPuts("Watchdog Reset\r\n");
		break;
	case 16:
		uartPuts("JTAG Reset\r\n");
		break;
	default:
		uartPuts("unknown\r\n");
		break;
	}

	uartPuts("InitPWM...");

	InitPWM();

	PORTC &= ~(1 << PC1);
	DDRC = (1 << PC1);
	PORTC |= (1 << PC1);

	uartPuts("OK\r\n");

	if (MCUCSR & (1 << WDRF)) {
		for (int xxx = 0; xxx < 255; xxx++) {
			SetColor(xxx, 0, 0);
			_delay_ms(10);
			wdt_reset();
		}
		MCUCSR &= ~(1 << WDRF);
	} else {
		for (int xxx = 0; xxx < 255; xxx++) {
			SetColor(0, xxx, 0);
			_delay_ms(10);
			wdt_reset();
		}
	}
	MCUCSR = 0;

	wdt_reset();

	uartPuts("I2C Init...");
	cli();
	i2c_master_init();
	SetColor(0, 0xff, 0xff);
	sei();
	uartPuts("OK\r\n");

	wdt_reset();

//	uartPuts("RTC Init...");
//	uint8_t i2c_errorcode, i2c_status;
//	if (!i2c_rtc_init(&i2c_errorcode, &i2c_status)) {
//		while (1) {
//			SetColor(0, 0, 0xff);
//			_delay_ms(100);
//			SetColor(0, 0, 0);
//			_delay_ms(100);
//		};
//	}

	uartPuts("Light Init...");
	SetColor(0, 0, 0);
	uartPuts("OK\r\n");

	int uiLight = 0;
	int uiLightFunction = 0;
	int uiLightPercent = 100;
//	int uiLightCmd = 0;
	int uiTempLight = 0;

	unsigned char uiFunctionX = 0;

	int uiR = DefaultR;
	int uiG = DefaultG;
	int uiB = DefaultB;

//	i2c_rtc_sram_read(I2C_ADDR_R, &uiR, 2);
//	i2c_rtc_sram_read(I2C_ADDR_G, &uiG, 2);
//	i2c_rtc_sram_read(I2C_ADDR_B, &uiB, 2);
//	i2c_rtc_sram_read(I2C_ADDR_LIGHT, &uiLight, 2);
//	i2c_rtc_sram_read(I2C_ADDR_PERCENT, &uiLightPercent, 2);

	uint8_t uiErrorCount = 0;

	while (1) {
		uartPuts("\r\nLoop start: ");

		uint8_t bHasError = 0;

		uiFunctionX++;

		wdt_reset();
		_delay_ms(50);

//		uartPuts("Read Distance1");
//		uiTempLight = 0;
//		if (read_distance(SLAVE_ADDR_DISTANCE1, &uiLightCmd, &uiTempLight)) {
//			switch (uiLightCmd) {
//			case 1:
//				uiLight = 0;
//				i2c_rtc_sram_write (I2C_ADDR_LIGHT, &uiLight, 2);
//				break;
//			case 2:
//				uiLight = 1;
//				i2c_rtc_sram_write (I2C_ADDR_LIGHT, &uiLight, 2);
//				break;
//			}
//		} else {
//			bHasError = 1;
//			uartPuts(" error");
//		}
//		uartPuts(",");
//
//		wdt_reset();
//		_delay_ms(10);
//
//		uartPuts("Read Distance2");
//		if (read_distance(SLAVE_ADDR_DISTANCE2, &uiLightCmd, &uiTempLight)) {
//			switch (uiLightCmd) {
//			case 1:
//				uiLight = 0;
//				i2c_rtc_sram_write (I2C_ADDR_LIGHT, &uiLight, 2);
//				break;
//			case 2:
//				uiLight = 1;
//				i2c_rtc_sram_write (I2C_ADDR_LIGHT, &uiLight, 2);
//				break;
//			}
//		} else {
//			bHasError = 1;
//			uartPuts(" error");
//		}
//		uartPuts(",");
//
//		wdt_reset();
//		_delay_ms(10);

		uartPuts("Read IR");

		if (!readIr(&uiLightPercent, &uiLight, &uiLightFunction, &uiR, &uiG,
				&uiB)) {
			bHasError = 1;
			uartPuts(" error");
		}
		uartPuts(",");

//		switch (uiTempLight) {
//		case 1:
//			SetColor(0x22, 0x22, 0x22);
//			break;
//		case 2:
//			SetColor(0xa0, 0xa0, 0xa0);
//			break;
//		case 3:
//			SetColor(0x00, 0x00, 0xff);
//			break;
//		case 4:
//			SetColor(0x00, 0xff, 0xff);
//			break;
//		case 5:
//			SetColor(0xff, 0x00, 0xff);
//			break;
//		case 6:
//			SetColor(0xff, 0xff, 0x00);
//			break;
//		default:
		if (uiLight == 0) {
			SetColor(0, 0, 0);
		} else if (uiLight == 1) {
			int dFunctionFactor = 1;
			switch (uiLightFunction) {
			case 1:
				dFunctionFactor = ((uiFunctionX % 10) == 0) ? 1 : 0;
				break;
			case 2:
				dFunctionFactor = ((uiFunctionX % 2) == 0) ? 1 : 0;
				break;
			}
			SetColor(dFunctionFactor * uiR * uiLightPercent / 100,
					dFunctionFactor * uiG * uiLightPercent / 100,
					dFunctionFactor * uiB * uiLightPercent / 100);
		}
//			break;
//		}

		if (bHasError) {
			uiErrorCount++;
		} else {
			uiErrorCount = 0;
		}
		if (uiErrorCount > 10) {
			uartPuts("\r\n\r\nToo many errors, resetting via Watchdog\r\n\r\n");
			while (1) {
				SetColor(0x22, 0x0, 0x0);
				_delay_ms(200);
				SetColor(0x0, 0x22, 0x0);
				_delay_ms(200);
			}
		}

		uartPuts("Loop End");
		wdt_reset ();
		_delay_ms(50);
	}
	return 0;
}
