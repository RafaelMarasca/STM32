.thumb				// Instruções THUMB
.syntax unified		// Usa sintaxe nova
.data
SENSOR1 : .word 0
SENSOR2 : .word 0 

.text				// Informa ao assembler para montar este texto

// Declara funções globais (para outro arquivo acessar)
.global MainAsm
.global GpioCfg
.global InterruptCfg
.global SystickCfg
.global EXTI0_IRQHandler
.global EXTI1_IRQHandler
.global SysTick_Handler

// Constantes
.equ	GPIOA_base,	0x40010800
.equ	GPIOA_CRL,	GPIOA_base + 0x00
.equ	GPIOA_CRH,	GPIOA_base + 0x04
.equ	GPIOA_IDR,	GPIOA_base + 0x08
.equ	GPIOA_ODR,	GPIOA_base + 0x0C
.equ	GPIOA_BSRR, GPIOA_base + 0x10
.equ	GPIOA_BRR,	GPIOA_base + 0x14
.equ	GPIOA_LCKR, GPIOA_base + 0x18
.equ	RCC_APB2ENR,	0x40021018
.equ    AFIO_base, 0x40010000
.equ    EXTICR1, AFIO_base + 0x08 
.equ    EXTI_base, 0x40010400
.equ    EXTI_IMR, EXTI_base + 0x00
.equ    EXTI_RTSR, EXTI_base + 0x08
.equ    EXTI_PR, EXTI_base + 0x14
.equ    NVIC_base, 0xE000E100 
.equ    SysTick_CSR, 0xE000E010
.equ    SysTick_RVR, 0xE000E014
.equ    SysTick_CVR, 0xE000E018
.equ    SysTick_irq_pos, NVIC_base + 0x3C
.equ    EXTI0_irq_pos, NVIC_base + 0x58
.equ    EXTI1_irq_pos, NVIC_base + 0x5C
.equ	MOTOR_DELAY,	0xF42400 
.equ    CRL_VALUE, 0x44446688
.equ    ACTLR, 0xE000E008
.equ    RESET_VAL, 0xFFFFFF

// Função de setup
MainAsm:
    BL GpioCfg
    BL InterruptCfg
    BL SystickCfg
    B MainLoop

// Loop infinito
MainLoop:
    B MainLoop

// Função de configuração para o GPIO.
GpioCfg:
    // Habilita o clock para o GPIOA e para o AFIO.
    LDR r6, =RCC_APB2ENR
    LDR r0, [r6]
    ORR r0, #0x5 
    STR r0, [r6]

    // Configura os pinos A0 e A1 como entrada p-d e os pinos A2 e A3 como saída o-d.
    LDR r6, =GPIOA_CRL
    LDR r0, =CRL_VALUE
    STR r0, [r6]

    // Faz com que o pino A3 esteja inicialmente setado.
    LDR r6, =GPIOA_ODR
    LDR r0, [r6]
    ORR r0, #0x4
    STR r0, [r6]

    BX lr

// Função de configuração para as interrupções externas.
InterruptCfg:
    // Configura as interrupções com borda de subida.
    LDR r6, =EXTI_RTSR
    LDR r0, [r6]
    ORR r0, #0x3
    STR r0, [r6]

    // Configura a máscara de interrupções dos EXTI 0 e 1.
    LDR r6, =EXTI_IMR
    LDR r0, [r6]
    ORR r0, #0x3
    STR r0, [r6]

    // Ativa as interrupções dos EXTI 0 e 1 no vetor de interrupções.
    LDR r6, =NVIC_base
    LDR r0, [r6]
    ORR r0, #0xC0
    STR r0, [r6]

    BX lr

// Função de configuração para o SysTick.
SystickCfg:
    // Seta os três bits do registrador de configuração para ativar a contagem e o periférico.
    LDR r6, =SysTick_CSR
    LDR r0, [r6]
    ORR r0, #0x7
    STR r0, [r6]

    // Carrega 16x10^6 no registrador de estouro (delay de 2s a 8MHz).
    LDR r6, =SysTick_RVR
    LDR r0, =MOTOR_DELAY
    STR r0, [r6]

    // Reseta a contagem.
    LDR r6, =SysTick_CVR
    MOV r0, #0x0
    STR r0, [r6]

    BX lr

// Função de tratamento para a interrupção no pino A0.
.type EXTI0_IRQHandler, %function
EXTI0_IRQHandler:
    // Tira o status de pendente para a IRQ.
    LDR r6, =EXTI_PR
    LDR r0, [r6]
    ORR r0, #0x1
    STR r0, [r6]

    // Se a flag SENSOR1 for diferente de 0, pula para o final da função.
    LDR r0, =SENSOR1
    LDR r1, [r0]

    CBNZ r1, SKIP1

    // Seta o pino A3, reseta o pino A2 e reseta a flag. 
    LDR r6, =GPIOA_ODR
    LDR r0, [r6]
    ORR r0, #0x8
    STR r0, [r6]

    MOV r0, #0x4
    LDR r3, [r6]
    BIC r0, r3, r0
    STR r0, [r6]

    MOV r0, #0x1
    LDR r3, =SENSOR1
    STR r0, [r3]

    SKIP1:

    BX lr

// Função de tratamento para a interrupção no pino A1.
.type EXTI1_IRQHandler, %function
EXTI1_IRQHandler:
    // Marca a IRQ como não-pendente.
    LDR r6, =EXTI_PR
    LDR r0, [r6]
    ORR r0, #0x2
    STR r0, [r6]

    // Se a flag SENSOR2 for diferente de 0, pula para o final da função.
    LDR r0, =SENSOR2
    LDR r1, [r0]

    CBNZ r1, SKIP2

    // Seta o pino A2, reseta a flag e reseta a contagem.
    LDR r6, =GPIOA_ODR
    LDR r0, [r6]
    ORR r0, #0x4
    STR r0, [r6]

    MOV r0, #0x1
    LDR r3, =SENSOR2
    STR r0, [r3]

    LDR r6, =SysTick_CVR
    MOV r0, #0x0
    STR r0, [r6]

    SKIP2:

    BX lr

// Função de tratamento para a interrupção de estouro do SysTick.
.type SysTick_Handler, %function
SysTick_Handler:
    // Se pelo menos uma das flags for 0, pula para o fim da função.
    LDR r0, =SENSOR1
    LDR r1, =SENSOR2

    LDR r2, [r0]
    LDR r3, [r1]

    ANDS r0, r2, r3

    CBZ r0, SKIP3

    // Seta o pino A3 e reseta as flags.
    LDR r6, =GPIOA_ODR
    MOV r0, #0x8
    LDR r3, [r6]
    BIC r0, r3, r0
    STR r0, [r6]
    
    MOV r0, #0x0
    LDR r3, =SENSOR1
    STR r0, [r3]

    LDR r3, =SENSOR2
    STR r0, [r3]

    SKIP3:
    
    BX lr