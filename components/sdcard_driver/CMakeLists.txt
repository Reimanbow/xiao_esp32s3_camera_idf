idf_component_register(SRCS "sdcard_driver.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES fatfs sdmmc)
