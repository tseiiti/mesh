#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <nvs_flash.h>

#include "esp_http_server.h"
#include "esp_http_client.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_netif.h"
#include "esp_spiffs.h"
#include "esp_mesh.h"

#include "driver/temperature_sensor.h"
#include "protocol_examples_common.h"
#include "mesh_light.h"
#include "led_strip.h"
#include "aux.h"

#define CONFIG_FILE_PATH "/spiffs/config.txt"
#define RESET_FILE_PATH "/spiffs/reset.txt"

static const char *TAG_SPI    = "tag_spiffs";
static const char *TAG_SERVER = "tag_server";
static const char *TAG_AP     = "tag_ap";
static const char *TAG        = "my_app";

char* ssid_name               = NULL;
char* ssid_password           = NULL;
char* ap_password             = NULL;
char* ap_id                   = NULL;
char* ap_ip                   = NULL;
char* ap_mac                  = NULL;

int reset_flag = 4;
struct stat st;

temperature_sensor_handle_t temp_sensor = NULL;
temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);

esp_chip_info_t chip_info;
uint32_t flash_size;



/*******************************************************
 * Tratamento do Arquivo de Configuração
 *******************************************************/

// Estrutura para montagem da partição usada para armazenar arquivos
static esp_vfs_spiffs_conf_t spiffs_conf = {
  .base_path = "/spiffs",
  .partition_label = NULL,
  .max_files = 5,
  .format_if_mount_failed = true
};

// Verificação do ponto de montagem (pasta de arquivos)
static esp_err_t check_spiffs(void) {
  ESP_LOGI(TAG_SPI, "Initializing SPIFFS");

  esp_err_t ret = esp_vfs_spiffs_register(&spiffs_conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG_SPI, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG_SPI, "Failed to find SPIFFS partition");
    } else {
      ESP_LOGE(TAG_SPI, "Failed to initialize SPIFFS (%s)",
               esp_err_to_name(ret));
    }
    return ret;
  }

  ESP_LOGI(TAG_SPI, "Performing SPIFFS_check().");
  ret = esp_spiffs_check(spiffs_conf.partition_label);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG_SPI, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
    return ret;
  } else {
    ESP_LOGI(TAG_SPI, "SPIFFS_check() successful");
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info(spiffs_conf.partition_label, &total, &used);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG_SPI,
             "Failed to get SPIFFS partition information (%s). Formatting...",
             esp_err_to_name(ret));
    esp_spiffs_format(spiffs_conf.partition_label);
    return ret;
  } else {
    ESP_LOGI(TAG_SPI, "Partition size: total: %d, used: %d", total, used);
  }

  if (used > total) {
    ESP_LOGW(TAG_SPI,
             "Number of used bytes cannot be larger than total. Performing "
             "SPIFFS_check().");
    ret = esp_spiffs_check(spiffs_conf.partition_label);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG_SPI, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
      return ret;
    } else {
      ESP_LOGI(TAG_SPI, "SPIFFS_check() successful");
    }
  }
  return ret;
}

// Salva arquivo de configuração
static esp_err_t save_config_file(char* _prm) {
  ESP_LOGI(TAG_SPI, "Config file save");

  char** tokens = split(_prm, '&');
  if (tokens) {
    if (stat(CONFIG_FILE_PATH, &st) == 0) {
      unlink(CONFIG_FILE_PATH);
    }
    
    FILE *f = fopen(CONFIG_FILE_PATH, "w");
    if (f == NULL) {
      ESP_LOGE(TAG_SPI, "Failed to open file for writing");
      return ESP_FAIL;
    }
    
    for (int i = 0; *(tokens + i); i++) {
      fprintf(f, "%s\n", *(tokens + i));
      free(*(tokens + i));
    }

    fclose(f);
    free(tokens);

    ESP_LOGI(TAG_SPI, "File written");
  }

  return ESP_OK;
}

// Lê arquivo de configuração
static esp_err_t read_config_file(void) {
  ESP_LOGI(TAG_SPI, "Reading file: %s", CONFIG_FILE_PATH);

  if (stat(CONFIG_FILE_PATH, &st) != 0) {
    return ESP_OK;
  }

  FILE *f = fopen(CONFIG_FILE_PATH, "r");
  if (f == NULL) {
    ESP_LOGE(TAG_SPI, "Failed to open file for reading");
    return ESP_FAIL;
  }
  
  char line[100];
  while (fgets(line, sizeof(line), f)) {
    line[strcspn(line, "\n")] = 0;
    char** prm = split(line, '=');
    if (strcmp(*(prm + 0), "ssid_name") == 0) {
      ssid_name = *(prm + 1);
    }
    if (strcmp(*(prm + 0), "ssid_password") == 0) {
      ssid_password = *(prm + 1);
    }
    if (strcmp(*(prm + 0), "ap_password") == 0) {
      ap_password = *(prm + 1);
    }
    if (strcmp(*(prm + 0), "ap_id") == 0) {
      ap_id = *(prm + 1);
    }
    free(prm);
  }
  fclose(f);
  
  return ESP_OK;
}

