/*
                            Sistemas Microcontrolados
E10 - Controle de velocidade de motor DC por PWM
*/

#include "stm32f1xx.h"
#define waitcte 1

//  Variável global que marca a coluna varrida. 
int col = 0;
//  Variável global que marca o tempo de espera entre dois acionamentos
//de um botão.
int wait = 0;

//  Variável global que marca a tecla pressionada.
char pressedKey = 0;
//  Vetor global que define os caracteres presentes no teclado matricial.
const char keyMTX[4][4] = {
    {   '1',  '2',  '3',  'A'}, 
    {   '4',  '5',  '6',  'B'},
    {   '7',  '8',  '9',  'C'},
    {   '*',  '0',  '#',  'D'}
};

//  Buffer que armazena o valor de até três dígitos do duty cycle.
char buffer[4] = "000";
//  Variável que armazena o tamanho atual do buffer.
int bufCount = 0;

//  Função que envia um caractere para a USART.
void EnviaDadoUSART(char tx_dado)  {
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = tx_dado;
}

//  Função de tratamento do SysTick que realiza a varredura do teclado.
void SysTick_Handler(void) {
    //  Incrementa a variável de iteração e volta para o começo se 
    //chegou ao fim das colunas.
    col++;
    if(col == 4)
        col = 0;

    //  Manda um sinal alto para a coluna no passo da iteração.
    GPIOA->ODR &= ~0xF0;
    GPIOA->ODR |= (1 << (col+4));

    //  Se a variável de espera for igual à constante de espera, foi
    //pressionada uma tecla.
    if(wait == waitcte) {
        //  Se a tecla pressionada foi um dígito de 0-9 e o buffer não está cheio,
        //insere no buffer o dígito lido e o imprime na USART.
        if(pressedKey >= '0' && pressedKey <= '9' && bufCount < 3) {
            buffer[bufCount++] = pressedKey;

            EnviaDadoUSART(pressedKey);
        }
        //  Se a tecla pressionda foi o 'D', atualiza o valor de PWM de acordo com 
        //o buffer e o imprime na USART.
        if(pressedKey == 'D') {
            //  Converte a string para um valor numérico na base 10.
            int dutyCycle = (buffer[0]-'0')*100 + (buffer[1]-'0')*10 + (buffer[2]-'0');

            //  Se o buffer não estava cheio, a expressão anterior é inválida, já que
            //a inserção é realizada no começo do buffer. Para sanar isso, para cada
            //dígito não inserido, é dividido o valor numérico por 10 (deslocamento
            //a esquerda). 
            for(int i = 0; i < 3-bufCount; i++)
                dutyCycle = dutyCycle/10;

            //  Se o valor numérico for maior que 100, o limita em 100.
            if(dutyCycle > 100) {
                dutyCycle = 100;

                EnviaDadoUSART('\r');
                EnviaDadoUSART('1');
                EnviaDadoUSART('0');
                EnviaDadoUSART('0');
            }

            //  Atualiza o registrador de captura e comparação com o novo duty cycle.
            TIM4->CCR1 = dutyCycle * TIM4->ARR/100;

            //  Limpa o buffer para leituras posteriores.
            bufCount = 0;
            buffer[0] = '0';
            buffer[1] = '0';
            buffer[2] = '0';


            EnviaDadoUSART('%');
            EnviaDadoUSART('\n');
        }
    }

    //  Espera o fim do delay entre o pressionamento das teclas.
    if(wait > 0) {
        wait--;
        if(wait == 0) 
            EXTI->IMR |= EXTI_IMR_IM0 | EXTI_IMR_IM1 | EXTI_IMR_IM2 | EXTI_IMR_IM3; 
    }
}

//  Funções de tratamento das interrupções das portas. Quando é detectada uma borda
//de subida em uma das linhas, atualiza a tecla pressionada de acordo com essa linha
//e a coluna encontrada na varredura. 

void EXTI0_IRQHandler(void) {
    EXTI->PR |= EXTI_PR_PR0;

    if(GPIOA->IDR & (1 << 0)) {
        EXTI->IMR &= ~EXTI_IMR_IM0;
        pressedKey = keyMTX[0][col];
        wait = waitcte;
    }
}

void EXTI1_IRQHandler(void) {
    EXTI->PR |= EXTI_PR_PR1;
    
    if(GPIOA->IDR & (1 << 1)) {
        EXTI->IMR &= ~EXTI_IMR_IM1;
        pressedKey = keyMTX[1][col];
        wait = waitcte;
    }
}

void EXTI2_IRQHandler(void) {
    EXTI->PR |= EXTI_PR_PR2;
    
    if(GPIOA->IDR & (1 << 2)) {
        EXTI->IMR &= ~EXTI_IMR_IM2;
        pressedKey = keyMTX[2][col];
        wait = waitcte;
    }
}

void EXTI3_IRQHandler(void) {
    EXTI->PR |= EXTI_PR_PR3;
    
    if(GPIOA->IDR & (1 << 3)) {
        EXTI->IMR &= ~EXTI_IMR_IM3;
        pressedKey = keyMTX[3][col];
        wait = waitcte;
    }
}

