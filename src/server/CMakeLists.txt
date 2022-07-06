# 添加源文件到SRC_LIST
aux_source_directory(. SRC_LIST) 

# 添加数据库源文件
aux_source_directory(./db DB_LIST) 

# 添加Redis中间件源文件
aux_source_directory(./redis REDIS_LIST) 

# 添加model源文件
aux_source_directory(./model MODEL_LIST) 

add_executable(chatserver ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

target_link_libraries(chatserver muduo_net muduo_base mysqlclient hiredis pthread)