// Salva arquivo de reset
static esp_err_t save_reset_file(void) {
  ESP_LOGI(TAG_SPI, "Reset file save");

  FILE *f;
  if (stat(RESET_FILE_PATH, &st) == 0) {
    unlink(RESET_FILE_PATH);
  }

  f = fopen(RESET_FILE_PATH, "w");
  if (f == NULL) {
    ESP_LOGE(TAG_SPI, "Failed to open file for writing");
    return ESP_FAIL;
  }

  fprintf(f, "%d", reset_flag);

  fclose(f);
  ESP_LOGI(TAG_SPI, "Integer saved to %s\n", RESET_FILE_PATH);

  return ESP_OK;
}

// Lê arquivo de reset
static esp_err_t read_reset_file(void) {
  ESP_LOGI(TAG_SPI, "Reading file");

  if (stat(RESET_FILE_PATH, &st) != 0) {
    return ESP_OK;
  }

  FILE *f = fopen(RESET_FILE_PATH, "r");
  if (f == NULL) {
    ESP_LOGE(TAG_SPI, "Failed to open file for reading");
    return ESP_FAIL;
  }

  fscanf (f, "%d", &reset_flag);
  fclose(f);
  
  return ESP_OK;
}


/*******************************************************
 * Tratamento do Servidor Web
 *******************************************************/

// página root
static esp_err_t root_handler(httpd_req_t *req) {
  ESP_LOGI(TAG_SERVER, "consumindo root");

  httpd_resp_set_type(req, "text/html");
  httpd_resp_send_chunk(req, HTML, -1);
  httpd_resp_send_chunk(req, NULL, 0);

  return ESP_OK;
}

// Estrura da página get para cadastro de configuração wifi (root)
static const httpd_uri_t root = {
  .uri = "/",
  .method = HTTP_GET,
  .handler = root_handler,
  .user_ctx = NULL
};

// página de temperature
static esp_err_t temperature_get_handler(httpd_req_t *req) {
  // ESP_LOGI(TAG_SERVER, "consumindo temperature");
  float tsens_value;
  ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_sensor, &tsens_value));
  char *buf = float_to_s(tsens_value);
  
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_send_chunk(req, buf, -1);
  httpd_resp_send_chunk(req, NULL, 0);

  free(buf);

  return ESP_OK;
}

// Estrutura da página de temperature
static const httpd_uri_t temperature = {
  .uri = "/temperature",
  .method = HTTP_GET,
  .handler = temperature_get_handler,
  .user_ctx = NULL
};

