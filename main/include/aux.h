
#ifndef __AUX_H__
#define __AUX_H__

const char *HTML = (const char*) "<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\">\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <link rel=\"icon\" type=\"image/x-icon\" href=\"data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAD//////Pz+//////+JiNP/LCq0/z07uf8wL7T/oqLc///////8/P7///////////////////////////////////////v7/P////3/zc3p/5aV1v+wsOH/T06//0NBuv///////f3+///////////////////////////////////////o5+T/cmdY/350X/+Adlr/nJNw/7u63v8ZGK3/7Oz3///////+/v///////////////////////////////////////2pfTv+oopn/393c/15SQP/Ixcb/JiS4/7Ky4f///////Pz+/////////////////////////////fz8//////+TjID/kYl8//////99dGf/s62X/1hYzf9qacf///////v7/v////////////////////////////38/P//////w764/2hdTP//////raeg/4qBZf+Xl97/Ly6z///////+/v/////////////////////////////+/v7//////+zr6f9hVUP/5OPg/+Lg3v9rX0f/w8La/xkYr//e3vL///////39/v////////////////////////////7+/v//////dmxd/7Grov//////al9O/8bCuf8yMr3/nZzZ///////8/P7////////////////////////////9/Pz//////6GakP+Admj//////4qBd/+ooIf/bW3U/1ZVv///////+vr9//3+/v/9/f7//v7//////////////f39///////Qzcj/Y1dG//////++ubT/f3Ra/6qp4P8lI7D//v7+///////+/v////////7+//////////////38/P//////9PPy/2NXRv/W087/8fDw/2dbRv/Ix9P/GRiv/7W04P/o6PX/3d3y/+Hh9P/09Pv//////////////////f39//////+HfnH/oZuQ//////9xZ1j/vris/2ho2v8iIbb/NjW//zU0vf8PDaj/hYTS///////8/Pv/2dfS/87Lxf/Y1tH/kIh7/3BmVf//////npaM/3huXv/Qzs7/ure9/7i0vP/Y1tP/np3b/zY1tf///////////3lvYP9qXk3/fnRl/3xyY/+FfG7/+fn4/9vZ1f92a1z/fHJi/4B2ZP95b13/cGRM/9jX4/9bWcb/7O33//////+blIj/n5mN///////9/Pz//v7+//7+/v/////////////////+/v7//////3lvYP/Cvrj///////v7/P//////zcnE/5yViv///////f39//7+/v///////v7+//7+/v///v7//fz8//////+xq6L/t7Kq///////9/fz/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==\">\
    <!--meta http-equiv=\"refresh\" content=\"1\"-->\
    <title>Web Server</title>\
    <style>\
      body {\
        font-family: 'Open Sans', sans-serif;\
        color: #333;\
        line-height: 1.6;\
        margin: 0;\
        padding: 20px;\
        background-color: #f8f8f8;\
      }\
      h1, h2 {\
        font-family: 'Montserrat', sans-serif;\
        color: #007bff;\
        margin: 0;\
      }\
      .container {\
        max-width: 960px;\
        margin: 28px auto;\
        padding: 20px;\
        background-color: #fff;\
        box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);\
        border-radius: 8px;\
      }\
      button {\
        background-color: #28a745;\
        color: #fff;\
        font-weight: bold;\
        padding: 4px 24px;\
        margin: 12px 0px 8px;\
        border: none;\
        border-radius: 5px;\
        cursor: pointer;\
        transition: background-color 0.3s ease;\
      }\
      button:hover {\
        background-color: #218838;\
      }\
      table {\
        width: 90%;\
        border-collapse: collapse;\
        margin: 20px auto;\
        background-color: #fff;\
        box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);\
        border-radius: 8px;\
        overflow: hidden;\
      }\
      th, td {\
        padding: 12px 15px;\
        text-align: left;\
        border-bottom: 1px solid #ddd;\
      }\
      th {\
        background-color: #007bff;\
        color: white;\
        font-weight: bold;\
      }\
      tr:nth-child(even) {\
        background-color: #f2f2f2;\
      }\
      tr:hover {\
        background-color: #e9e9e9;\
      }\
      tbody tr:last-child td {\
        border-bottom: none;\
      }\
    </style>\
  </head>\
  <body>\
    <div class=\"container\">\
      <h1>Web Server</h1>\
      <button type=\"button\" class=\"config\" onclick=\"location.href='/ssid_new'\">Configuração</button>\
      <h3>Sensor de Temperatura</h3>\
      <div id=\"info\"></div>\
      <div id=\"temperature\"></div>\
    </div>\
    <script>\
      setTimeout(function() {\
        fetch('/info')\
        .then(response => { return response.text(); })\
        .then(data => {\
          document.querySelector('#info').innerHTML = `<p><strong>Informações do AP:</strong> ${data}</p>`;\
        })\
        .catch(error => { /* console.log('Error fetching /info:', error); */ });\
      }, 2000);\
      var intervalId = setInterval(function() {\
        fetch('/temperature')\
        .then(response => { return response.text(); })\
        .then(data => {\
          document.querySelector('#temperature').innerHTML = `<p><strong>Temperatura do Nó:</strong> ${data} °C</p>`;\
        })\
        .catch(error => { /* console.log('Error fetching /temperature:', error); */ });\
      }, 1000);\
    </script>\
  </body>\
</html>\
";

