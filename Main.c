// Todos os direitos reservados, isso não é uma licença MIT. Não altere ou copie.

// The Hang Memory's Game

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>

#define LED_PIN 9
#define BOTAO_PIN_A 2 
#define BOTAO_PIN_B 7
#define DEBOUNCE_DELAY 50 
#define OLED_SDA 21 
#define OLED_SCL 20 
#define I2C_PORT i2c0 

//inicia Display
ssd1306_t oled;

//variáveis de funcionamento global.
uint8_t tentativas_restantes = 3;
uint8_t palavra_atual = 0;
char sequencia_mostrada[20];
char sequencia_inserida[20];
int indice_sequencia = 0;
int pontos = 0;

//arrays de nível recorrente para funcionamento específica da jogabilidade.
const char *palavras[] = {"PERA", "MORANGO", "EMBARCADOS", "CONTROLADOR"};
const char *palavra_parcial[] = {"PE_A", "M_RANG_", "EMB_RC_DOS", "CONTR_L_DOR"};
const char *binarios[] = {
    "0001", // PERA
    "0011", // MORANGO
    "0110", // EMBARCADOS
    "1001"  // CONTROLADOR
};

//função modular padronizada que atualiza o display conforme exibe a mensagem inicial e a palavra a ser identificada.
void atualizar_display(const char *mensagem, const char *palavra_parcial) {
    printf("%s - %s\n", mensagem, palavra_parcial);
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 0, 1, mensagem);
    ssd1306_draw_string(&oled, 0, 20, 1, "Palavra:");
    ssd1306_draw_string(&oled, 0, 30, 1, palavra_parcial);

    char tentativas[20];
    sprintf(tentativas, "Tentativas: %d", tentativas_restantes);
    ssd1306_draw_string(&oled, 0, 50, 1, tentativas);

    ssd1306_show(&oled);
}

//buffer de mensagem temporária na memória do programa, conversa diretamente com a função anterior
void mostrar_mensagem_temporaria(const char *mensagem) {
    // printf("Exibindo mensagem temporária: %s\n", mensagem); //para debug no serial
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 20, 1, mensagem);
    ssd1306_show(&oled);
    sleep_ms(4000); // Mostra a mensagem por 4 segundos
}

//"animacao_boneco" e "animacao_inicio" feitas exclusivamente para interatividade lúdica entre usuário e máquina (exibe animação e nome do jogo).
void animacao_boneco() {
    // printf("Exibindo animação do boneco da forca\n");

    // Primeiro frame
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 15, 1, "Bem vindo!");
    ssd1306_draw_string(&oled, 90, 15, 1, " O");
    ssd1306_draw_string(&oled, 90, 25, 1, "/|\\");
    ssd1306_draw_string(&oled, 90, 35, 1, "/ \\");
    ssd1306_show(&oled);
    sleep_ms(3000);

    // Segundo frame
    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 15, 1, "Bem vindo!");
    ssd1306_draw_string(&oled, 90, 10, 1, "+---+");
    ssd1306_draw_string(&oled, 90, 15, 1, "|   O");
    ssd1306_draw_string(&oled, 90, 25, 1, "|  /|\\");
    ssd1306_draw_string(&oled, 90, 35, 1, "|  / \\");
    ssd1306_draw_string(&oled, 90, 45, 1, "|");
    ssd1306_draw_string(&oled, 90, 50, 1, "===");
    ssd1306_show(&oled);
    sleep_ms(3000);
}
void animacao_inicio() {
    // printf("Exibindo animação inicial\n"); //para debug no serial

    const char *titulo = "The Hang Memory's Game";
    char buffer[30] = "---a-g-M-m-r----a-e-----";

    for (int i = 0; i < strlen(titulo); i++) {
        buffer[i] = titulo[i];
        ssd1306_clear(&oled);
        ssd1306_draw_string(&oled, 0, 15, 1, buffer);
        ssd1306_show(&oled);
        sleep_ms(100);
    }

    sleep_ms(1000);

    animacao_boneco();

    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 20, 1, "Comecando...");
    ssd1306_show(&oled);
    sleep_ms(3000);

    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 20, 1, "Observe bem o LED...");
    ssd1306_show(&oled);
    sleep_ms(3000);

    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 0, 20, 1, "Pronto? ");
    ssd1306_show(&oled);
    sleep_ms(3000);
}

//função modular que cria a lógica de piscar o led (em binário) conforme a palavra indicada é exibida.
void piscar_sequencia_binaria(const char *binario) {

    // printf("Piscando sequência binária: %s\n", binario); debug serial (habilite para ver o binário correspondente à palavra)
    
    for (int i = 0; i < strlen(binario); i++) {
        if (binario[i] == '1') {
            gpio_put(LED_PIN, 1);
        } else {
            gpio_put(LED_PIN, 0);
        }
        sleep_ms(400);
    }
    gpio_put(LED_PIN, 0);
}

