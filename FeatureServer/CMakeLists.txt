

add_executable(FeatureClient FeatureClient.cpp)
target_link_libraries(FeatureClient muduo_net)





add_executable( ServerMain
        main.cpp
        FeatureServer.cpp
        FileHelper.cpp
        ConfigFileReader.cpp
        HiRedis.cpp )
target_link_libraries(ServerMain muduo_net hiredis)
set_target_properties(ServerMain PROPERTIES COMPILE_FLAGS "-Wno-error=old-style-cast")

