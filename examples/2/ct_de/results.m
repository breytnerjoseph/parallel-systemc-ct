% trace_1 = load("speed.dat");
trace_1 = load("vehicle0.dat");
trace_2 = load("vehicle0rk.dat");
% speed = load("vehicle_ct.dat");
% trace_2 = load("../data_comparison/matlab_data/speed.txt");
% trace_3 = load("../data_comparison/matlab_data/gear.txt");
% thresholds = load("internal.txt");




figure(1)
hold on;
grid on;
% plot(trace_1(:,1), trace_1(:,2) , '-ro', 'LineWidth', 2);
% plot(speed(:,1), speed(:,3) , '-ro', 'LineWidth', 2);
plot(trace_1(:,1), trace_1(:,3) , '-b', 'LineWidth', 2);
plot(trace_2(:,1), trace_2(:,3) , '--r', 'LineWidth', 2);
% plot(trace_2(:,1), trace_2(:,2) , 'or', 'LineWidth', 2);
% plot(thresholds(:,1), thresholds(:,2) , '-.b', 'LineWidth', 1);
% plot(thresholds(:,1), thresholds(:,3) , '-.r', 'LineWidth', 1);
legend("Speed SC CT/DE", "Speed MATLAB", "Up threshold", "Down threshold");
xlim([0 25]);
ylim([0 65]);


% figure(2);
% hold on;
% grid on;
% plot(trace_1(:,1), trace_1(:,3) , '-g', 'LineWidth', 2);
% plot(trace_3(:,1), trace_3(:,2) , '-b', 'LineWidth', 2);
% legend("Gear SC", "Gear MATLAB");
% xlim([0 25]);
% ylim([0.5 4.5]);

drawnow();


input("Press enter");