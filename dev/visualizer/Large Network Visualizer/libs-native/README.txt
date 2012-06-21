NOTE: xuggler is a native encoding library. If you want to export videos directly from the Large Network Visualizer, please perform the
following steps:

   1) Download the appropriate archive from /home/sethhetu/libs on the FM server:
      A) For 32-bit systems:
      scp 172.18.127.157:/home/sethhetu/libs/xuggler_libs_32.tar.bz2 .

      B) For 64-bit systems:
      scp 172.18.127.157:/home/sethhetu/libs/xuggler_libs_64.tar.bz2 .

   3) Extract the contents of the archive you downloaded into THIS directory (libs-native).

   4) In Eclipse, under "Run->Run Configurations...", click on your application (under "Java Applications") and go to the "Environment" tab.
      A) Click on "Select", and then scroll to "LD_LIBRARY_PATH". Click the check box next to it and choose "Ok".
      B) "LD_LIBRARY_PATH" should now be in the area "Environment variables to set". Double-click it, and at the end of the "value" field, add, e.g.:
         /home/USERNAME/simmobility/dev/visualizer/Large Network Visualizer/libs-native
      C) Click "Ok", then "Apply".
      D) Note: If you run the Large Network Visualizer from the command line, just set LD_LIBRARY_PATH as usual (.bashrc, or when launching the program).

Now you can encode videos directly from the Visualizer.

If you prefer to download/compile xuggler from scratch, the official web site is here:
   http://www.xuggle.com/xuggler/downloads/

We are currently using Xuggler LGPL 3.4 - "Forrest".

