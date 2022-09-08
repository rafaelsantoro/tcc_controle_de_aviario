# tcc_controle_de_aviario
Repositório para os códigos fontes produzidos no Trabalho de Conclusão de Curso para um protótipo de 
Controle Aviário do acadêmico Rafael R. Santoro no curso de Engenharia de Biossistemas do campus IFSP-Avaré.

## English

SANTORO, Rafael Ranal. Design and construction of an electronic device to control, monitor,
record temperature, luminosity, humidity and presence of ammonia in a poultry farm. 2022.
Monograph (Bachelor’s degree in Biosystems Engineering) – IFSP – Federal Institute of São
Paulo, Avaré, 2020.


The objective of this work was to build a functional equipment for the collection of parameters,
temperature, humidity, luminosity and presence of ammonia, in aviaries, recording its data and
making it available on the internet through the MQTT protocol, following the concept of
internet of things. This device can also activate devices to control the variables, through relays.
The prototype was designed to be used in farms of small poultry producers, and can be
connected to the electrical system of fans, resistors, exhaust fans, curtains, etc. The
ESP-8266EX was used as a microcontroller on a WeMos D1 Mini board, and the MQ-135,
DHT22 and LDR GL-5528 sensors. The source code was developed on the Arduino IDE
platform, in open source format. The tests were carried out in closed and open environments,
and in the end, a prototype was obtained that kept the temperature and humidity values within
established parameters, controlled the luminosity by time criterion and values obtained by the
luminosity sensor, as well as detected with successfully the presence of ammonia.


Keywords: microcontroller; aviary, internet of things; MQTT protocol; ESP-8266EX.

## Portuguese

SANTORO, Rafael Ranal. Projeto e construção de um dispositivo eletrônico para controle,
monitoramento, registro da temperatura, luminosidade, umidade e presença de amônia em uma
granja de aves. 2022. Monografia (Bacharelado em Engenharia de Biossistemas) – IFSP –
Instituto Federal de São Paulo, Avaré, 2022.

Neste trabalho, objetivou-se a construção de um equipamento funcional para a coleta de
parâmetros tais como: temperatura, umidade, luminosidade e presença de amônia, em aviários,
registrando seus dados e disponibilizando na internet por meio de protocolo MQTT, seguindo
o conceito de internet das coisas. Podendo este dispositivo também realizar o acionamento de
dispositivos para controle das variáveis, por meio de relés. O protótipo foi projetado para ser
utilizado em granjas de pequenos produtores de aves, podendo ser conectado ao sistema elétrico
de ventiladores, resistências, exaustores, cortinas, etc. Utilizou-se como microcontrolador o
ESP-8266EX em uma placa WeMos D1 Mini, e os sensores MQ-135, DHT22 e LDR GL-5528.
O código-fonte foi desenvolvido na plataforma Arduino IDE, no formato de código livre. Os
testes foram realizados em ambiente fechado e aberto, e ao final obteve-se um protótipo que
manteve os valores de temperatura e umidade dentro de parâmetros estabelecidos, controlou a
luminosidade por critério de tempo e valores obtidos pelo sensor de luminosidade, bem como
detectou com sucesso a presença de amônia.


Palavras-chave: microcontrolador; aviário; internet das coisas; protocolo MQTT; ESP-8266EX.

