trace_1 = load("sync_statistics_data");

delta_t = [0.01 0.1 1	2	3	4	5	10	100 ];

wc_time = [ 0.63972217	0.07365583	0.02939617	0.02380150	0.02227933	0.02119967	0.02210683	0.02314733	0.06317650 ];

% figure(1);
% hold on;
% grid on;
% plot(trace_1(:,1), trace_1(:,2) , '-o', 'LineWidth', 2);

figure(2);
hold on;
grid on;
loglog(delta_t, wc_time, '-bo', 'LineWidth', 2);
loglog([0.01 100], [0.0218 0.0218], '-r', 'LineWidth', 2);
ytick = [0.640 0.0737 0.0294 0.0212];
xtick = [0.01 0.1 1	4	10	100 ]
set(gca,'YTick', ytick);
set(gca,'XTick', xtick);
legend("Wall-clock time vs Dt", "Wall-clock time adaptive Dt")


input("Press enter");