trace_1 = load("speed.dat");
trace_2 = load("../data_comparison/matlab_data/speed.txt");
trace_3 = load("../data_comparison/matlab_data/gear.txt");

figure(1);
hold on;
grid on;
plot(trace_1(:,1), trace_1(:,2) , '-ro', 'LineWidth', 2);
plot(trace_2(:,1), trace_2(:,2) , '-b', 'LineWidth', 2);
legend("Speed SC", "Speed MATLAB");
xlim([0 30]);
% ylim([0 6]);



figure(2);
hold on;
grid on;
plot(trace_1(:,1), trace_1(:,3) , '-g', 'LineWidth', 2);
plot(trace_3(:,1), trace_3(:,2) , '-b', 'LineWidth', 2);
legend("Gear SC", "Gear MATLAB");


input("Press enter");