static const char* HTML_NEW = "\
<!DOCTYPE html>\
<html lang=\"en\">\
<head>\
  <meta charset=\"UTF-8\">\
  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
  <link rel=\"icon\" type=\"image/x-icon\" href=\"data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAD//////Pz+//////+JiNP/LCq0/z07uf8wL7T/oqLc///////8/P7///////////////////////////////////////v7/P////3/zc3p/5aV1v+wsOH/T06//0NBuv///////f3+///////////////////////////////////////o5+T/cmdY/350X/+Adlr/nJNw/7u63v8ZGK3/7Oz3///////+/v///////////////////////////////////////2pfTv+oopn/393c/15SQP/Ixcb/JiS4/7Ky4f///////Pz+/////////////////////////////fz8//////+TjID/kYl8//////99dGf/s62X/1hYzf9qacf///////v7/v////////////////////////////38/P//////w764/2hdTP//////raeg/4qBZf+Xl97/Ly6z///////+/v/////////////////////////////+/v7//////+zr6f9hVUP/5OPg/+Lg3v9rX0f/w8La/xkYr//e3vL///////39/v////////////////////////////7+/v//////dmxd/7Grov//////al9O/8bCuf8yMr3/nZzZ///////8/P7////////////////////////////9/Pz//////6GakP+Admj//////4qBd/+ooIf/bW3U/1ZVv///////+vr9//3+/v/9/f7//v7//////////////f39///////Qzcj/Y1dG//////++ubT/f3Ra/6qp4P8lI7D//v7+///////+/v////////7+//////////////38/P//////9PPy/2NXRv/W087/8fDw/2dbRv/Ix9P/GRiv/7W04P/o6PX/3d3y/+Hh9P/09Pv//////////////////f39//////+HfnH/oZuQ//////9xZ1j/vris/2ho2v8iIbb/NjW//zU0vf8PDaj/hYTS///////8/Pv/2dfS/87Lxf/Y1tH/kIh7/3BmVf//////npaM/3huXv/Qzs7/ure9/7i0vP/Y1tP/np3b/zY1tf///////////3lvYP9qXk3/fnRl/3xyY/+FfG7/+fn4/9vZ1f92a1z/fHJi/4B2ZP95b13/cGRM/9jX4/9bWcb/7O33//////+blIj/n5mN///////9/Pz//v7+//7+/v/////////////////+/v7//////3lvYP/Cvrj///////v7/P//////zcnE/5yViv///////f39//7+/v///////v7+//7+/v///v7//fz8//////+xq6L/t7Kq///////9/fz/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==\">\
  <title>Configuração WIFI</title>\
  <style>\
    body {\
      align-items: center;\
      background-color: #000;\
      display: flex;\
      justify-content: center;\
      height: 100vh;\
    }\
    .form {\
      background-color: #15172b;\
      border-radius: 20px;\
      box-sizing: border-box;\
      height: 500px;\
      padding: 20px;\
      width: 320px;\
    }\
    .title {\
      color: #eee;\
      font-family: sans-serif;\
      font-size: 36px;\
      font-weight: 600;\
      margin-top: 30px;\
    }\
    .subtitle {\
      color: #eee;\
      font-family: sans-serif;\
      font-size: 16px;\
      font-weight: 600;\
      margin-top: 10px;\
    }\
    .input-container {\
      height: 50px;\
      position: relative;\
      width: 100%;\
    }\
    .ic1 {\
      margin-top: 40px;\
    }\
    .ic2 {\
      margin-top: 30px;\
    }\
    .input {\
      background-color: #303245;\
      border-radius: 12px;\
      border: 0;\
      box-sizing: border-box;\
      color: #eee;\
      font-size: 18px;\
      height: 100%;\
      outline: 0;\
      padding: 4px 20px 0;\
      width: 100%;\
    }\
    .cut {\
      background-color: #15172b;\
      border-radius: 10px;\
      height: 20px;\
      left: 20px;\
      position: absolute;\
      top: -20px;\
      transform: translateY(0);\
      transition: transform 200ms;\
      width: 76px;\
    }\
    .cut-short {\
      width: 50px;\
    }\
    .input:focus~.cut,\
    .input:not(:placeholder-shown)~.cut {\
      transform: translateY(8px);\
    }\
    .placeholder {\
      color: #65657b;\
      font-family: sans-serif;\
      left: 20px;\
      line-height: 14px;\
      pointer-events: none;\
      position: absolute;\
      transform-origin: 0 50%;\
      transition: transform 200ms, color 200ms;\
      top: 20px;\
    }\
    .input:focus~.placeholder,\
    .input:not(:placeholder-shown)~.placeholder {\
      transform: translateY(-30px) translateX(10px) scale(0.75);\
    }\
    .input:not(:placeholder-shown)~.placeholder {\
      color: #808097;\
    }\
    .input:focus~.placeholder {\
      color: #dc2f55;\
    }\
    .submit {\
      background-color: #08d;\
      border-radius: 12px;\
      border: 0;\
      box-sizing: border-box;\
      color: #eee;\
      cursor: pointer;\
      font-size: 18px;\
      height: 50px;\
      margin-top: 38px;\
      text-align: center;\
      width: 100%;\
    }\
    .reset {\
      background-color: #F54927;\
    }\
    .submit:active {\
      background-color: #06b;\
    }\
  </style>\
</head>\
<body>\
  <form action=\"/ssid_create\" method=\"POST\" enctype=\"application/x-www-form-urlencoded\">\
    <div class=\"title\">Conexão WIFI de Origem</div>\
    <div class=\"subtitle\">Cadastrar Nova Configuração de Conexão</div>\
    <div class=\"input-container ic1\">\
      <input id=\"ssid_name\" name=\"ssid_name\"  class=\"input\" type=\"text\" placeholder=\" \" />\
      <div class=\"cut\"></div>\
      <label for=\"ssid_name\" class=\"placeholder\">Nome da Rede</label>\
    </div>\
    <div class=\"input-container ic2\">\
      <input id=\"ssid_password\" name=\"ssid_password\" class=\"input\" type=\"password\" placeholder=\" \" />\
      <div class=\"cut\"></div>\
      <label for=\"ssid_password\" class=\"placeholder\">Senha da Rede</label>\
    </div>\
    <div class=\"input-container ic2\">\
      <input id=\"ap_password\" name=\"ap_password\" class=\"input\" type=\"password\" placeholder=\" \" />\
      <div class=\"cut\"></div>\
      <label for=\"ap_password\" class=\"placeholder\">Senha Interna do Ponto de Acesso Mesh</label>\
    </div>\
    <div class=\"input-container ic2\">\
      <input id=\"ap_id\" name=\"ap_id\" class=\"input\" type=\"number\" placeholder=\" \" />\
      <div class=\"cut\"></div>\
      <label for=\"ap_id\" class=\"placeholder\">ID do Ponto de Acesso Mesh</label>\
    </div>\
    <!-- input type=\"hidden\" name=\"hidden_field\" value=\"hidden_field\" -->\
    <button type=\"text\" class=\"submit\">salvar</button>\
    <button type=\"reset\" class=\"submit reset\" onclick=\"javascript:document.querySelector('form').submit();\">reset</button>\
  </form>\
</body>\
</html>\
";

