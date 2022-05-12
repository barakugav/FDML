% m1 = 0
% y1 = 0
% y2 = m2 * x2
% (x - x1)^2 + y^2 = d1^2
% (x - x2)^2 + (y - y2)^2 = d1^2
% (x2 - x1)^2 + y2^2 = d1^2 + d2^2 - 2 * k

% x1 = x + sqrt(d1^2 - y^2)
% x1 = (r1 + sqrt(s1))
% r1 = x
% s1 = (d1^2 - y^2)

% (x - x2)^2 + (y - m2 * x2)^2 = d1^2
% x^2 + x2^2 - 2 * x2 * x + y^2 + m2^2 * x2^2 - 2 * x2 * m2 * y - d1^2 = 0
% (1 + m2^2) * x2^2 - 2 * x2 * (x + m2 * y) + x^2 + y^2 - d1^2 = 0
% x2 = (x + m2 * y)/(1 + m2^2) + sqrt(((x + m2 * y)^2 - (1 + m2^2) * (x^2 + y^2 - d1^2)) / (1 + m2^2)^2)

% x2 = (r2 + sqrt(s2))
% r2 = ((x + m2 * y) / (1 + m2^2))
% s2 = (((x + m2 * y)^2 - (1 + m2^2) * (x^2 + y^2 - d1^2)) / (1 + m2^2)^2)

% (x2 - x1)^2 + y2^2 = (x - x1)^2 + y^2 + (x - x2)^2 + (y - y2)^2 - 2 * k
% x2^2 + x1^2 - 2 * x2 * x1 + y2^2 = x^2 + x1^2 - 2 * x * x1 + y^2 + x^2 + x2^2 - 2 * x * x2 + y^2 + y2^2 - 2 * y * y2 - 2 * k
% x2 * x1 - x * x1 - x * x2 - m2 * y * x2 + x^2 + y^2 - k = 0

% (r1 + sqrt(s1)) * (r2 + sqrt(s2)) - (r1 + sqrt(s1)) * x - (r2 + sqrt(s2)) * x - (r2 + sqrt(s2)) * m2 * y + x^2 + y^2 - k = 0
% r1 * r2 + sqrt(s1) * r2 + sqrt(s2) * r1 + sqrt(s1 * s2) - r1 * x - sqrt(s1) * x - r2 * x - sqrt(s2) * x - r2 * m2 * y - sqrt(s2) * m2 * y + x^2 + y^2 - k = 0
% x * r2 + sqrt(s1) * r2 + sqrt(s2) * x + sqrt(s1 * s2) - x * x - sqrt(s1) * x - r2 * x - sqrt(s2) * x - r2 * m2 * y - sqrt(s2) * m2 * y + x^2 + y^2 - k = 0
% sqrt(s1) * r2 + sqrt(s1 * s2) - sqrt(s1) * x - r2 * m2 * y - sqrt(s2) * m2 * y + y^2 - k = 0
% sqrt(s1) * (r2 - x) - sqrt(s2) * m2 * y = (r2 * m2 * y - y^2 + k) - sqrt(s1 * s2)
% s1 * (r2 - x)^2 + s2 * m2^2 * y^2 - 2 * sqrt(s1 * s2) (r2 - x) * m2 * y = (r2 * m2 * y - y^2 + k)^2 + s1 * s2 - 2 * sqrt(s1 * s2) * (r2 * m2 * y - y^2 + k)
% 2 * sqrt(s1 * s2) * (r2 * m2 * y - y^2 + k - (r2 - x) * m2 * y = (r2 * m2 * y - y^2 + k)^2 + s1 * s2 - s1 * (r2 - x)^2 - s2 * m2^2 * y^2
% 2 * sqrt(s1 * s2) * (m2 * y * x - y^2 + k) = (r2 * m2 * y - y^2 + k)^2 + s1 * s2 - s1 * (r2 - x)^2 - s2 * m2^2 * y^2
% 4 * s1 * s2 * (m2 * y * x - y^2 + k)^2 = ((r2 * m2 * y - y^2 + k)^2 + s1 * s2 - s1 * (r2 - x)^2 - s2 * m2^2 * y^2)^2

syms d1 d2 k m2 x y s1 r2 s2
eqb = 4 * s1 * s2 * (m2 * y * x - y^2 + k)^2 == ((r2 * m2 * y - y^2 + k)^2 + s1 * s2 - s1 * (r2 - x)^2 - s2 * m2^2 * y^2)^2
eq = subs(eqb, [s1, r2, s2], [(d1^2 - y^2), ((x + m2 * y) / (1 + m2^2)), (((x + m2 * y)^2 - (1 + m2^2) * (x^2 + y^2 - d2^2)) / (1 + m2^2)^2)])
sol_a = reduceRedundancies([eq], [x, y, m2])
fid_a = fopen('sol_a.txt', 'wt');
fprintf(fid_a, '%s\n', char(sol_a));
fclose(fid_a);
