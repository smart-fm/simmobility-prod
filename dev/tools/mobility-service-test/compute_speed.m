#!/usr/bin/octave -qf

## usage ./compute_cdf.m 

arg_list = argv ();
filename = arg_list{1};
left = arg_list{2};  %In hours
right = arg_list{3};


pkg load nan;

left_minute = str2double(left)*60;
right_minute = str2double(right)*60;
t = dlmread(filename);

indices = find (  t(:,1) >= left_minute & t(:,1) <= right_minute);
seg_density = t(indices,2);



[f,x] = ecdf(seg_density);
plot(x,f);
grid minor;
outfile = sprintf("%s.%s-%s.speed.cdf.eps", filename, left, right);
printf("\n\nImage written %s\n\n" ,outfile)
saveas(gcf,outfile)
