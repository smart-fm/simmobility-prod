=begin
** Form generated from reading ui file 'mainwindow.ui'
**
** Created: Thu May 3 10:53:36 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
=end

require 'Qt4'

class Ui_Main_window
    attr_reader :menuOpenTraceFile
    attr_reader :menuItemQuit
    attr_reader :centralwidget
    attr_reader :verticalLayout
    attr_reader :agViewCanvas
    attr_reader :fileProgress
    attr_reader :horizontalLayout
    attr_reader :viewCreateDestroy
    attr_reader :viewUpdates
    attr_reader :agTicksCmb
    attr_reader :horizontalSpacer
    attr_reader :menubar
    attr_reader :menuFile
    attr_reader :statusbar

    def setupUi(main_window)
    if main_window.objectName.nil?
        main_window.objectName = "main_window"
    end
    main_window.resize(741, 393)
    @menuOpenTraceFile = Qt::Action.new(main_window)
    @menuOpenTraceFile.objectName = "menuOpenTraceFile"
    @menuItemQuit = Qt::Action.new(main_window)
    @menuItemQuit.objectName = "menuItemQuit"
    @centralwidget = Qt::Widget.new(main_window)
    @centralwidget.objectName = "centralwidget"
    @verticalLayout = Qt::VBoxLayout.new(@centralwidget)
    @verticalLayout.objectName = "verticalLayout"
    @agViewCanvas = Qt::GraphicsView.new(@centralwidget)
    @agViewCanvas.objectName = "agViewCanvas"

    @verticalLayout.addWidget(@agViewCanvas)

    @fileProgress = Qt::ProgressBar.new(@centralwidget)
    @fileProgress.objectName = "fileProgress"
    @fileProgress.value = 24

    @verticalLayout.addWidget(@fileProgress)

    @horizontalLayout = Qt::HBoxLayout.new()
    @horizontalLayout.objectName = "horizontalLayout"
    @viewCreateDestroy = Qt::PushButton.new(@centralwidget)
    @viewCreateDestroy.objectName = "viewCreateDestroy"
    @viewCreateDestroy.checkable = true

    @horizontalLayout.addWidget(@viewCreateDestroy)

    @viewUpdates = Qt::PushButton.new(@centralwidget)
    @viewUpdates.objectName = "viewUpdates"
    @viewUpdates.checkable = true

    @horizontalLayout.addWidget(@viewUpdates)

    @agTicksCmb = Qt::ComboBox.new(@centralwidget)
    @agTicksCmb.objectName = "agTicksCmb"
    @agTicksCmb.enabled = false

    @horizontalLayout.addWidget(@agTicksCmb)

    @horizontalSpacer = Qt::SpacerItem.new(40, 20, Qt::SizePolicy::Expanding, Qt::SizePolicy::Minimum)

    @horizontalLayout.addItem(@horizontalSpacer)


    @verticalLayout.addLayout(@horizontalLayout)

    main_window.centralWidget = @centralwidget
    @menubar = Qt::MenuBar.new(main_window)
    @menubar.objectName = "menubar"
    @menubar.geometry = Qt::Rect.new(0, 0, 741, 26)
    @menuFile = Qt::Menu.new(@menubar)
    @menuFile.objectName = "menuFile"
    main_window.setMenuBar(@menubar)
    @statusbar = Qt::StatusBar.new(main_window)
    @statusbar.objectName = "statusbar"
    main_window.statusBar = @statusbar

    @menubar.addAction(@menuFile.menuAction())
    @menuFile.addAction(@menuOpenTraceFile)
    @menuFile.addAction(@menuItemQuit)

    retranslateUi(main_window)
    Qt::Object.connect(@menuItemQuit, SIGNAL('activated()'), main_window, SLOT('close()'))

    Qt::MetaObject.connectSlotsByName(main_window)
    end # setupUi

    def setup_ui(main_window)
        setupUi(main_window)
    end

    def retranslateUi(main_window)
    main_window.windowTitle = Qt::Application.translate("main_window", "Agent Concurrency Checker", nil, Qt::Application::UnicodeUTF8)
    @menuOpenTraceFile.text = Qt::Application.translate("main_window", "Open Trace File", nil, Qt::Application::UnicodeUTF8)
    @menuItemQuit.text = Qt::Application.translate("main_window", "Quit", nil, Qt::Application::UnicodeUTF8)
    @viewCreateDestroy.text = Qt::Application.translate("main_window", "Show Create/Destroy", nil, Qt::Application::UnicodeUTF8)
    @viewUpdates.text = Qt::Application.translate("main_window", "Show Agent Updates", nil, Qt::Application::UnicodeUTF8)
    @agTicksCmb.insertItems(0, [Qt::Application.translate("main_window", "(Update Ticks)", nil, Qt::Application::UnicodeUTF8)])
    @menuFile.title = Qt::Application.translate("main_window", "File", nil, Qt::Application::UnicodeUTF8)
    end # retranslateUi

    def retranslate_ui(main_window)
        retranslateUi(main_window)
    end

end

module Ui
    class Main_window < Ui_Main_window
    end
end  # module Ui

if $0 == __FILE__
    a = Qt::Application.new(ARGV)
    u = Ui_Main_window.new
    w = Qt::MainWindow.new
    u.setupUi(w)
    w.show
    a.exec
end
