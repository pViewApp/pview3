set(Qt_VERSION 6 CACHE STRING "Specifies the version of the Qt Tookit to use.")

find_package(Qt${Qt_VERSION} COMPONENTS Core Gui Widgets REQUIRED)
find_package(Boost REQUIRED)
find_package(Qwt REQUIRED)
find_package(date CONFIG REQUIRED)

add_executable(pview
	datafile.cpp
	datafile.h
	main.cpp
	types.h
	
	ui/accountpage.cpp
	ui/accountpage.h
	ui/datafilemanager.h
	ui/mainwindow.cpp
	ui/mainwindow.h
	
	ui/navigation.h
	ui/page.cpp
	ui/page.h
	ui/accountdialog.h
	ui/util.h
	ui/util.cpp
	actions.h
	actions.cpp
)

target_include_directories(pview PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(pview PRIVATE ${QWT_INCLUDE_DIR})
target_compile_features(pview PRIVATE cxx_std_17)
target_link_libraries(pview PRIVATE ${Boost_LIBRARIES} ${QWT_LIBRARIES} Qt6::Core Qt6::Gui Qt6::Widgets date::date)

set_target_properties(pview PROPERTIES
	MACOSX_BUNDLE_GUI_IDENTIFIER io.github.pviewapp.pview
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	MACOSX_BUNDLE TRUE
	WIN32_EXECUTABLE TRUE
)

