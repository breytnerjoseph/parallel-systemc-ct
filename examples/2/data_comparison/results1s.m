ct_de_1sec = load("ct_de_1sec.dat");
tdf_1sec = load("tdf_1sec.dat");
tdf_10ms = load("tdf_10ms.dat") 
speed_matlab = load("matlab_data/speed.txt");
gear_matlab = load("matlab_data/gear.txt");
thresholds = load("internal.txt");

figure(1);
title('Vehicle speed - SystemC');
hold on;
grid on;
plot(ct_de_1sec(:,1), ct_de_1sec(:,2) , 'or', 'LineWidth', 2);
plot(speed_matlab(:,1), speed_matlab(:,2) , '-b', 'LineWidth', 2);
plot(tdf_1sec(:,1), tdf_1sec(:,2) , '--k', 'LineWidth', 1);
plot(tdf_10ms(1:100:end,1), tdf_10ms(1:100:end,2) , 'og', 'LineWidth', 2);
plot(thresholds(:,1), thresholds(:,2) , '-.b', 'LineWidth', 1);
plot(thresholds(:,1), thresholds(:,3) , '-.r', 'LineWidth', 1);

legend("Speed SC CT/DE", "Speed MATLAB", "Speed SC TDF 1s", "Speed SC TDF 10ms", "Up threshold", "Down threshold" );
ylim([0, 55]);
xlim([0, 20]);
xticks([0:1:20]);



% figure(2);
% title('Gear shifting - SystemC');
% hold on;
% grid on;
% plot(ct_de_1sec(:,1), ct_de_1sec(:,3) , 'or', 'LineWidth', 2);
% plot(gear_matlab(:,1), gear_matlab(:,2) , '-b', 'LineWidth', 2);
% plot(tdf_1sec(:,1), tdf_1sec(:,3) , '--k', 'LineWidth', 1);
% plot(tdf_10ms(:,1), tdf_10ms(:,3) , '--g', 'LineWidth', 1);
% legend("Gear SC CT/DE", "Gear MATLAB", "Gear SC TDF 1s", "Gear SC TDF 10ms");
% ylim([0.5, 4.5]);
% xlim([0, 20]);


input("Press enter");