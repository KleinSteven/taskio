file(GLOB_RECURSE examples *.cpp)

foreach (example ${examples})
    # 获取当前文件的文件名（不带扩展名），并将结果保存在变量 target_name 中
    get_filename_component(target_name ${example} NAME_WE)

    add_executable(${target_name} ${example})
    #target_link_libraries(${target_name} PRIVATE taskio)
endforeach ()