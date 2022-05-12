% m1 * m2 = -1
% y1 = m1 * x1
% y2 = m2 * x2
% (x - x1)^2 + (y - y1)^2 = d1^2
% (x - x2)^2 + (y - y2)^2 = d1^2
% (x2 - x1)^2 + (y2 - y1)^2 = d1^2 + d2^2 - 2 * k

% (x2 - x1)^2 + (y2 - y1)^2 = (x - x1)^2 + (y - y1)^2 + (x - x2)^2 + (y - y2)^2 - 2 * k
% x2^2 + x1^2 - 2 * x2 * x1 + y2^2 + y1^2 - 2 * y1 * y2 = x^2 + x1^2 - 2 * x * x1 + y^2 + y1^2 - 2 * y * y1 + x^2 + x2^2 - 2 * x * x2 + y^2 + y2^2 - 2 * y * y2 - 2 * k
% - x2 * x1 - y1 * y2 = - x * x1 - y * y1 + - x * x2 - y * y2 + x^2 + y^2 - k
% x2 * x1 + m1 * x1 * m2 * x2 - x * x1 - y * m1 * x1 + - x * x2 - y * m2 * x2 + x^2 + y^2 - k = 0

% - (x + m1 * y) * x1 - (x + m2 * y) * x2 + x^2 + y^2 - k = 0

% x1 = (r1 + sqrt(s1))
% x2 = (r2 + sqrt(s2))
% - (x + m1 * y) * (r1 + sqrt(s1)) - (x + m2 * y) * (r2 + sqrt(s2)) = - x^2 - y^2 + k

% - sqrt(s1) * (x + m1 * y)
% - sqrt(s2) * (x + m2 * y)
% = r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k

% + sqrt(s1) * (- x - m1 * y)
% + sqrt(s2) * (- x - m2 * y)
% = r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k

% + s1 * (x + m1 * y)^2
% + s2 * (x + m2 * y)^2
% + 2 * sqrt(s1 * s2) * (x + m1 * y) * (x + m2 * y) =
% + (r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k)^2

% + 2 * sqrt(s1 * s2) * (x + m1 * y) * (x + m2 * y) =
% + (r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k)^2
% - s1 * (x + m1 * y)^2
% - s2 * (x + m2 * y)^2

% 4 * s1 * s2 * (x + m1 * y)^2 * (x + m2 * y)^2 =
% ((r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k)^2 - s1 * (x + m1 * y)^2 - s2 * (x + m2 * y)^2)^2


% eq11 = r1 == (x + m1 * y) / (1 + m1^2)
% eq12 = s1 == ((x + m1 * y)^2 - (1 + m1^2) * (x^2 + y^2 - d1^2)) / (1 + m1^2)^2
% eq21 = r2 == (x + m2 * y) / (1 + m2^2)
% eq22 = s2 == ((x + m2 * y)^2 - (1 + m2^2) * (x^2 + y^2 - d2^2)) / (1 + m2^2)^2

syms d1 d2 k m1 m2 x y
eq = 4 * ((x + m1 * y)^2 - (1 + m1^2) * (x^2 + y^2 - d1^2)) / (1 + m1^2)^2 * ((x + m2 * y)^2 - (1 + m2^2) * (x^2 + y^2 - d2^2)) / (1 + m2^2)^2 * (x + m1 * y)^2 * (x + m2 * y)^2 == (((x + m1 * y) / (1 + m1^2) * (x + m1 * y) + (x + m2 * y) / (1 + m2^2) * (x + m2 * y) - x^2 - y^2 + k)^2 - ((x + m1 * y)^2 - (1 + m1^2) * (x^2 + y^2 - d1^2)) / (1 + m1^2)^2 * (x + m1 * y)^2 - ((x + m2 * y)^2 - (1 + m2^2) * (x^2 + y^2 - d2^2)) / (1 + m2^2)^2 * (x + m2 * y)^2)^2
sol_o = reduceRedundancies([eq, m1 * m2 == -1], [x, y, m1])

fid_o = fopen('sol_o2.txt', 'wt')
fprintf(fid_o, '%s\n', char(sol_o))
fclose(fid_o)

%% sol_oa = subs(sol_o, [m1], [0])
%% fid_oa = fopen('sol_o.txt', 'wt')
%% fprintf(fid_oa, '%s\n', char(sol_oa))
%% fclose(fid_oa)
