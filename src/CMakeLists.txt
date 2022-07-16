set(QT_VERSION "5" CACHE STRING "The Qt Version (5 or 6)")
find_package(Boost REQUIRED)
find_package(Qt${QT_VERSION} COMPONENTS Core Gui Widgets Network REQUIRED)
find_package(Qwt REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(pview
	pv/Account.h
	pv/Account.cpp
	pv/Algorithms.h
	pv/Algorithms.cpp
	pv/Transaction.h
	pv/Transaction.cpp
	pv/Security.h
	pv/Security.cpp
	pv/DataFile.h
	pv/DataFile.cpp
	pv/Signals.h

	pvui/AccountPage.cpp
	pvui/AccountPage.h
  pvui/AutoFillingDelegate.h
  pvui/AutoFillingDelegate.cpp
	pvui/DataFileManager.h
	pvui/DataFileManager.cpp
  pvui/DateUtils.h
  pvui/DateUtils.cpp
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
	pvui/MarketValueReport.h
	pvui/MarketValueReport.cpp
	pvui/Page.cpp
	pvui/Page.h
	pvui/Report.h
	pvui/Report.cpp
	pvui/ReportFactory.h
	pvui/SecurityInsertionWidget.cpp
	pvui/SecurityInsertionWidget.h
	pvui/SecurityModel.cpp
	pvui/SecurityModel.h
	pvui/SecurityPage.cpp
	pvui/SecurityPage.h
	pvui/SecurityPriceDialog.cpp
	pvui/SecurityPriceDialog.h
	pvui/SecurityPriceDownloader.h
	pvui/SecurityPriceDownloader.cpp
	pvui/SecurityPriceInsertionWidget.cpp
	pvui/SecurityPriceInsertionWidget.h
	pvui/SecurityPriceModel.cpp
	pvui/SecurityPriceModel.h
	pvui/SecurityUtils.cpp
	pvui/SecurityUtils.h
  pvui/SettingsDialog.cpp
  pvui/SettingsDialog.h
	pvui/StandardReportFactory.cpp
	pvui/StandardReportFactory.h
	pvui/ThemeManager.cpp
	pvui/ThemeManager.h
  pvui/Version.cpp
  pvui/Version.h
	pvui/TransactionInsertionWidget.cpp
	pvui/TransactionInsertionWidget.h
	pvui/TransactionModel.cpp
	pvui/TransactionModel.h

	pvui/main.cpp

  resources/icons.qrc
)

target_include_directories(pview SYSTEM PRIVATE ${SQLite3_INCLUDE_DIRS})
target_include_directories(pview SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
target_include_directories(pview SYSTEM PRIVATE ${QWT_INCLUDE_DIR})
target_compile_features(pview PUBLIC cxx_std_17)
target_compile_definitions(pview PRIVATE PVIEW_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} PVIEW_VERSION_MINOR=${PROJECT_VERSION_MINOR} PVIEW_VERSION_PATCH=${PROJECT_VERSION_PATCH})
target_link_libraries(pview PRIVATE ${SQLite3_LIBRARIES} ${Boost_LIBRARIES} Qt${QT_VERSION}::Core Qt${QT_VERSION}::Gui Qt${QT_VERSION}::Gui Qt${QT_VERSION}::Widgets Qt${QT_VERSION}::Network ${QWT_LIBRARIES})

if(MSVC)
	target_compile_options(pview PRIVATE /W4 /WX)
else()
	target_compile_options(pview PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

set_target_properties(pview PROPERTIES
   MACOSX_BUNDLE_GUI_IDENTIFIER io.github.pviewapp.pview
	 MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
   MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}
	 MACOSX_BUNDLE TRUE
	 WIN32_EXECUTABLE TRUE
)
