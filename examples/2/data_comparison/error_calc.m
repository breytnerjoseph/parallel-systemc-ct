pkg load signal;

ct_de_1sec = load("ct_de_1sec.dat");
tdf_1sec = load("tdf_1sec.dat");
tdf_10ms = load("tdf_10ms.dat") 
speed_matlab = load("matlab_data/speed.txt");
gear_matlab = load("matlab_data/gear.txt");
thresholds = load("internal.txt");

tdf1_area = trapz(tdf_1sec(:,2), tdf_1sec(:,1))
tdf10_area = trapz(tdf_10ms(:,2), tdf_10ms(:,1))
ct_area = trapz(ct_de_1sec(:,2), ct_de_1sec(:,1))
matlab_area = trapz(speed_matlab(:,2), speed_matlab(:,1))

figure(1);
title('Vehicle speed - SystemC');
hold on;
grid on;
plot(ct_de_1sec(:,1), ct_de_1sec(:,2) , 'or', 'LineWidth', 2);
plot(speed_matlab(:,1), speed_matlab(:,2) , '-b', 'LineWidth', 2);
plot(tdf_1sec(:,1), tdf_1sec(:,2) , '--k', 'LineWidth', 1);
plot(tdf_10ms(1:100:end,1), tdf_10ms(1:100:end,2) , 'og', 'LineWidth', 2);