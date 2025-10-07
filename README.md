# Meu projeto mesh

Permite criar rede mesh usando microcontroladores ESP32. Projeto baseado no guia https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/esp-wifi-mesh.html

Esse projeto permite configurar o nome e a senha da rede sem a necessidade de compilar o firmware.

Ao iniciar pela primeira vez o dispositivo gera um ponto de acesso virtual "myssid" sem senha. E através do ip 192.168.4.1 é possível cadastrar o nome e a senha da internet local pela tela de configuração. Então o dispositivo irá reiniciar e criar ponto de acesso da rede internet.

Para bons resultados recomenda-se ao menos 3 dispositivos, já que um será o nó raiz e deve ficar próximo ao roteador de internet. Para mais informações acessso a documentação da Espressif Systems.

Foi utilizado o modelo ESP32 S3 nos testes.
