% Load data
trace_1_seq = load("ct_test_module0.dat");
trace_2_seq = load("ct_test_module1.dat");
trace_3_seq = load("ct_test_module2.dat");
trace_4_seq = load("ct_test_module3.dat");

trace_1_par = load("ct_test_module0_par.dat");
trace_2_par = load("ct_test_module1_par.dat");
trace_3_par = load("ct_test_module2_par.dat");
trace_4_par = load("ct_test_module3_par.dat");

% trace_ctde2 = load("circuit0odeint.dat");
%  trace_ctde2 = load("circuit1.dat");

max_time = 2;

figure(1);
hold on;
grid on;    
% set(gca,'XTick',0:0.05:max_time);
% set(gca,'YTick',0:0.05:0.5);
plot(trace_1_seq(:,1), trace_1_seq (:,2), '-b', 'LineWidth', 1);
plot(trace_1_par(:,1), trace_1_par (:,2), '-r', 'LineWidth', 1);

figure(2);
hold on;
grid on;    
% set(gca,'XTick',0:0.05:max_time);
% set(gca,'YTick',0:0.05:0.5);
plot(trace_2_seq(:,1), trace_2_seq (:,2), '-b', 'LineWidth', 1);
plot(trace_2_par(:,1), trace_2_par (:,2), '-r', 'LineWidth', 1);

figure(3);
hold on;
grid on;    
% set(gca,'XTick',0:0.05:max_time);
% set(gca,'YTick',0:0.05:0.5);
plot(trace_3_seq(:,1), trace_3_seq (:,2), '-b', 'LineWidth', 1);
plot(trace_3_par(:,1), trace_3_par (:,2), '-r', 'LineWidth', 1);

figure(4);
hold on;
grid on;    
% set(gca,'XTick',0:0.05:max_time);
% set(gca,'YTick',0:0.05:0.5);
plot(trace_4_seq(:,1), trace_4_seq (:,2), '-b', 'LineWidth', 1);
plot(trace_4_par(:,1), trace_4_par (:,2), '-r', 'LineWidth', 1);

% plot(trace_ctde2(:,1), trace_ctde2(:,2), '-or', 'LineWidth', 2);

% // figure(2);
% // hold on;
% // grid on;    
% // set(gca,'XTick',0:0.05:max_time);
% // set(gca,'YTick',0:0.05:0.5);
% // axis([0 max_time -0.002 0.502]);
% // plot(trace_ctde2(:,1), trace_ctde2(:,2), '-or', 'LineWidth', 2);

drawnow();

input("Press enter to exit");