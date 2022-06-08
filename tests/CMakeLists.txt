project(tests LANGUAGES CXX)

add_executable(tst_grouper tst_grouper.cpp
                           ../grouper.h ../grouper.cpp
                           ../group.h ../bookmark.h)
add_test(NAME tst_grouper COMMAND tst_grouper)

target_link_libraries(tst_grouper PRIVATE Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Concurrent)
