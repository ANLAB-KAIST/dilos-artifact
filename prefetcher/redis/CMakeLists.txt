project(redis C)


add_library(prefetch_redis SHARED redis.c)
target_link_libraries(prefetch_redis PUBLIC osv)
target_link_libraries(prefetch_redis PUBLIC dl)
target_include_directories(prefetch_redis PRIVATE ../../apps/redis/redis/src)
target_include_directories(prefetch_redis PRIVATE ../../apps/redis/redis/deps/lua/src)

target_compile_options(prefetch_redis PRIVATE -fno-plt)