//função modular criada para registrar a sequência indicada pelo jogador (input exclusivo de feedback visual do usuário).
void piscar_sequencia(const char *palavra) {

    // printf("Piscando sequência para a palavra: %s\n", palavra); debug serial (habilite se quiser ver qual é a palavra exibida)
    
    for (int i = 0; i < strlen(palavra); i++) {

        // printf("Piscar LED para letra: %c\n", palavra[i]);

        gpio_put(LED_PIN, 1);
        sleep_ms(400);
        gpio_put(LED_PIN, 0);
        sleep_ms(400);
        sequencia_mostrada[i] = palavra[i];
    }
    sequencia_mostrada[strlen(palavra)] = '\0';

    // Mostra binário após sequência
    piscar_sequencia_binaria(binarios[palavra_atual]);
    mostrar_mensagem_temporaria("Sua vez");
}

//função booleana que retorna o input do jogador no botão A
bool verificar_sequencia() {
    printf("Verificando sequência inserida pelo jogador\n"); //para debug no serial
    if (strcmp(sequencia_mostrada, sequencia_inserida) == 0) {
        return true;
    }
    return false;
}

//função que valida a lógica de acerto e erro do loop principal.
void aguardar_sequencia(const char *palavra, const char *parcial) {
    // printf("Aguardando confirmação do jogador\n");

    if (verificar_sequencia()) {
        pontos += strlen(palavra) * 2; // Ganha pontos após palavra
        mostrar_mensagem_temporaria("Acertou!");
        palavra_atual++;
    } else {
        tentativas_restantes--;
        mostrar_mensagem_temporaria("Errou");

        if (tentativas_restantes > 0) {
            atualizar_display("Tente novamente!", parcial);
        } else {
            mostrar_mensagem_temporaria("Jogo reiniciado!");
            tentativas_restantes = 3;
            palavra_atual = 0;
            pontos = 0;
        }
    }
    indice_sequencia = 0;
    memset(sequencia_inserida, 0, sizeof(sequencia_inserida)); //bloco de memória para a sequencia inserida pelo jogador (armazena a informação).
}

loop_principal(){
    if (palavra_atual >= 4) {
      mostrar_mensagem_temporaria("Parabens, voce venceu!");
      return;
  }

  // printf("Iniciando nível com a palavra: %s\n", palavras[palavra_atual]); (spoiler da palavra)
  atualizar_display("Memorize a palavra:", palavra_parcial[palavra_atual]);
  piscar_sequencia(palavras[palavra_atual]);

  indice_sequencia = 0;
  while (gpio_get(BOTAO_PIN_B) != 0) {
      if (gpio_get(BOTAO_PIN_A) == 0) {
          sequencia_inserida[indice_sequencia++] = palavras[palavra_atual][indice_sequencia];
          gpio_put(LED_PIN, 1);
          sleep_ms(200);
          gpio_put(LED_PIN, 0);

          while (gpio_get(BOTAO_PIN_A) == 0) {
              sleep_ms(DEBOUNCE_DELAY);
          }
      }
      sleep_ms(10);
  }

  sequencia_inserida[indice_sequencia] = '\0';
  aguardar_sequencia(palavras[palavra_atual], palavra_parcial[palavra_atual]);
  sleep_ms(100);
}

pinos(){
    printf("Iniciando o programa...\n");

    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(OLED_SDA, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(OLED_SDA);
    gpio_pull_up(OLED_SCL);

    ssd1306_init(&oled, 128, 64, 0x3C, I2C_PORT);
    // printf("Display inicializado com sucesso!\n"); debug display serial

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    // printf("LED configurado no GPIO %d\n", LED_PIN); debug do led

    gpio_init(BOTAO_PIN_A);
    gpio_set_dir(BOTAO_PIN_A, GPIO_IN);
    gpio_pull_up(BOTAO_PIN_A);
    // printf("Botão A configurado no GPIO %d\n", BOTAO_PIN_A); debug botao A

    gpio_init(BOTAO_PIN_B);
    gpio_set_dir(BOTAO_PIN_B, GPIO_IN);
    gpio_pull_up(BOTAO_PIN_B);
    // printf("Botão B configurado no GPIO %d\n", BOTAO_PIN_B); debug botao B
}
//pinagens e configurações de funções (arrays, ponteiros, loop principal).
int main() {
    stdio_init_all();
    pinos();

    animacao_inicio();

    while (1) {
        loop_principal();
    }
    
    printf("Programa concluído!\n"); //usado para debugar o código durante o processo de criação

    return 0;
}
