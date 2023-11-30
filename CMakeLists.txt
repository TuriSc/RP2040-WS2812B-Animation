set(TARGET_NAME "ws2812b_animation")

if (NOT TARGET ${TARGET_NAME})
    add_library(${TARGET_NAME} INTERFACE)

    target_sources(${TARGET_NAME} INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}/ws2812b_animation.c
            ${CMAKE_CURRENT_LIST_DIR}/inc/utf8-iterator/source/utf-8.c
    )

    target_include_directories(${TARGET_NAME} INTERFACE
            ${CMAKE_CURRENT_LIST_DIR}
            ${CMAKE_CURRENT_LIST_DIR}/inc/
            ${CMAKE_CURRENT_LIST_DIR}/inc/CP0-EU/
            ${CMAKE_CURRENT_LIST_DIR}/inc/utf8-iterator/source/
    )

    pico_generate_pio_header(${TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

    target_link_libraries(${TARGET_NAME} INTERFACE
        pico_stdlib
        hardware_pio
    )
endif()
