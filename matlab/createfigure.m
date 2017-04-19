function createfigure()

exp1 = csvread('~/Dropbox/Studium/Bachelor Thesis/Thesis/data/exp1.log');
exp2 = csvread('~/Dropbox/Studium/Bachelor Thesis/Thesis/data/exp2.log');

figure1 = figure;
axes1 = axes('Parent',figure1);
cdfplot(abs(exp1(:,2)));
hold on;
cdfplot(abs(exp2(:,2)));

xlabel('Offset from master in nanoseconds (log)','FontSize',13.2,'FontName','SansSerif');
ylabel('F(x)','FontSize',13.2,'FontName','SansSerif');

box(axes1,'on');
grid(axes1,'on');

set(axes1,'FontName','SansSerif','FontSize',12,'XMinorTick','on','XScale','log');

legend1 = legend('Software','Hardware','Location','NW');
set(legend1,'Location','northwest','FontSize',10.8);
