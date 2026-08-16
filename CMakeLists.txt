if(CONFIG_SOFLE_BONGO_DISPLAY)
  zephyr_library()
  zephyr_library_sources(src/sofle_bongo_status.c)
endif()
