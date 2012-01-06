NOTE: xuggler is a native encoding library. If you want to export videos directly from the Large Network Visualizer, go to: 
   http://www.xuggle.com/xuggler/downloads/

...and download Xuggler LGPL 3.4 - "Forrest", under the "Linux" tab. Make sure you get the correct library set; 32-bit or 64-bit!
Then, install it, e.g.:
   chmod 755 xuggle-xuggler.3.4.1012-i686-pc-linux-gnu.sh
   sudo ./xuggle-xuggler.3.4.1012-i686-pc-linux-gnu.sh

...selecting all the default options. Now, go to:
   /usr/local/xuggler/lib

...and copy all the files there into THIS (libs-native) directory. Now you can encode videos directly from the Visualizer.

