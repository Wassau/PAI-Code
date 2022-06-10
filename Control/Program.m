%% Modelo no lineal. Azimuth
syms Tm peso b I
Tm = 1;    % Torque motor
Radio = 1; % radio del arco
I = 1; % Inercia aparente del arco en el eje del motor.
peso = 30;% peso del trolley 
b = 0.08;% friccion en el eje.

A = [  0 1 ;  1  -b/I ];
B =  [ Tm/I -peso*r/I -b/I]';
C = [1 0];
D = 0;
Ts= 0.005;

%% Ecuacion de trayectoria Zenith Elevacion
clear all 
close all
g = 9.8;
v0 =   0.041888; % velocidad tangencial
theta = 60 * pi/180;

prueba = 0 : 0.2 : 12;
%  Y = prueba *tan(theta) - (g*prueba.^2)/(2*v0 *cos(theta)^2);

xmax = input ('hora');

% plot(prueba,Y)
for i = 0 : 0.2: xmax
    Y = (i*tan(theta)) - (g*i^2)/(2*v0 *cos(theta)^2);
    hold on 
    plot (i,Y,'r--*')
    pause(0.1)
end
   
%% Polarplot
R = 1.20;
t = 1.20;
Vt = 0.041888;
W = Vt/ R;
posact = input('posactual');
posini = posact *pi/180;
 theta = posini+ W;
% polarplot(theta,1,'r--','LineWidth',2)
thet = 0: 0.2*pi/180 : theta;
% Az = 7.6394 * UTC;
polarplot(thet,rho*R,'r--*')

%% Hora


T = timezones('America');









