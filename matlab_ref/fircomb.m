% Authors: P. Dutilleux, U Zölzer
%
%--------------------------------------------------------------------------
% This source code is provided without any warranties as published in 
% DAFX book 2nd edition, copyright Wiley & Sons 2011, available at 
% http://www.dafx.de. It may be used for educational purposes and not 
% for commercial applications without further permission.
%--------------------------------------------------------------------------
filename = '/Users/jocekav/Documents/GitHub/2022-MUSI6106/bagpipe.wav';

[x,Fs] = audioread(filename);

% x=zeros(100,1);x(1)=1; % unit impulse signal of length 100
g=0.5;
Delayline=zeros(100,1);% memory allocation for length 10
for n=1:length(x)
	y(n)=x(n)+g*Delayline(100);
	Delayline=[x(n);Delayline(1:100-1)];
end

compFile = '/Users/jocekav/Documents/GitHub/2022-MUSI6106/bagpipe_FIR.wav';
[compX,Fs] = audioread(compFile);

diff = transpose(compX(1:22000)) - y(1:22000);
xAxis = 1:22000;
% ln1 = plot(xAxis, transpose(compX(1:22000)), xAxis, y(1:22000), xAxis, diff, 'LineWidth', 1);
% legend({'Matlab function','C++ Implementation', 'Difference'})
% title('FIR comparison')

plot(diff, 'LineWidth', 1);
title('FIR difference')