// página de info
static esp_err_t info_get_handler(httpd_req_t *req) {
  // ESP_LOGI(TAG_SERVER, "consumindo info");

  esp_chip_info(&chip_info);
  esp_flash_get_size(NULL, &flash_size);
  unsigned major_rev = chip_info.revision / 100;
  unsigned minor_rev = chip_info.revision % 100;

  char *buf = "Model: ";
  buf = concat(buf, CONFIG_IDF_TARGET);
  buf = concat(buf, ", ");
  buf = concat(buf, to_s(chip_info.cores));
  buf = concat(buf, " CPU core(s)");
  buf = concat(buf, (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? ", WiFi/" : "");
  buf = concat(buf, (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "");
  buf = concat(buf, (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "");
  buf = concat(buf, (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");
  buf = concat(buf, ", silicon revision v");
  buf = concat(buf, to_s(major_rev));
  buf = concat(buf, to_s(minor_rev));
  buf = concat(buf, ", ");
  buf = concat(buf, to_s(flash_size / (1024 * 1024)));
  buf = concat(buf, "MB ");
  buf = concat(buf, (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
  buf = concat(buf, " flash. Minimum free heap size: ");
  buf = concat(buf, to_s(esp_get_minimum_free_heap_size()));
  buf = concat(buf, "bytes");

  httpd_resp_set_type(req, "text/text");
  httpd_resp_send_chunk(req, buf, -1);
  httpd_resp_send_chunk(req, NULL, 0);

  free(buf);

  return ESP_OK;
}

// Estrutura da página de info
static const httpd_uri_t info = {
  .uri = "/info",
  .method = HTTP_GET,
  .handler = info_get_handler,
  .user_ctx = NULL
};

// Página get para cadastro de configuração wifi
static esp_err_t ssid_new_handler(httpd_req_t *req) {
  const char *response = (const char *) HTML_NEW;
  esp_err_t error = httpd_resp_send(req, response, strlen(response));
  if (error == ESP_OK) {
    ESP_LOGI(TAG_SERVER, "SSID New - Response sent Successfully");
  } else {
    ESP_LOGI(TAG_SERVER, "SSID New - Error %d while sending Response", error);
  }
  return error;
}

// Estrura da página get para cadastro de configuração wifi
static const httpd_uri_t ssid_new = {
  .uri = "/ssid_new",
  .method = HTTP_GET,
  .handler = ssid_new_handler,
  .user_ctx = NULL
};

// Página post para cadastro de configuração wifi
static esp_err_t ssid_create_handler(httpd_req_t *req) {
  char buf[100];
  int ret, remaining = req->content_len;
  char *prm = "";

  if (remaining > 0) {
    while (remaining > 0) {
      if ((ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)))) <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
          continue;
        }
        return ESP_FAIL;
      }
      prm = concat(prm, buf);
      remaining -= ret;
    }

    // Salva arquivo de configuração
    ret = save_config_file(prm);

    if (ret != ESP_OK) {
      return ret;
    }
  }

  char *response = NULL;
  if (ap_ip == NULL) {
    response = "<html><body><h3 style=\"font-family: 'Open Sans', sans-serif; color: #333; margin: 40px;\">\
      As configura&ccedil;&otilde;es foram atualizadas. Favor conectar na rede.</h3></body></html>";
  } else {
    response = "<html><head><script>setTimeout(function(){ window.location.href = '/'; }, 10000);</script></head>\
      <body><h3 style=\"font-family: 'Open Sans', sans-serif; color: #333; margin: 40px;\">\
      Reiniciando... Aguarde!</h3></body></html>";
    httpd_resp_set_hdr(req, "Location", "/");
  }

  esp_err_t error = httpd_resp_send(req, (const char *) response, strlen(response));
  if (error == ESP_OK) {
    ESP_LOGI(TAG_SERVER, "SSID New - Response sent Successfully");
  } else {
    ESP_LOGI(TAG_SERVER, "SSID New - Error %d while sending Response", error);
  }

  for (int i = 5; i >= 0; i--) {
    printf("Restarting in %d seconds...\n", i);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  esp_restart();
  return error;
}

// Estrura da página post para cadastro de configuração wifi
static const httpd_uri_t ssid_create = {
  .uri = "/ssid_create",
  .method = HTTP_POST,
  .handler = ssid_create_handler,
  .user_ctx = NULL
};

// Página de configuração do LED
static esp_err_t led_handler(httpd_req_t *req) {
  const char *response = (const char *) HTML_LED;
  esp_err_t error = httpd_resp_send(req, response, strlen(response));
  if (error == ESP_OK) {
    ESP_LOGI(TAG_SERVER, "LED On - Response sent Successfully");
  } else {
    ESP_LOGI(TAG_SERVER, "LED On - Error %d while sending Response", error);
  }
  return error;
}

// Estrura da página get para cadastro de configuração wifi
static const httpd_uri_t led = {
  .uri = "/led",
  .method = HTTP_GET,
  .handler = led_handler,
  .user_ctx = NULL
};



// API led_on
#define BLINK_GPIO 48
#define QUERY_KEY_MAX_LEN 64
static led_strip_handle_t led_strip;

static esp_err_t led_on_handler(httpd_req_t *req) {
  char *buf;
  size_t buf_len;
  int r = 0, g = 0, b = 0;

  buf_len = httpd_req_get_url_query_len(req) + 1;
  if (buf_len > 1) {
    buf = malloc(buf_len);
    ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
      ESP_LOGI(TAG, "Found URL query => %s", buf);
      char param[QUERY_KEY_MAX_LEN] = {0};
      if (httpd_query_key_value(buf, "r", param, sizeof(param)) == ESP_OK) {
        r = atoi(param);
      }
      if (httpd_query_key_value(buf, "g", param, sizeof(param)) == ESP_OK) {
        g = atoi(param);
      }
      if (httpd_query_key_value(buf, "b", param, sizeof(param)) == ESP_OK) {
        b = atoi(param);
      }
      
      led_strip_set_pixel(led_strip, 0, r, g, b);
      led_strip_refresh(led_strip);
      // vTaskDelay(3000 / portTICK_PERIOD_MS);
      // led_strip_clear(led_strip);
    }
    free(buf);
  }

  httpd_resp_set_type(req, "text/text");
  httpd_resp_send_chunk(req, NULL, 0);

  return ESP_OK;
}

// Estrura da página led_on
static const httpd_uri_t led_on = {
  .uri = "/led_on",
  .method = HTTP_GET,
  .handler = led_on_handler,
  .user_ctx = NULL
};

// Inicia e registra as páginas no servidor web
static httpd_handle_t start_webserver(void) {
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  ESP_LOGI(TAG_SERVER, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    ESP_LOGI(TAG_SERVER, "Registering URI handlers");
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &temperature);
    httpd_register_uri_handler(server, &info);
    httpd_register_uri_handler(server, &ssid_new);
    httpd_register_uri_handler(server, &ssid_create);
    httpd_register_uri_handler(server, &led);
    httpd_register_uri_handler(server, &led_on);
    return server;
  }

  ESP_LOGI(TAG_SERVER, "Error starting server!");
  return NULL;
}

// Encerra o servidor web
static esp_err_t stop_webserver(httpd_handle_t server) {
  return httpd_stop(server);
}

// Manipulador de desconexão
static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  httpd_handle_t *server = (httpd_handle_t *)arg;
  if (*server) {
    ESP_LOGI(TAG_SERVER, "Stopping webserver");
    if (stop_webserver(*server) == ESP_OK) {
      *server = NULL;
    } else {
      ESP_LOGE(TAG_SERVER, "Failed to stop http server");
    }
  }
}

// Manipulador de conexão
static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data) {
  httpd_handle_t *server = (httpd_handle_t *)arg;
  if (*server == NULL) {
    ESP_LOGI(TAG_SERVER, "Starting webserver");
    *server = start_webserver();
  }
}



