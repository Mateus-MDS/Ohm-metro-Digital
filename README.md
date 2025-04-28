Ohm�metro Digital - Indentifica��o de Resistores Comerciais da S�ria E24, Na BitDogLab

Descri��o
O ohm�metro digital mede resist�ncias de 500 ? at� 100 k? usando um divisor de tens�o e o ADC interno do Raspberry Pi Pico. O valor medido � arredondado conforme a s�rie E24 e exibido no display OLED, junto com as tr�s faixas de cores (dois d�gitos significativos e multiplicador). Simultaneamente, a matriz de LEDs 5�5 pinta cada linha principal com a cor correspondente a cada faixa. Em caso de falha na leitura, o OLED mostra mensagem de erro e os LEDs permanecem apagados.

Objetivos
Medir automaticamente resistores entre 500 ? e 100 k?.
Aproximar o valor medido � s�rie comercial E24.
Exibir no OLED o valor E24 e as cores das tr�s faixas.
Mostrar visualmente as faixas na matriz de LEDs.
Permitir atualiza��o de firmware via bot�o B (modo BOOTSEL).
Registrar log de diagn�stico via USB a cada segundo.

Componentes Utilizados
Componente - Conex�o BitDogLab - Fun��o no Projeto

Raspberry Pi Pico -	Microcontrolador principal
Resistor de 10 k? - Entre 3,3 V e GP28 - Refer�ncia fixa no divisor de tens�o
Resistor sob teste - Entre GP28 e GND - Componente cuja resist�ncia ser� medida
Display OLED SSD1306 - I�C (GP14 SDA, GP15 SCL) - Exibi��o de valor e cores
Matriz de LEDs 5�5 - PIO (GP7) - Visualiza��o das tr�s faixas de cor
Bot�o B	- GP6 - Dispara interrup��o para modo BOOTSEL
USB Serial- USB - Log de diagn�stico via terminal

Funcionamento
Modo de medi��o:
No loop principal, o Pico faz 500 leituras do ADC para calcular a m�dia da tens�o no resistor desconhecido. Com base na lei do divisor de tens�o e no resistor de refer�ncia de 10 k?, calcula-se o valor de resist�ncia R_x. Em seguida, verifica-se se R_x est� dentro da faixa v�lida (500 ? a 100 k?). Se estiver fora, o display OLED exibe uma mensagem informando que a medi��o s� funciona nesse intervalo.
Se R_x for v�lido, o programa busca no vetor da s�rie E24 o valor comercial mais pr�ximo, escolhendo aquele que apresenta o menor erro em rela��o � medi��o. A rotina de cores ent�o separa os dois primeiros d�gitos e o multiplicador desse valor, mapeando cada parte para as cores das tr�s faixas. O OLED mostra, em tr�s linhas, as cores correspondentes e o valor E24; simultaneamente, a matriz de LEDs 5�5 pinta cada linha principal com a cor de cada faixa. Por fim, um log peri�dico via USB imprime no terminal o primeiro d�gito e a resist�ncia medida para fins de diagn�stico.

Modo BOOTSEL: 
Ao pressionar o Bot�o B conectado � interrup��o, dispara-se o handler que chama reset_usb_boot(), reiniciando o Pico em modo de boot por USB. Esse modo permite atualizar o firmware diretamente, sem necessidade de reconectar manualmente o cabo no modo de inicializa��o.

L�gicas de suportes: 
Debounce e Interrup��es: O GPIO do Bot�o B usa pull-up interno e interrup��o em borda de descida para resposta imediata e sem bouncing.
Controle de Perif�ricos: I�C inicializa o SSD1306; o PIO carrega e executa o programa de anima��o da matriz de LEDs; o ADC � configurado para entrada anal�gica no pino 28.
Fluxo de Loop: O loop principal alterna entre leitura do ADC, c�lculo de resist�ncia, atualiza��o de display/LEDs e log USB, garantindo responsividade e clareza na indica��o do valor medido.

Estrutura do C�digo
Inicializa��o:

Configura��o de GPIOs, ADC, I2C e PIO
Inicializa��o do display OLED
Carregamento dos padr�es para matriz de LEDs

Leitura de Entradas:

Leitor ADC GPIO 28
Bot�o (com debounce de 50ms)

Como Executar o Projeto:

Conecte o resistor de refer�ncia de 10 k? entre 3,3 V e GP28; insira o resistor a testar entre GP28 e GND.
Ligue o OLED (GP14 SDA, GP15 SCL) e a matriz de LEDs (GP7 via PIO).
Conecte o Pico ao PC via USB.
Compile e grave o firmware no RP2040.
Ao energizar, o sistema inicia medi��o cont�nua.
Pressione B para entrar em modo BOOTSEL e atualizar o c�digo via USB.

Autor:
Nome: Mateus Moreira da Silva

Reposit�rio: GitHub

V�deo de Demonstra��o: YouTube
