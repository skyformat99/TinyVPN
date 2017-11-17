INCLUDE_DIRECTORIES(${TinyVPN_SOURCE_DIR}/include)
LINK_DIRECTORIES(${TinyVPN_SOURCE_DIR}/third_lib)

SET(CLIENT_SRC client.cpp common.cpp client_main.cpp)
SET(SERVER_SRC server.cpp common.cpp server_main.cpp)

ADD_EXECUTABLE(client ${CLIENT_SRC})
ADD_EXECUTABLE(server ${SERVER_SRC})

TARGET_LINK_LIBRARIES(client gflags pthread)
TARGET_LINK_LIBRARIES(server gflags tins pthread)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
