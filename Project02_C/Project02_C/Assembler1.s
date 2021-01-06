.include<m123def.inc>

.global ler_AD

.extern leituraL
.extern leituraH

ler_AD:
	SBI ADCSRA, 6
	LDS r16, 0
	CPI r16, 0b01000000
	BRNE ler_AD

	LDS ADCL, leituraL
	LDS ADCH, leituraH

	STS leituraL
	STS leituraH

	ret
