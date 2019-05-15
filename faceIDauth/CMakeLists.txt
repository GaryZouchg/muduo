add_executable(faceIDserver faceIDserver.cpp)
target_link_libraries(faceIDserver muduo_net)

add_executable(faceIDclient faceIDclient.cpp)
target_link_libraries(faceIDclient muduo_net)





add_executable( ServerMain
        main.cpp
        FeatureServer.cpp
        FileHelper.cpp
        ConfigFileReader.cpp
        )
target_link_libraries(ServerMain muduo_net )
set_target_properties(ServerMain PROPERTIES COMPILE_FLAGS "-Wno-error=old-style-cast")

