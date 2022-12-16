% Simple MATLAB code to receive VILLAS UDP samples
%
% @author Megha Gupta <meghagupta1191@gmail.com>
% @author Steffen Vogel <post@steffenvogel.de>
% @copyright 2014-2022, Institute for Automation of Complex Power Systems, EONERC
% @license Apache 2.0
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

close all;
clear all;
clc;

% Configuration and connection
t = udp('127.0.0.1',                ...
    'LocalPort', 12000,             ...
    'RemotePort', 12001,            ...
    'Timeout', 60,                  ...
    'DatagramTerminateMode', 'on' ...
);
disp('UDP Receiver started');

fopen(t);
disp('UDP Socket bound');

num_values = 5;
num_samples = 500;

data = zeros(num_values, num_samples);
i = 1;

disp('Receiving data');
while i < num_samples
    % Wait for connection
    % Read data from the socket
    
    [ dat, count ] = fread(t, num_values, 'float32');
    
    data(:,i) = dat;
    i = i + 1;
end

plot(data');

fclose(t);
delete(t);
