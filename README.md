Ohmímetro Digital - Indentificação de Resistores Comerciais da Séria E24, Na BitDogLab

Descrição
O ohmímetro digital mede resistências de 500 ? até 100 k? usando um divisor de tensão e o ADC interno do Raspberry Pi Pico. O valor medido é arredondado conforme a série E24 e exibido no display OLED, junto com as três faixas de cores (dois dígitos significativos e multiplicador). Simultaneamente, a matriz de LEDs 5×5 pinta cada linha principal com a cor correspondente a cada faixa. Em caso de falha na leitura, o OLED mostra mensagem de erro e os LEDs permanecem apagados.

Objetivos
Medir automaticamente resistores entre 500 ? e 100 k?.
Aproximar o valor medido à série comercial E24.
Exibir no OLED o valor E24 e as cores das três faixas.
Mostrar visualmente as faixas na matriz de LEDs.
Permitir atualização de firmware via botão B (modo BOOTSEL).
Registrar log de diagnóstico via USB a cada segundo.

Componentes Utilizados
Componente - Conexão BitDogLab - Função no Projeto

Raspberry Pi Pico -	Microcontrolador principal
Resistor de 10 k? - Entre 3,3 V e GP28 - Referência fixa no divisor de tensão
Resistor sob teste - Entre GP28 e GND - Componente cuja resistência será medida
Display OLED SSD1306 - I²C (GP14 SDA, GP15 SCL) - Exibição de valor e cores
Matriz de LEDs 5×5 - PIO (GP7) - Visualização das três faixas de cor
Botão B	- GP6 - Dispara interrupção para modo BOOTSEL
USB Serial- USB - Log de diagnóstico via terminal

Funcionamento
Modo de medição:
No loop principal, o Pico faz 500 leituras do ADC para calcular a média da tensão no resistor desconhecido. Com base na lei do divisor de tensão e no resistor de referência de 10 k?, calcula-se o valor de resistência R_x. Em seguida, verifica-se se R_x está dentro da faixa válida (500 ? a 100 k?). Se estiver fora, o display OLED exibe uma mensagem informando que a medição só funciona nesse intervalo.
Se R_x for válido, o programa busca no vetor da série E24 o valor comercial mais próximo, escolhendo aquele que apresenta o menor erro em relação à medição. A rotina de cores então separa os dois primeiros dígitos e o multiplicador desse valor, mapeando cada parte para as cores das três faixas. O OLED mostra, em três linhas, as cores correspondentes e o valor E24; simultaneamente, a matriz de LEDs 5×5 pinta cada linha principal com a cor de cada faixa. Por fim, um log periódico via USB imprime no terminal o primeiro dígito e a resistência medida para fins de diagnóstico.

Modo BOOTSEL: 
Ao pressionar o Botão B conectado à interrupção, dispara-se o handler que chama reset_usb_boot(), reiniciando o Pico em modo de boot por USB. Esse modo permite atualizar o firmware diretamente, sem necessidade de reconectar manualmente o cabo no modo de inicialização.

Lógicas de suportes: 
Debounce e Interrupções: O GPIO do Botão B usa pull-up interno e interrupção em borda de descida para resposta imediata e sem bouncing.
Controle de Periféricos: I²C inicializa o SSD1306; o PIO carrega e executa o programa de animação da matriz de LEDs; o ADC é configurado para entrada analógica no pino 28.
Fluxo de Loop: O loop principal alterna entre leitura do ADC, cálculo de resistência, atualização de display/LEDs e log USB, garantindo responsividade e clareza na indicação do valor medido.

Estrutura do Código
Inicialização:

Configuração de GPIOs, ADC, I2C e PIO
Inicialização do display OLED
Carregamento dos padrões para matriz de LEDs

Leitura de Entradas:

Leitor ADC GPIO 28
Botão (com debounce de 50ms)

Como Executar o Projeto:

Conecte o resistor de referência de 10 k? entre 3,3 V e GP28; insira o resistor a testar entre GP28 e GND.
Ligue o OLED (GP14 SDA, GP15 SCL) e a matriz de LEDs (GP7 via PIO).
Conecte o Pico ao PC via USB.
Compile e grave o firmware no RP2040.
Ao energizar, o sistema inicia medição contínua.
Pressione B para entrar em modo BOOTSEL e atualizar o código via USB.

Autor:
Nome: Mateus Moreira da Silva

Repositório: GitHub

Vídeo de Demonstração: YouTube
