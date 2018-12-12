# colete-matchvision (Colete eletrônico para treinar jogadores de futebol)

## Pra que serve?

Para ser utilizada em treinos de futebol com o objetivo de melhorar a acuidade visual dos jogadores.


## Como funciona?

O equipamento consiste em um ESP8266 conectado a uma bateria de lítio e a fitas LED modelo WS2812b. Essas fitas tem a característica de os LEDs poderem ser controlados individualmente. Assim, foi possível construir um display de 7 segmentos que cobrisse todo o peitoral do jogador.

De acordo com a pesquisa do médico que encomendou o desenvolvimento, ao mudar o padrão escrito na camisa isso força os jogadores a não olharem para a bola e sim para a camisa dos seus companheiros, melhorando assim seu desempenho.

O dispositivo é programado para de tempos em tempos alternar os números do colete. Ao receber a bola os jogadores deveriam dizer o número da camisa de seus companheiros e ao chutar também. Esse exercício os forçam a sempre olhar pra frente, e não para o chão.


## Arquivos principais e suas funções

**/espColorWebServerWs2812b-mod.ino**

Esse arquivo único é compilado e transferido para o ESP8266. Nele contém a importação das bibliotecas que foram utilizadas, como por exemplo a de controle dos LEDs como a de controle do Wifi da placa. Contém também a definição das variáveis e da webpage que roda em um servidor na placa.

Contém as funções para gravar dados na EEPROM e de atualizar o tempo atual. Por fim contém as funções principais, setup() e loop().

