% Load data
trace_ctde1 = load("circuit0.dat");
trace_ctde2 = load("circuit0odeint.dat");
%  trace_ctde2 = load("circuit1.dat");

max_time = 2;

figure(1);
hold on;
grid on;    
% set(gca,'XTick',0:0.05:max_time);
% set(gca,'YTick',0:0.05:0.5);
axis([0 max_time -0.002 0.502]);
plot(trace_ctde1(:,1), trace_ctde1(:,2), '-ob', 'LineWidth', 2);
plot(trace_ctde2(:,1), trace_ctde2(:,2), '-or', 'LineWidth', 2);

% // figure(2);
% // hold on;
% // grid on;    
% // set(gca,'XTick',0:0.05:max_time);
% // set(gca,'YTick',0:0.05:0.5);
% // axis([0 max_time -0.002 0.502]);
% // plot(trace_ctde2(:,1), trace_ctde2(:,2), '-or', 'LineWidth', 2);

drawnow();

input("Press enter to exit");