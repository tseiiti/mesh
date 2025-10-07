set(requires esp-tls esp_netif esp_driver_tsens esp_http_server esp_http_client spiffs esp_driver_gpio esp_driver_ledc nvs_flash spi_flash)
idf_build_get_property(target IDF_TARGET)

if(${target} STREQUAL "linux")
    list(APPEND requires esp_stubs protocol_examples_common)
else()
    list(APPEND requires esp_wifi esp_eth)
endif()

idf_component_register(SRCS "mesh_light.c" 
                            "aux.c"
                            "main.c"
                       PRIV_REQUIRES ${requires}
                       INCLUDE_DIRS "." "include")
