#!/usr/bin/octave-cli -q

## usage ./compute_cdf.m filename

warning("off")
arg_list = argv ();
filename = arg_list{1};

pkg load nan;

rowdata = dlmread(filename);



[f,x] = ecdf(rowdata);
plot(x,f);
grid minor;
%axis ([0 290])
outfile = sprintf("%s.cdf.eps", filename);
outdatafile = sprintf("%s.cdf.dat", filename);

saveas(gcf,outfile)
printf("\n\nImage written %s\n" ,outfile)

dlmwrite(outdatafile, [x,f], " ")
printf("Data file written %s\n\n" ,outdatafile)
