set(fakku-downloader_SRCS
    main.cpp
    downloader.cpp
)

# set(agatha-server_HDRS
#     core.h
#     config.h
#     commandexecuter.h
#     requesthandler.h
#     webservice.h
# )

# qt5_wrap_cpp(agatha-server_MOC_HDRS ${agatha-server_HDRS})

# final lib
add_executable(fakku-downloader ${fakku-downloader_SRCS})

qt5_use_modules(fakku-downloader Core Network)

# link lib with Qt libs
target_link_libraries(fakku-downloader)
