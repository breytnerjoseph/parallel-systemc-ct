trace_01 = load("sync_statistics_data_01");
trace_1 = load("sync_statistics_data_1");
trace_10 = load("sync_statistics_data_10");

figure(1);
hold on;
grid on;
plot(trace_01(:,1), trace_01(:,2) , '-ro', 'LineWidth', 2);
plot(trace_1(:,1), trace_1(:,2) , '-go', 'LineWidth', 2);
plot(trace_10(:,1), trace_10(:,2) , '-bo', 'LineWidth', 2);
legend("Initial step 0.1", "Initial step 1", "Initial step 10");

input("Press enter");