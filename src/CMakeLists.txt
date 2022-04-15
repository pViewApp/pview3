set(Qt_VERSION 6 CACHE STRING "Specifies the version of the Qt Tookit to use.")

find_package(Qt${Qt_VERSION} COMPONENTS Core Gui Widgets REQUIRED)
find_package(Boost REQUIRED)
find_package(Qwt REQUIRED)
find_package(date CONFIG REQUIRED)

add_executable(pview
	"Actions.h"
	"Actions.cpp"
	"DataFile.cpp"
	"DataFile.h"
	"Main.cpp"
	"Types.h"
	
	"ui/AccountPage.cpp"
	"ui/AccountPage.h"
	"ui/DataFileManager.h"
	"ui/MainWindow.cpp"
	"ui/MainWindow.h"
	"ui/SecurityModel.h"
	"ui/SecurityModel.cpp"
	"ui/SecurityPage.h"
	"ui/SecurityPage.cpp"
	"ui/Page.cpp"
	"ui/Page.h"
 "ui/TransactionModel.h" "ui/TransactionModel.cpp")

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