/*******************************************************
 * Tratamento Access Point sem Internet
 *******************************************************/

// Manipulador de eventos do ap wifi (sem internet)
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_id == WIFI_EVENT_AP_STACONNECTED) {
    wifi_event_ap_staconnected_t *event =
        (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG_AP, "station " MACSTR " join, AID=%d", MAC2STR(event->mac),
             event->aid);
  } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
    wifi_event_ap_stadisconnected_t *event =
        (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG_AP, "station " MACSTR " leave, AID=%d, reason=%d",
             MAC2STR(event->mac), event->aid, event->reason);
  }
}

// Inicializa o SoftAP
static void wifi_init_softap(void) {
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

  wifi_config_t wifi_config = {
    .ap = {
      .ssid = CONFIG_ESP_WIFI_SSID,
      .ssid_len = strlen(CONFIG_ESP_WIFI_SSID),
      .channel = CONFIG_ESP_WIFI_CHANNEL,
      .password = CONFIG_ESP_WIFI_PASSWORD,
      .max_connection = CONFIG_ESP_MAX_STA_CONN,
      .authmode = WIFI_AUTH_WPA2_PSK,
      .pmf_cfg = {
        .required = true,
      },
      .gtk_rekey_interval = CONFIG_ESP_GTK_REKEY_INTERVAL,
    },
  };
  if (strlen(CONFIG_ESP_WIFI_PASSWORD) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
           CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD,
           CONFIG_ESP_WIFI_CHANNEL);
}



/*******************************************************
 * Tratamento Rede Mesh
 *******************************************************/

#define RX_SIZE          (1500)
#define TX_SIZE          (1460)

static const char *MESH_TAG = "mesh_main";
static uint8_t tx_buf[TX_SIZE] = { 0, };
static uint8_t rx_buf[RX_SIZE] = { 0, };
static bool is_running = true;
static bool is_mesh_connected = false;
static mesh_addr_t mesh_parent_addr;
static int mesh_layer = -1;
static esp_netif_t *netif_sta = NULL;

mesh_light_ctl_t light_on = {
  .cmd = MESH_CONTROL_CMD,
  .on = 1,
  .token_id = MESH_TOKEN_ID,
  .token_value = MESH_TOKEN_VALUE,
};

mesh_light_ctl_t light_off = {
  .cmd = MESH_CONTROL_CMD,
  .on = 0,
  .token_id = MESH_TOKEN_ID,
  .token_value = MESH_TOKEN_VALUE,
};

