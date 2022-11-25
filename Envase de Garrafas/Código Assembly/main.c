/**
    Projeto: Envase de Garrafas
    Descrição: Implementação de um sistema de envase de garrafas em assembly
*/
#include "stm32f1xx.h"


//Loop assembly
extern void MainAsm(void);
 
int main () {
    MainAsm(); //Chama o loop assembly
}