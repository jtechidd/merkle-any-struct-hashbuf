find_package(OpenSSL)

add_library(libmerkle SHARED merkle.c buffer.c)

target_link_libraries(libmerkle PUBLIC OpenSSL::Crypto)

add_executable(merkle_main main.c)

target_link_libraries(merkle_main PRIVATE libmerkle)