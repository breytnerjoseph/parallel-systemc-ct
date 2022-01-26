delta_t = [
0.001
0.01
0.10
1.00
2.00
3.00
4.00
5.00
8.00
10.00
];

wallclock_t = [
6.73138720
0.6860812
0.0815144
0.0294698
0.0245616
0.0245368
0.0228758
0.024428
0.0261526
0.027277
]

figure(1);
hold on;
grid on;
semilogy(delta_t, wallclock_t, '-bo', 'LineWidth', 2);
set(gca,'YTick', [6.7314 0.6861 0.0815 0.0227]);
set(gca,'XTick', delta_t);

legend("Delta t");

input("Press enter");