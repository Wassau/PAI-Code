%% Modelo  lineal. Azimuth
syms Tm peso b I
Tm = 4.10;    % Torque motor
r = 0.600; % radio del arco
I = 440 * (1/10000); % Inercia aparente del arco en el eje del motor.
masac = 0.40;% peso del trolley
masaa = 0.581; % masa arco
b = 0.08;% friccion en el eje.

A = [   0  1 ; -(masac+masaa)*g*r/I  -b/I ];
B =  [ 1 Tm/I ]';
C = [1 0];
D = 0;
Ts= 0.005;
SS = ss(A,B,C,D)
step(SS)
figure()
rlocus(SS)




%% Ecuacion de trayectoria Zenith Elevacion
clear all 
close all
g = 9.8;
v0 =   0.041888; % velocidad tangencial
theta = 60 * pi/180;


% Y = x*tan(theta) - (g*x^2)/(2*v0 *cos(theta)^2);

xmax = input ('hora');

for i = 0 : 0.2: xmax
    Y = (i*tan(theta)) - (g*i^2)/(2*v0 *cos(theta)^2);
    hold on 
    pause(0.1)
    plot (i,Y,'r--','LineWidth',2)
end
   
%% Polarplot
R = 1.20;
t = 120;
Vt = 0.041888;
W = Vt/ R;
posact = input('posactual');
posini = posact *pi/180;
 theta = posini+ W;
polarplot(0.57,'LineWidth',2,'r--')

Az = 7.6394 * UTC;
% for i= 0 : 0.2 : t
%     theta = posini+ W*i;
%     figure()
%     polarplot(theta,R,'LineWidth',2,'r--')
%     hold on
%     grid on
%     pause (0.3)
% end

%% Hora


    
    
    