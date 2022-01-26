ct_de = load("ct_de_100ms.dat");
tdf = load("tdf_100ms.dat") 
speed_matlab = load("matlab_data/speed.txt");
gear_matlab = load("matlab_data/gear.txt");

figure(1);
title('Vehicle speed - SystemC datapoints spaced 1 s');
hold on;
grid on;
plot(ct_de(:,1), ct_de(:,2) , 'or', 'LineWidth', 1);
plot(tdf(:,1), tdf(:,2) , '-k', 'LineWidth', 1);
plot(speed_matlab(:,1), speed_matlab(:,2) , 'ob', 'LineWidth', 1);
legend("Speed SC CT/DE", "Speed SC TDF", "Speed MATLAB");
ylim([0, 55]);
xlim([0, 20]);



figure(2);
title('Gear shifting - SystemC datapoints spaced 1 s');
hold on;
grid on;
plot(ct_de(:,1), ct_de(:,3) , 'or', 'LineWidth', 1);
plot(tdf(:,1), tdf(:,3) , '-k', 'LineWidth', 1);
plot(gear_matlab(:,1), gear_matlab(:,2) , 'ob', 'LineWidth', 1);
legend("Gear SC CT/DE", "Gear SC TDF", "Gear MATLAB");
ylim([0.5, 4.5]);
xlim([0, 20]);


input("Press enter");