// criar tarefa de envio
void esp_mesh_p2p_tx_main(void *arg) {
  int i;
  esp_err_t err;
  int send_count = 0;
  mesh_addr_t route_table[CONFIG_MESH_ROUTE_TABLE_SIZE];
  int route_table_size = 0;
  mesh_data_t data;
  data.data = tx_buf;
  data.size = sizeof(tx_buf);
  data.proto = MESH_PROTO_BIN;
  data.tos = MESH_TOS_P2P;
  is_running = true;

  while (is_running) {
    /* non-root do nothing but print */
    if (!esp_mesh_is_root()) {
      ESP_LOGI(MESH_TAG, "layer:%d, rtableSize:%d, %s", mesh_layer,
               esp_mesh_get_routing_table_size(),
               is_mesh_connected ? "NODE" : "DISCONNECT");
      vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
      continue;
    }
    esp_mesh_get_routing_table((mesh_addr_t *)&route_table,
                               CONFIG_MESH_ROUTE_TABLE_SIZE * 6,
                               &route_table_size);
    if (send_count && !(send_count % 100)) {
      ESP_LOGI(MESH_TAG, "size:%d/%d,send_count:%d", route_table_size,
               esp_mesh_get_routing_table_size(), send_count);
    }
    send_count++;
    tx_buf[25] = (send_count >> 24) & 0xff;
    tx_buf[24] = (send_count >> 16) & 0xff;
    tx_buf[23] = (send_count >> 8) & 0xff;
    tx_buf[22] = (send_count >> 0) & 0xff;
    if (send_count % 2) {
      memcpy(tx_buf, (uint8_t *)&light_on, sizeof(light_on));
    } else {
      memcpy(tx_buf, (uint8_t *)&light_off, sizeof(light_off));
    }

    for (i = 0; i < route_table_size; i++) {
      err = esp_mesh_send(&route_table[i], &data, MESH_DATA_P2P, NULL, 0);
      if (err) {
        ESP_LOGE(MESH_TAG,
                 "[ROOT-2-UNICAST:%d][L:%d]parent:" MACSTR " to " MACSTR
                 ", heap:%" PRId32 "[err:0x%x, proto:%d, tos:%d]",
                 send_count, mesh_layer, MAC2STR(mesh_parent_addr.addr),
                 MAC2STR(route_table[i].addr), esp_get_minimum_free_heap_size(),
                 err, data.proto, data.tos);
      } else if (!(send_count % 100)) {
        ESP_LOGW(MESH_TAG,
                 "[ROOT-2-UNICAST:%d][L:%d][rtableSize:%d]parent:" MACSTR
                 " to " MACSTR ", heap:%" PRId32 "[err:0x%x, proto:%d, tos:%d]",
                 send_count, mesh_layer, esp_mesh_get_routing_table_size(),
                 MAC2STR(mesh_parent_addr.addr), MAC2STR(route_table[i].addr),
                 esp_get_minimum_free_heap_size(), err, data.proto, data.tos);
      }
    }
    /* if route_table_size is less than 10, add delay to avoid watchdog in this
     * task. */
    if (route_table_size < 10) {
      vTaskDelay(1 * 1000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

// criar tarefa de recepção
void esp_mesh_p2p_rx_main(void *arg) {
  int recv_count = 0;
  esp_err_t err;
  mesh_addr_t from;
  int send_count = 0;
  mesh_data_t data;
  int flag = 0;
  data.data = rx_buf;
  data.size = RX_SIZE;
  is_running = true;

  while (is_running) {
    data.size = RX_SIZE;
    err = esp_mesh_recv(&from, &data, portMAX_DELAY, &flag, NULL, 0);
    if (err != ESP_OK || !data.size) {
      ESP_LOGE(MESH_TAG, "err:0x%x, size:%d", err, data.size);
      continue;
    }
    /* extract send count */
    if (data.size >= sizeof(send_count)) {
      send_count = (data.data[25] << 24) | (data.data[24] << 16) |
                   (data.data[23] << 8) | data.data[22];
    }
    recv_count++;
    /* process light control */
    mesh_light_process(&from, data.data, data.size);
    if (!(recv_count % 1)) {
      ESP_LOGW(
          MESH_TAG,
          "[#RX:%d/%d][L:%d] parent:" MACSTR ", receive from " MACSTR
          ", size:%d, heap:%" PRId32 ", flag:%d[err:0x%x, proto:%d, tos:%d]",
          recv_count, send_count, mesh_layer, MAC2STR(mesh_parent_addr.addr),
          MAC2STR(from.addr), data.size, esp_get_minimum_free_heap_size(), flag,
          err, data.proto, data.tos);
    }
  }
  vTaskDelete(NULL);
}

// inicia comunicação P2P
esp_err_t esp_mesh_comm_p2p_start(void) {
  static bool is_comm_p2p_started = false;
  if (!is_comm_p2p_started) {
    is_comm_p2p_started = true;
    xTaskCreate(esp_mesh_p2p_tx_main, "MPTX", 3072, NULL, 5, NULL);
    xTaskCreate(esp_mesh_p2p_rx_main, "MPRX", 3072, NULL, 5, NULL);
  }
  return ESP_OK;
}

// manipulação de eventos mesh
void mesh_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data) {
  mesh_addr_t id = {
      0,
  };
  static uint16_t last_layer = 0;

  switch (event_id) {
    case MESH_EVENT_STARTED: {
      esp_mesh_get_id(&id);
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_MESH_STARTED>ID:" MACSTR "",
               MAC2STR(id.addr));
      is_mesh_connected = false;
      mesh_layer = esp_mesh_get_layer();
    } break;
    case MESH_EVENT_STOPPED: {
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOPPED>");
      is_mesh_connected = false;
      mesh_layer = esp_mesh_get_layer();
    } break;
    case MESH_EVENT_CHILD_CONNECTED: {
      mesh_event_child_connected_t *child_connected =
          (mesh_event_child_connected_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_CONNECTED>aid:%d, " MACSTR "",
               child_connected->aid, MAC2STR(child_connected->mac));
    } break;
    case MESH_EVENT_CHILD_DISCONNECTED: {
      mesh_event_child_disconnected_t *child_disconnected =
          (mesh_event_child_disconnected_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHILD_DISCONNECTED>aid:%d, " MACSTR "",
               child_disconnected->aid, MAC2STR(child_disconnected->mac));
    } break;
    case MESH_EVENT_ROUTING_TABLE_ADD: {
      mesh_event_routing_table_change_t *routing_table =
          (mesh_event_routing_table_change_t *)event_data;
      ESP_LOGW(MESH_TAG,
               "<MESH_EVENT_ROUTING_TABLE_ADD>add %d, new:%d, layer:%d",
               routing_table->rt_size_change, routing_table->rt_size_new,
               mesh_layer);
    } break;
    case MESH_EVENT_ROUTING_TABLE_REMOVE: {
      mesh_event_routing_table_change_t *routing_table =
          (mesh_event_routing_table_change_t *)event_data;
      ESP_LOGW(MESH_TAG,
               "<MESH_EVENT_ROUTING_TABLE_REMOVE>remove %d, new:%d, layer:%d",
               routing_table->rt_size_change, routing_table->rt_size_new,
               mesh_layer);
    } break;
    case MESH_EVENT_NO_PARENT_FOUND: {
      mesh_event_no_parent_found_t *no_parent =
          (mesh_event_no_parent_found_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_NO_PARENT_FOUND>scan times:%d",
               no_parent->scan_times);
    }
    /* TODO handler for the failure */
    break;
    case MESH_EVENT_PARENT_CONNECTED: {
      mesh_event_connected_t *connected = (mesh_event_connected_t *)event_data;
      esp_mesh_get_id(&id);
      mesh_layer = connected->self_layer;
      memcpy(&mesh_parent_addr.addr, connected->connected.bssid, 6);
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_PARENT_CONNECTED>layer:%d-->%d, parent:" MACSTR
               "%s, ID:" MACSTR ", duty:%d",
               last_layer, mesh_layer, MAC2STR(mesh_parent_addr.addr),
               esp_mesh_is_root()  ? "<ROOT>"
               : (mesh_layer == 2) ? "<layer2>"
                                   : "",
               MAC2STR(id.addr), connected->duty);
      last_layer = mesh_layer;
      mesh_connected_indicator(mesh_layer);
      is_mesh_connected = true;
      if (esp_mesh_is_root()) {
        esp_netif_dhcpc_stop(netif_sta);
        esp_netif_dhcpc_start(netif_sta);
      }
      esp_mesh_comm_p2p_start();
    } break;
    case MESH_EVENT_PARENT_DISCONNECTED: {
      mesh_event_disconnected_t *disconnected =
          (mesh_event_disconnected_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_PARENT_DISCONNECTED>reason:%d",
               disconnected->reason);
      is_mesh_connected = false;
      mesh_disconnected_indicator();
      mesh_layer = esp_mesh_get_layer();
    } break;
    case MESH_EVENT_LAYER_CHANGE: {
      mesh_event_layer_change_t *layer_change =
          (mesh_event_layer_change_t *)event_data;
      mesh_layer = layer_change->new_layer;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_LAYER_CHANGE>layer:%d-->%d%s", last_layer,
               mesh_layer,
               esp_mesh_is_root()  ? "<ROOT>"
               : (mesh_layer == 2) ? "<layer2>"
                                   : "");
      last_layer = mesh_layer;
      mesh_connected_indicator(mesh_layer);
    } break;
    case MESH_EVENT_ROOT_ADDRESS: {
      mesh_event_root_address_t *root_addr =
          (mesh_event_root_address_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_ADDRESS>root address:" MACSTR "",
               MAC2STR(root_addr->addr));
    } break;
    case MESH_EVENT_VOTE_STARTED: {
      mesh_event_vote_started_t *vote_started =
          (mesh_event_vote_started_t *)event_data;
      ESP_LOGI(
          MESH_TAG,
          "<MESH_EVENT_VOTE_STARTED>attempts:%d, reason:%d, rc_addr:" MACSTR "",
          vote_started->attempts, vote_started->reason,
          MAC2STR(vote_started->rc_addr.addr));
    } break;
    case MESH_EVENT_VOTE_STOPPED: {
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_VOTE_STOPPED>");
      break;
    }
    case MESH_EVENT_ROOT_SWITCH_REQ: {
      mesh_event_root_switch_req_t *switch_req =
          (mesh_event_root_switch_req_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_ROOT_SWITCH_REQ>reason:%d, rc_addr:" MACSTR "",
               switch_req->reason, MAC2STR(switch_req->rc_addr.addr));
    } break;
    case MESH_EVENT_ROOT_SWITCH_ACK: {
      /* new root */
      mesh_layer = esp_mesh_get_layer();
      esp_mesh_get_parent_bssid(&mesh_parent_addr);
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_ROOT_SWITCH_ACK>layer:%d, parent:" MACSTR "",
               mesh_layer, MAC2STR(mesh_parent_addr.addr));
    } break;
    case MESH_EVENT_TODS_STATE: {
      mesh_event_toDS_state_t *toDs_state =
          (mesh_event_toDS_state_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_TODS_REACHABLE>state:%d", *toDs_state);
    } break;
    case MESH_EVENT_ROOT_FIXED: {
      mesh_event_root_fixed_t *root_fixed =
          (mesh_event_root_fixed_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_ROOT_FIXED>%s",
               root_fixed->is_fixed ? "fixed" : "not fixed");
    } break;
    case MESH_EVENT_ROOT_ASKED_YIELD: {
      mesh_event_root_conflict_t *root_conflict =
          (mesh_event_root_conflict_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_ROOT_ASKED_YIELD>" MACSTR ", rssi:%d, capacity:%d",
               MAC2STR(root_conflict->addr), root_conflict->rssi,
               root_conflict->capacity);
    } break;
    case MESH_EVENT_CHANNEL_SWITCH: {
      mesh_event_channel_switch_t *channel_switch =
          (mesh_event_channel_switch_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_CHANNEL_SWITCH>new channel:%d",
               channel_switch->channel);
    } break;
    case MESH_EVENT_SCAN_DONE: {
      mesh_event_scan_done_t *scan_done = (mesh_event_scan_done_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_SCAN_DONE>number:%d", scan_done->number);
    } break;
    case MESH_EVENT_NETWORK_STATE: {
      mesh_event_network_state_t *network_state =
          (mesh_event_network_state_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_NETWORK_STATE>is_rootless:%d",
               network_state->is_rootless);
    } break;
    case MESH_EVENT_STOP_RECONNECTION: {
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_STOP_RECONNECTION>");
    } break;
    case MESH_EVENT_FIND_NETWORK: {
      mesh_event_find_network_t *find_network =
          (mesh_event_find_network_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_FIND_NETWORK>new channel:%d, router BSSID:" MACSTR
               "",
               find_network->channel, MAC2STR(find_network->router_bssid));
    } break;
    case MESH_EVENT_ROUTER_SWITCH: {
      mesh_event_router_switch_t *router_switch =
          (mesh_event_router_switch_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_ROUTER_SWITCH>new router:%s, channel:%d, " MACSTR
               "",
               router_switch->ssid, router_switch->channel,
               MAC2STR(router_switch->bssid));
    } break;
    case MESH_EVENT_PS_PARENT_DUTY: {
      mesh_event_ps_duty_t *ps_duty = (mesh_event_ps_duty_t *)event_data;
      ESP_LOGI(MESH_TAG, "<MESH_EVENT_PS_PARENT_DUTY>duty:%d", ps_duty->duty);
    } break;
    case MESH_EVENT_PS_CHILD_DUTY: {
      mesh_event_ps_duty_t *ps_duty = (mesh_event_ps_duty_t *)event_data;
      ESP_LOGI(MESH_TAG,
               "<MESH_EVENT_PS_CHILD_DUTY>cidx:%d, " MACSTR ", duty:%d",
               ps_duty->child_connected.aid - 1,
               MAC2STR(ps_duty->child_connected.mac), ps_duty->duty);
    } break;
    default:
      ESP_LOGI(MESH_TAG, "unknown id:%" PRId32 "", event_id);
      break;
  }
}

// manipulação de IPs
void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                      void *event_data) {
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
  ESP_LOGI(MESH_TAG, "<IP_EVENT_STA_GOT_IP>IP:" IPSTR,
           IP2STR(&event->ip_info.ip));
  
  int len = snprintf(NULL, 0, IPSTR, IP2STR(&event->ip_info.ip)) + 1;
  ap_ip = malloc(len);
  snprintf(ap_ip, len, IPSTR, IP2STR(&event->ip_info.ip));

  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  ap_mac = malloc(18);
  snprintf(ap_mac, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  printf("\n==============================================\n== IP address:  %s\n== MAC address: %s\n==============================================\n\n", ap_ip, ap_mac);
}

// inicia mesh
static esp_err_t start_mesh(void) {
  esp_err_t ret = ESP_FAIL;
  if ((ret = mesh_light_init()) != ESP_OK) return ret;
  if ((ret = esp_netif_create_default_wifi_mesh_netifs(&netif_sta, NULL)) != ESP_OK) return ret;
  wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();

  if ((ret = esp_wifi_init(&config)) != ESP_OK) return ret;
  if ((ret = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL)) != ESP_OK) return ret;
  if ((ret = esp_wifi_set_storage(WIFI_STORAGE_FLASH)) != ESP_OK) return ret;
  if ((ret = esp_wifi_start()) != ESP_OK) return ret;
  if ((ret = esp_mesh_init()) != ESP_OK) return ret;
  if ((ret = esp_event_handler_register(MESH_EVENT, ESP_EVENT_ANY_ID, &mesh_event_handler, NULL)) != ESP_OK) return ret;
  if ((ret = esp_mesh_set_topology(CONFIG_MESH_TOPOLOGY)) != ESP_OK) return ret;
  if ((ret = esp_mesh_set_max_layer(CONFIG_MESH_MAX_LAYER)) != ESP_OK) return ret;
  if ((ret = esp_mesh_set_vote_percentage(1)) != ESP_OK) return ret;
  if ((ret = esp_mesh_set_xon_qsize(128)) != ESP_OK) return ret;
  if ((ret = esp_mesh_enable_ps()) != ESP_OK) return ret;
  if ((ret = esp_mesh_set_ap_assoc_expire(60)) != ESP_OK) return ret;
  if ((ret = esp_mesh_set_announce_interval(600, 3300)) != ESP_OK) return ret;

  mesh_cfg_t cfg = MESH_INIT_CONFIG_DEFAULT();
  uint8_t mesh_id[6] = {0x77, 0x77, 0x77, 0x77, 0x77, 0x77};
  memcpy((uint8_t *)&cfg.mesh_id, mesh_id, 6);
  cfg.channel = CONFIG_MESH_CHANNEL;
  cfg.router.ssid_len = strlen(ssid_name);
  memcpy((uint8_t *)&cfg.router.ssid, ssid_name, cfg.router.ssid_len);
  memcpy((uint8_t *)&cfg.router.password, ssid_password, strlen(ssid_password));
  ret = esp_mesh_set_ap_authmode(CONFIG_MESH_AP_AUTHMODE);
  if (ret != ESP_OK) return ret;
  cfg.mesh_ap.max_connection = CONFIG_MESH_AP_CONNECTIONS;
  cfg.mesh_ap.nonmesh_max_connection = CONFIG_MESH_NON_MESH_AP_CONNECTIONS;
  memcpy((uint8_t *)&cfg.mesh_ap.password, ap_password, strlen(ap_password));
  if ((ret = esp_mesh_set_config(&cfg)) != ESP_OK) return ret;

  /* mesh start */
  if ((ret = esp_mesh_start()) != ESP_OK) return ret;
  if ((ret = esp_mesh_set_active_duty_cycle(CONFIG_MESH_PS_DEV_DUTY, CONFIG_MESH_PS_DEV_DUTY_TYPE)) != ESP_OK) return ret;
  if ((ret = esp_mesh_set_network_duty_cycle(CONFIG_MESH_PS_NWK_DUTY, CONFIG_MESH_PS_NWK_DUTY_DURATION, CONFIG_MESH_PS_NWK_DUTY_RULE)) != ESP_OK) return ret;
  ESP_LOGI(
      MESH_TAG, "mesh starts successfully, heap:%" PRId32 ", %s<%d>%s, ps:%d",
      esp_get_minimum_free_heap_size(),
      esp_mesh_is_root_fixed() ? "root fixed" : "root not fixed",
      esp_mesh_get_topology(), esp_mesh_get_topology() ? "(chain)" : "(tree)",
      esp_mesh_is_ps_enabled());
  return ESP_OK;
}



