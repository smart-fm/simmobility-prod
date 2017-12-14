#!/usr/bin/octave-cli -q

## usage ./compute_cdf.m 

arg_list = argv ();
filename = arg_list{1};
left = arg_list{2};  %In hours
right = arg_list{3};

warning("off")
pkg load nan;

left_minute = str2double(left)*60;
right_minute = str2double(right)*60;
t = dlmread(filename);

indices = find (  t(:,1) >= left_minute & t(:,1) <= right_minute);
seg_density = t(indices,2);



[f,x] = ecdf(seg_density);
plot(x,f);
axis([0 290 0.5 1])
grid minor;
outfile = sprintf("%s.%s-%s.seg_density.cdf.eps\n\n", filename, left, right);
printf("\n\nImage written %s" ,outfile)
saveas(gcf,outfile)

outdatafile = sprintf("%s.%s-%s.seg_density.cdf.dat", filename, left, right);
dlmwrite(outdatafile, [x,f], 'delimiter', ' ', 'precision',16)
printf("Data file written %s\n\n" ,outdatafile)

