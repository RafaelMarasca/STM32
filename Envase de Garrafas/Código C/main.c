/**
    Projeto: Envase de Garrafas
    Descrição: Implementa um sistema de envase de garrafas em C
*/

#include "stm32f1xx.h"

// Variáveis globais para controle dos botões
int SENSOR1 = 0;
int SENSOR2 = 0;

// Função para configuração dos pinos de entrada e saída
void gpioa_pin_setup(void) {
    // Pino A0 como input pull-down
    GPIOA->CRL |= GPIO_CRL_CNF0_1;
    GPIOA->CRL &= ~GPIO_CRL_CNF0_0;

    // Pino A1 como input pull-down
    GPIOA->CRL |= GPIO_CRL_CNF1_1;
    GPIOA->CRL &= ~GPIO_CRL_CNF1_0;

    // Pino A2 como output open drain com velocidade máxima de 2MHz.
    GPIOA->CRL |= GPIO_CRL_MODE2_1;
    GPIOA->CRL &= ~(GPIO_CRL_MODE2_0);
    GPIOA->ODR |= (GPIO_ODR_ODR2);

    // Pino A3 como output open drain com velocidade máxima de 2MHz.
    GPIOA->CRL |= GPIO_CRL_MODE3_1;
    GPIOA->CRL &= ~(GPIO_CRL_MODE3_0);
    GPIOA->ODR &= ~(GPIO_ODR_ODR3);
} 

// Função para a configuração das interrupções dos pinos de entrada.
void input_interrupt_setup(void) {
    // Seta os bits correspondentes às interrupções externas nos canais 0 e 1.
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA | AFIO_EXTICR1_EXTI1_PA;
    
    // Configura as interrupções externas nos canais 0 e 1 para rising edge.
    EXTI->RTSR = EXTI_RTSR_TR0 | EXTI_RTSR_TR1;

    // Configura a máscara de interrupção das EXTI 0 e 1.
    EXTI->IMR = EXTI_IMR_MR0 | EXTI_IMR_MR1; 

    // Seta os bits das EXTI no vetor de interrupções.
    NVIC->ISER[0] |= (uint32_t)(1<<EXTI0_IRQn) | (uint32_t)(1<<EXTI1_IRQn);
    
}

// Função para configuração do SysTick.
void systick_setup(void) {
    // Define o valor de estouro como 16x10^6. Com um clock de 8MHz, a contagem dura 2s.
    SysTick->LOAD = 16000000;

    // Reseta o valor de contagem.
    SysTick->VAL  = 0;

    // Seta os bits do registrador de controle para iniciar a contagem e ativar o SysTick.
    SysTick->CTRL |= (1<<SysTick_CTRL_ENABLE_Pos)
                    |(1<<SysTick_CTRL_TICKINT_Pos) 
                    | (1<<SysTick_CTRL_CLKSOURCE_Pos);
}

// Função de tratamento de interrupção da EXTI0
void EXTI0_IRQHandler(void) {
    // Marca a interrupção como não-pendente.
    EXTI->PR |= EXTI_PR_PR0;

    // Se a flag SENSOR1 for 0, seta o pino A3, reseta o pino A2 e seta a flag.
    if(!SENSOR1) {
        GPIOA->ODR |= GPIO_ODR_ODR3;
        GPIOA->ODR &= ~GPIO_ODR_ODR2;
        SENSOR1 = 1;
    }
}

// Função de tratamento de interrupção da EXTI0.
void EXTI1_IRQHandler(void) {
    // Marca a interrupção como não-pendente.
    EXTI->PR |= EXTI_PR_PR1;

    // Se a flag SENSOR2 for 0, seta o pino A2, reseta a contagem e seta a flag.
    if(!SENSOR2) {
        GPIOA->ODR |= GPIO_ODR_ODR2;
        SENSOR2 = 1;
        SysTick->VAL = 0;
    }
}

// Função de tratamento de interrupção do SysTick.
void SysTick_Handler(void) {
    // Se ambas as flags forem 1, reseta o pino A3 e seta as flags.
    if(SENSOR2 && SENSOR1) {
        GPIOA->ODR &= ~GPIO_ODR_ODR3;
        SENSOR1 = SENSOR2 = 0;
    }
}

int main(void) {
    // Ativa o clock do barramento APB2 para o AFIO e para o GPIOA.
    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN);

    // Chama as funções de setup.
    gpioa_pin_setup();
    input_interrupt_setup();
    systick_setup();

    // Loop infinito.
    while(1) {           
        
    }

    return 0;
}