const char *HTML_LED = (const char*) "<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\">\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    <link rel=\"icon\" type=\"image/x-icon\" href=\"data:image/x-icon;base64,AAABAAEAEBAAAAEAIABoBAAAFgAAACgAAAAQAAAAIAAAAAEAIAAAAAAAAAQAAAAAAAAAAAAAAAAAAAAAAAD//////Pz+//////+JiNP/LCq0/z07uf8wL7T/oqLc///////8/P7///////////////////////////////////////v7/P////3/zc3p/5aV1v+wsOH/T06//0NBuv///////f3+///////////////////////////////////////o5+T/cmdY/350X/+Adlr/nJNw/7u63v8ZGK3/7Oz3///////+/v///////////////////////////////////////2pfTv+oopn/393c/15SQP/Ixcb/JiS4/7Ky4f///////Pz+/////////////////////////////fz8//////+TjID/kYl8//////99dGf/s62X/1hYzf9qacf///////v7/v////////////////////////////38/P//////w764/2hdTP//////raeg/4qBZf+Xl97/Ly6z///////+/v/////////////////////////////+/v7//////+zr6f9hVUP/5OPg/+Lg3v9rX0f/w8La/xkYr//e3vL///////39/v////////////////////////////7+/v//////dmxd/7Grov//////al9O/8bCuf8yMr3/nZzZ///////8/P7////////////////////////////9/Pz//////6GakP+Admj//////4qBd/+ooIf/bW3U/1ZVv///////+vr9//3+/v/9/f7//v7//////////////f39///////Qzcj/Y1dG//////++ubT/f3Ra/6qp4P8lI7D//v7+///////+/v////////7+//////////////38/P//////9PPy/2NXRv/W087/8fDw/2dbRv/Ix9P/GRiv/7W04P/o6PX/3d3y/+Hh9P/09Pv//////////////////f39//////+HfnH/oZuQ//////9xZ1j/vris/2ho2v8iIbb/NjW//zU0vf8PDaj/hYTS///////8/Pv/2dfS/87Lxf/Y1tH/kIh7/3BmVf//////npaM/3huXv/Qzs7/ure9/7i0vP/Y1tP/np3b/zY1tf///////////3lvYP9qXk3/fnRl/3xyY/+FfG7/+fn4/9vZ1f92a1z/fHJi/4B2ZP95b13/cGRM/9jX4/9bWcb/7O33//////+blIj/n5mN///////9/Pz//v7+//7+/v/////////////////+/v7//////3lvYP/Cvrj///////v7/P//////zcnE/5yViv///////f39//7+/v///////v7+//7+/v///v7//fz8//////+xq6L/t7Kq///////9/fz/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==\">\
    <!--meta http-equiv=\"refresh\" content=\"1\"-->\
    <title>Web Server</title>\
    <style>\
      body {\
        font-family: 'Open Sans', sans-serif;\
        color: #333;\
        line-height: 1.6;\
        margin: 0;\
        padding: 20px;\
        background-color: #f8f8f8;\
      }\
      h1, h2 {\
        font-family: 'Montserrat', sans-serif;\
        color: #007bff;\
        margin: 0;\
      }\
      .container {\
        max-width: 960px;\
        margin: 28px auto;\
        padding: 20px;\
        background-color: #fff;\
        box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);\
        border-radius: 8px;\
      }\
      button {\
        background-color: #28a745;\
        color: #fff;\
        padding: 10px 20px;\
        border: none;\
        border-radius: 5px;\
        cursor: pointer;\
        transition: background-color 0.3s ease;\
      }\
      button:hover {\
        background-color: #218838;\
      }\
      .slidecontainer {\
        width: 100%;\
      }\
    </style>\
  </head>\
  <body>\
    <div class=\"container\">\
      <h1>Teste LED</h1>\
      <div id=\"led\">\
        <label for=\"slider_r\">R: 0</label>\
        <input type=\"range\" id=\"slider_r\" class=\"slidecontainer\" name=\"slider_r\" min=\"0\" max=\"255\" value=\"0\" oninput=\"this.previousElementSibling.innerText = `R: ${this.value}`\">\
        <label for=\"slider_g\">G: 0</label>\
        <input type=\"range\" id=\"slider_g\" class=\"slidecontainer\" name=\"slider_g\" min=\"0\" max=\"255\" value=\"0\" oninput=\"this.previousElementSibling.innerText = `G: ${this.value}`\">\
        <label for=\"slider_b\">B: 0</label>\
        <input type=\"range\" id=\"slider_b\" class=\"slidecontainer\" name=\"slider_b\" min=\"0\" max=\"255\" value=\"0\" oninput=\"this.previousElementSibling.innerText = `B: ${this.value}`\">\
      </div>\
    </div>\
    <script>\
      const sliders = document.querySelectorAll('.slidecontainer');\
      sliders.forEach(slider => {\
        slider.addEventListener('change', () => {\
          let r = document.getElementById('slider_r').value;\
          let g = document.getElementById('slider_g').value;\
          let b = document.getElementById('slider_b').value;\
          fetch(`/led_on?r=${r}&g=${g}&b=${b}`)\
            .then(response => response.text())\
            .catch(error => console.error('Error:', error));\
        });\
      });\
    </script>\
  </body>\
</html>\
";

char* concat(char* _str1, char* _str2);
char* to_s(int _num);
char* float_to_s(float _num);
char** split(char* _str, char _delim);

#endif /* __AUX_H__ */