You'll need ruby installed. I'm not sure if Ubuntu's packaged version (1.8) works; you can install 1.9 easily enough with rvm:
    sudo apt-get install rvm
    rvm install 1.9.3
    rvm use 1.9.3
    rvmsudo gem install qtbindings

Then, simply:
    ruby main.rb

If you change the ui file in QtDesigner, you'll have to regenerate it:
    rbuic4 mainwindow.ui -x -o mainwindow_ui.rb

...if you don't have clear path to rbuic4, you might have to find it manually. E.g., mine was here:
    /usr/share/ruby-rvm/gems/ruby-1.9.3/gems/qtbindings-4.6.3.4/bin/1.9/rbuic4


