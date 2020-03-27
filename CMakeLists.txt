project(ipc_tests)
cmake_minimum_required(VERSION 3.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Xenomai QUIET)

option(ENABLE_XENO FALSE)

add_executable(unnamed_pipe src/ipc/unnamed_pipe.c)

add_executable(fifo_reader src/ipc/fifo_reader.c)

add_executable(fifo_writer src/ipc/fifo_writer.c)

add_executable(posix-shm-server src/ipc/posix-shm-server.c)
target_link_libraries(posix-shm-server rt)

add_executable(posix-shm-client src/ipc/posix-shm-client.c)
target_link_libraries(posix-shm-client rt)

add_executable(posix-shm-sem-writer src/ipc/posix-shm-sem-writer.c)
target_link_libraries(posix-shm-sem-writer pthread rt)

add_executable(posix-shm-sem-reader src/ipc/posix-shm-sem-reader.c)
target_link_libraries(posix-shm-sem-reader pthread rt)


add_executable(unix_soc_client src/ipc/unix-domain-socket-client.c)

add_executable(unix_soc_server src/ipc/unix-domain-socket-server.c)

add_executable(soc_client src/ipc/socket-client.c)

add_executable(soc_server src/ipc/socket-server.c)


if(${ENABLE_XENO})
    add_executable(xddp_label       src/RTipc/xddp_label.c)
    add_executable(iddp_label       src/RTipc/iddp_label.c)
    add_executable(xbotcore_flow    src/RTipc/xbotcore_flow.c)
    set_xeno_flags(xddp_label)
    set_xeno_flags(iddp_label)
    set_xeno_flags(xbotcore_flow)
endif()
