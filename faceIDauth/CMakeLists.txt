add_executable(faceIDserver faceIDserver.cpp)
target_link_libraries(faceIDserver muduo_net)

add_executable(faceIDclient faceIDclient.cpp)
target_link_libraries(faceIDclient muduo_net)