//  Configura o SysTick para um delay de 1/(8e6/8e5) = 100ms
void systickCfg(void) {
    SysTick->LOAD = 800000;
    SysTick->VAL = 0x0;
    SysTick->CTRL |= 0b111;
}

//  Configura a GPIOA para o teclado.
void tecladoCfg(void) {
    // PA0-PA3 como entradas pull-down (CRL = 1000 << pos)

    GPIOA->CRL |= GPIO_CRL_CNF0_1;
    GPIOA->CRL &= ~(GPIO_CRL_CNF0_0); 

    GPIOA->CRL |= GPIO_CRL_CNF1_1;
    GPIOA->CRL &= ~(GPIO_CRL_CNF1_0);

    GPIOA->CRL |= GPIO_CRL_CNF2_1;
    GPIOA->CRL &= ~(GPIO_CRL_CNF2_0);

    GPIOA->CRL |= GPIO_CRL_CNF3_1;
    GPIOA->CRL &= ~(GPIO_CRL_CNF3_0);

    // PA4-PA7 como saídas push-pull (CRL = 0010 << pos)

    GPIOA->CRL &= ~(GPIO_CRL_CNF4_0);
    GPIOA->CRL |= GPIO_CRL_MODE4_1;
    
    GPIOA->CRL &= ~(GPIO_CRL_CNF5_0);
    GPIOA->CRL |= GPIO_CRL_MODE5_1;

    GPIOA->CRL &= ~(GPIO_CRL_CNF6_0);
    GPIOA->CRL |= GPIO_CRL_MODE6_1;

    GPIOA->CRL &= ~(GPIO_CRL_CNF7_0);
    GPIOA->CRL |= GPIO_CRL_MODE7_1;
}

//  Configura a USART1
void usartCfg(void) {
    //  Ativa o PA9 como saída de função alternativa (TX do USART1)
    GPIOA->CRH |= GPIO_CRH_CNF9_1;  
    GPIOA->CRH &= ~(GPIO_CRH_CNF9_0);
    GPIOA->CRH |= GPIO_CRH_MODE9_1;

    //  Baud rate = 9600baud/s
    USART1->BRR = 8000000/9600;
    //  Ativa a transmissão e o periférico.
    USART1->CR1 |= USART_CR1_TE;
    USART1->CR1 |= USART_CR1_UE; 
}

//  Configura o TIM4
void timCfg(void) {
    //  Valor de auto carregamento = 999.
    TIM4->ARR = 0x999;
    //  Prescaler em 79 => freq_contagem = 8e6/(79+1) = 100KHz.
    TIM4->PSC = 79;
    //  Inicia a contagem em 0.
    TIM4->CNT = 0;

    //  Inicializa o duty cycle do PWM como 0.
    TIM4->CCR1 = 0;

    //  Configura o modo captura e comparação para PWM (bits OC1M em 110)
    //no canal 1 (função alternativa PB6).
    TIM4->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;
    TIM4->CCMR1 &= ~(TIM_CCMR1_OC1M_0); 

    //  Ativa a captura/comparação.
    TIM4->CCER |= TIM_CCER_CC1E;

    //  Ativa o timer 4.
    TIM4->CR1 |= TIM_CR1_CEN;

    //  Configura a PB6 como saída de função alternativa.
    GPIOB->CRL |= GPIO_CRL_CNF6_1;  
    GPIOB->CRL &= ~(GPIO_CRL_CNF6_0);
    GPIOB->CRL |= GPIO_CRL_MODE6_1;
}

int main(void) {
    //  Ativa GPIOA, GPIOB, GPIOC, AFIO, USART1 no barramento rápido.
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN | RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPCEN;
    //  Ativa TIM4 no barramento lento.
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    //  PC13 como saída para debug.
    GPIOC->CRH = (GPIOC->CRH & ~(GPIO_CRH_MODE13_Msk | GPIO_CRH_CNF13_Msk)) | (0b0110 << GPIO_CRH_MODE13_Pos); 

    //  Configura os periféricos.
    tecladoCfg();
    usartCfg();
    systickCfg();
    timCfg();

    //  Ativa as interrupções do EXTI no AFIO como borda de subida.
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PA | AFIO_EXTICR1_EXTI1_PA | AFIO_EXTICR1_EXTI2_PA | AFIO_EXTICR1_EXTI3_PA;
    EXTI->RTSR |= EXTI_RTSR_RT0 | EXTI_RTSR_RT1 | EXTI_RTSR_RT2 | EXTI_RTSR_RT3;
    EXTI->IMR |= EXTI_IMR_IM0 | EXTI_IMR_IM1 | EXTI_IMR_IM2 | EXTI_IMR_IM3; 

    //  Ativa as interrupções no vetor de interrupções do NVIC.
    NVIC->ISER[0] |= (1 << EXTI0_IRQn) | (1 << EXTI1_IRQn) | (1 << EXTI2_IRQn) | (1 << EXTI3_IRQn);

    //  Loop principal.
    while(1);

    return 0;
}