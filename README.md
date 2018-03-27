## ESP8266 Wifi - Arduino IDE - Ligar/Desligar LED



Steps para rodar o código local na máquina:

1. Instalar Arduino IDE:
https://www.arduino.cc/en/Main/Software

2. Pegar código do GitHub:
https://github.com/nathalials/nodemcu-iot/blob/master/NodeMCUIoT.ino

    2.1.  Alterar as credenciais do wifi (linha 26 e 27)

    2.2.  Alterar a org (linha 33):
        
    Obs.: Sua org pode ser encontrada no início da URL https://**iv9si4**.internetofthings.ibmcloud.com/

    2.3. Configurar device type, device ID e token: conforme o que foi colocado na plataforma (linha 34, 35 e 36).


3. Instalar board ESP8266WiFi:

    3.1. Abrir Arduino IDE, selecionar Arduino, preferences.
    
    3.2. Na janela de preferências, no campo `Additional Boards Manager URLs`, colocar o link: http://arduino.esp8266.com/stable/package_esp8266com_index.json  e depois clicar em OK.

    3.3. No Arduino IDE, selecionar Tools - Board - Boards Manager.
        
    3.4. No campo de busca, digitar: `esp8266`. Selecionar board esp8266 e instalar.
        
4. Instalar biblioteca PubSubClient:
        
    4.1. No Arduino IDE, selecionar Sketch - Include Library - Manage Libraries.
        
    4.2. Digitar `PubSubClient` no campo de buscar. Selecionar PubSubClient e instalar biblioteca.

5. Instalar biblioteca: Adafruit BMP085

    5.1. Selecionar Sketch - Include library - Manage libraries
    
    5.2. Digitar `adafruit bmp085 library` no campo de busca. Selecionar adafruit bmp085 library e instalar biblioteca.
 
 6. Clicar em Verify (verificar código)
 
 7. Clicar em Upload (rodar código)


