% Speed-up vs number of processors

processors = [1, 2, 4, 32]

speed_up_error8 = [1 1 1; 1.88 1.80 1.78; 3.08 2.81 2.72; 3.78 3.28 3.11]
% speed_up_error5 = [1, 1.80, 2.81, 3.28]
% speed_up_error3 = [1, 1.78, 2.72, 3.11]


figure(1);
hold on;
grid on;    
% set(gca,'XTick',0:0.05:max_time);
% set(gca,'YTick',0:0.05:0.5);
bar(processors, speed_up_error8);
% bar(processors, speed_up_error5, '-or', 'LineWidth', 2);
% bar(processors, speed_up_error3, '-oc', 'LineWidth', 2);
set(gca, 'XScale','log', 'XTick',processors, 'XTickLabel',processors, 'YTick',sort(A))

drawnow();

input("Press enter to exit");