TEMPLATE = subdirs

SUBDIRS += DiXCoverApp \
           DiXCoverTest

DiXCoverApp.file = src/DiXCoverApp.pro
DiXCoverTest.file = test/DiXCoverTest.pro

DiXCoverTest.depends = DiXCoverApp
