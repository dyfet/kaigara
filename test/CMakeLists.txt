# SPDX-License-Identifier: BSD-3-Clause
# Copyright (C) 2026 David Sugar <tychosoft@gmail.com>

# libcpr tests

add_executable(test_bitset bitset.c)
add_test(NAME test-bitset COMMAND test_bitset)
target_link_libraries(test_bitset PRIVATE import_cpr)

add_executable(test_memory memory.c)
add_test(NAME test-memory COMMAND test_memory)
target_link_libraries(test_memory PRIVATE import_cpr)

add_executable(test_memio memio.c)
add_test(NAME test-memio COMMAND test_memio)
target_link_libraries(test_memio PRIVATE import_cpr)

add_executable(test_strchar strchar.c)
add_test(NAME test-strchar COMMAND test_strchar)
target_link_libraries(test_strchar PRIVATE import_cpr)

add_executable(test_endian endian.c)
add_test(NAME test-endian COMMAND test_endian)

add_executable(test_thread thread.c)
add_test(NAME test-thread COMMAND test_thread)
target_link_libraries(test_thread PRIVATE import_cpr)

add_executable(test_keyfile keyfile.c)
set_target_properties(test_keyfile PROPERTIES COMPILE_DEFINITIONS "TEST_DATA=\"${CMAKE_SOURCE_DIR}/test\"")
target_link_libraries(test_keyfile PRIVATE import_cpr)
add_test(NAME test-keyfile COMMAND test_keyfile)

# minicrypt tests

add_executable(test_sha1 sha1.c)
add_test(NAME test-sha1 COMMAND test_sha1)
target_link_libraries(test_sha1 PRIVATE import_minicrypt)

add_executable(test_sha256 sha256.c)
add_test(NAME test-sha256 COMMAND test_sha256)
target_link_libraries(test_sha256 PRIVATE import_minicrypt)

add_executable(test_hmac hmac.c)
add_test(NAME test-hmac COMMAND test_hmac)
target_link_libraries(test_hmac PRIVATE import_minicrypt)

add_executable(test_md5 md5.c)
add_test(NAME test-md5 COMMAND test_md5)
target_link_libraries(test_md5 PRIVATE import_minicrypt)

add_executable(test_random random.c)
add_test(NAME test-random COMMAND test_random)
target_link_libraries(test_random PRIVATE import_minicrypt)

add_executable(test_ring256 ring256.c)
add_test(NAME test-ring256 COMMAND test_ring256)
target_link_libraries(test_ring256 PRIVATE import_minicrypt)

add_executable(test_aes aes.c)
add_test(NAME test-aes COMMAND test_aes)
target_link_libraries(test_aes PRIVATE import_minicrypt)

