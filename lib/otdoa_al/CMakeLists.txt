zephyr_library_get_current_dir_lib_name(${ZEPHYR_BASE} OTDOA_AL_LIB_TARGET_NAME)
zephyr_library_named(${OTDOA_AL_LIB_TARGET_NAME})

# lfs.h is not in the include path, so add it explicitly here
zephyr_library_include_directories(${ZEPHYR_BASE}/../modules/fs/littlefs)

zephyr_library_sources(otdoa_thread.c)
zephyr_library_sources(otdoa_thread_http.c)
zephyr_library_sources(otdoa_thread_nordic_at.c)
zephyr_library_sources(otdoa_http.c)
zephyr_library_sources(otdoa_http_api.c)
zephyr_library_sources(otdoa_nordic_at.c)
zephyr_library_sources(otdoa_fs.c)
zephyr_library_sources(otdoa_log.c)
zephyr_library_sources(otdoa_timers.c)
zephyr_library_sources(otdoa_shell.c)
zephyr_library_sources_ifdef(CONFIG_OTDOA_LITTLE_FS otdoa_little_fs.c)
