

add_executable(FeatureClient FeatureClient.cpp)
target_link_libraries(FeatureClient muduo_net)





add_executable( ServerMain
        main.cpp
        FeatureServer.cpp
        FileHelper.cpp
        ConfigFileReader.cpp
        HiRedis.cpp )

#link_directories("mak")

#link_libraries(ServerMain /usr/local/lib/libhiredis.so.0.14)
#find_library(HIREDIS_LIB hiredis /usr/local/lib )

#link_directories(/home/test2/github/hiredis)

#find_library(HIREDIS_LIBRARY NAMES hiredis)

#if( HIREDIS_LIBRARY)
#    message(STATUS "found hiredis")
#endif()


#message(HIREDIS_LIB)
target_link_libraries(ServerMain muduo_net hiredis)


set_target_properties(ServerMain PROPERTIES COMPILE_FLAGS "-Wno-error=old-style-cast")

