find_package(Boost REQUIRED)
find_package(date CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qwt REQUIRED)

add_executable(pview
	pv/Account.h
	pv/Account.cpp
	pv/Action.h
	pv/Actions.h
	pv/Algorithms.h
	pv/Algorithms.cpp
	pv/Invalidatable.h
	pv/Transaction.h
	pv/Transaction.cpp
	pv/Date.h
	pv/Decimal.h
	pv/Security.h
	pv/Security.cpp
	pv/DataFile.h
	pv/DataFile.cpp

	pvui/AccountPage.cpp
	pvui/AccountPage.h
	pvui/ActionMappings.h
	pvui/DataFileManager.h
	pvui/ExtendedSpinBox.h
	pvui/ExtendedSpinbox.cpp
	pvui/FormatUtils.h
	pvui/FormatUtils.cpp
	pvui/ModelUtils.h
	pvui/ModelUtils.cpp
	pvui/HoldingsReport.cpp
	pvui/HoldingsReport.h
	pvui/HoldingsModel.cpp
	pvui/HoldingsModel.h
	pvui/MainWindow.cpp
	pvui/MainWindow.h
	pvui/NavigationModel.h
	pvui/NavigationModel.cpp
	pvui/Page.cpp
	pvui/Page.h
	pvui/pView.cpp
	pvui/Report.h
	pvui/ReportFactory.h
	pvui/SecurityInsertionWidget.cpp
	pvui/SecurityInsertionWidget.h
	pvui/SecurityModel.cpp
	pvui/SecurityModel.h
	pvui/SecurityPage.cpp
	pvui/SecurityPage.h
	pvui/SecurityPriceDialog.cpp
	pvui/SecurityPriceDialog.h
	pvui/SecurityPriceInsertionWidget.cpp
	pvui/SecurityPriceInsertionWidget.h
	pvui/SecurityPriceModel.cpp
	pvui/SecurityPriceModel.h
	pvui/SecurityUtils.cpp
	pvui/SecurityUtils.h
	pvui/StandardReportFactory.h
	pvui/StandardReportFactory.cpp
	pvui/TransactionInsertionWidget.cpp
	pvui/TransactionInsertionWidget.h
	pvui/TransactionModel.cpp
	pvui/TransactionModel.h
)

target_include_directories(pview PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(pview PRIVATE ${QWT_INCLUDE_DIR})
target_compile_features(pview PRIVATE cxx_std_17)
target_link_libraries(pview PRIVATE ${Boost_LIBRARIES} ${QWT_LIBRARIES} Qt6::Core Qt6::Gui Qt6::Widgets date::date)

if(MSVC)
	target_compile_options(pview PRIVATE /W4 /WX)
else()
	target_compile_options(pview PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

set_target_properties(pview PROPERTIES
		 MACOSX_BUNDLE_GUI_IDENTIFIER io.github.pviewapp.pview
	 MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	 MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	 MACOSX_BUNDLE TRUE
	 WIN32_EXECUTABLE TRUE
)