/*******************************************************
 * Tratamento Principal
 *******************************************************/

void app_main(void) {
  // Inicializa partição de arquivos e Verifica arquivo de configuração
  ESP_ERROR_CHECK(check_spiffs());
  ESP_ERROR_CHECK(read_config_file());
  
  if (ssid_name != NULL)     ESP_LOGI(TAG, "ssid_name....: '%s'", ssid_name);
  if (ssid_password != NULL) ESP_LOGI(TAG, "ssid_password: '%s'", ssid_password);
  if (ap_password != NULL)   ESP_LOGI(TAG, "ap_password..: '%s'", ap_password);
  if (ap_id != NULL)         ESP_LOGI(TAG, "ap_id........: '%s'", ap_id);


  printf("Example configured to blink addressable LED!");
  led_strip_config_t strip_config = {
      .strip_gpio_num = BLINK_GPIO,
      .max_leds = 1,
  };
  led_strip_spi_config_t spi_config = {
      .spi_bus = SPI2_HOST,
      .flags.with_dma = true,
  };
  ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
  led_strip_clear(led_strip);



  // Inicializa sensor de temperatura
  ESP_LOGI(TAG, "Install temperature sensor, expected temp ranger range: 10~50 ℃");
  ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_sensor));
  ESP_LOGI(TAG, "Enable temperature sensor");
  ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));

  // Inicializa rede wifi
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // Inicializa Rede Mesh
  ret = ESP_FAIL;
  if (ssid_name != NULL && ssid_password != NULL && ap_password != NULL) {
    ESP_LOGI(TAG, "Iniciando Rede Mesh");
    ret = start_mesh();
  }

  // Inicializa ponto de acesso sem internet se rede mesh falhar
  if (ret != ESP_OK) {
    ESP_LOGI(TAG, "Iniciando Access Point sem Internet");
    wifi_init_softap();
  }

  // Inicializa servidor web
  static httpd_handle_t server = NULL;
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
  server = start_webserver();



  // Tratamento de reset de configuração
  esp_reset_reason_t reset_reason = esp_reset_reason();
  if (reset_reason == ESP_RST_POWERON) {
    ESP_LOGI(TAG, "Reset reason: Power on reset");
    read_reset_file();
    reset_flag--;
    ESP_LOGI(TAG, "Reset Flag: %d", reset_flag);
    if (reset_flag <= 0) {
      if (stat(CONFIG_FILE_PATH, &st) == 0) {
        unlink(CONFIG_FILE_PATH);
        esp_restart();
      }
    }
    save_reset_file();
    for (int i = 30; i >= 0; i--) {
      printf("Restarting in %d seconds...\n", i);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
  reset_flag = 4;
  save_reset_file();
}
