.include<m123def.inc>

.global read_analog

.extern leituraL
.extern leituraH

read_analog:
	SBI ADCSRA, 6
	LDS r16, 0
	CPI r16, 0b01000000
	BRNE ler_AD

	LDS ADCL, leituraL
	LDS ADCH, leituraH

	STS leituraL
	STS leituraH

	ret
