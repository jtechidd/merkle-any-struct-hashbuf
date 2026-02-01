cmake_minimum_required(VERSION 3.28)
project(MerkleAnyStructHashBuf C)

find_package(OpenSSL)

add_library(libmerkle SHARED darray.c buffer.c hash_data.c merkle.c)
target_link_libraries(libmerkle PUBLIC OpenSSL::Crypto)

add_executable(merkle_main main.c)
target_link_libraries(merkle_main PRIVATE libmerkle)