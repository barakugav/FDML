% y1 = m1 * x1
% y2 = m2 * x2
% (x - x1)^2 + (y - y1)^2 = d1^2
% (x - x2)^2 + (y - y2)^2 = d1^2
% (x2 - x1)^2 + (y2 - y1)^2 = d1^2 + d2^2 - 2 * k


% (x2 - x1)^2 + (y2 - y1)^2 = (x - x1)^2 + (y - y1)^2 + (x - x2)^2 + (y - y2)^2 - 2 * k
% x2^2 + x1^2 - 2 * x2 * x1 + y2^2 + y1^2 - 2 * y1 * y2 = x^2 + x1^2 - 2 * x * x1 + y^2 + y1^2 - 2 * y * y1 + x^2 + x2^2 - 2 * x * x2 + y^2 + y2^2 - 2 * y * y2 - 2 * k
% - x2 * x1 - y1 * y2 = - x * x1 - y * y1 + - x * x2 - y * y2 + x^2 + y^2 - k
% x2 * x1 + m1 * x1 * m2 * x2 - x * x1 - y * m1 * x1 + - x * x2 - y * m2 * x2 + x^2 + y^2 - k = 0

% (1 + m1 * m2) x1 * x2 - (x + m1 * y) * x1 - (x + m2 * y) * x2 + x^2 + y^2 - k = 0

% x1 = (r1 + sqrt(s1))
% x2 = (r2 + sqrt(s2))
% (1 + m1 * m2) (r1 + sqrt(s1)) * (r2 + sqrt(s2)) - (x + m1 * y) * (r1 + sqrt(s1)) - (x + m2 * y) * (r2 + sqrt(s2)) = - x^2 - y^2 + k


% + (1 + m1 * m2) * (r1 * r2)
% + (1 + m1 * m2) * r1 * sqrt(s2)
% + (1 + m1 * m2) * r2 * sqrt(s1)
% + (1 + m1 * m2) * sqrt(s1 * s2))
% - sqrt(s1) * (x + m1 * y)
% - sqrt(s2) * (x + m2 * y)
% = r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k

% + sqrt(s1) * (r2 * (1 + m1 * m2) - x - m1 * y)
% + sqrt(s2) * (r1 * (1 + m1 * m2) - x - m2 * y)
% = r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) (r1 * r2) - sqrt(s1 * s2) * (1 + m1 * m2)

% + s1 * (r2 * (1 + m1 * m2) - x - m1 * y)^2
% + s2 * (r1 * (1 + m1 * m2) - x - m2 * y)^2
% + 2 * sqrt(s1 * s2) * (r2 * (1 + m1 * m2) - x - m1 * y) * (r1 * (1 + m1 * m2) - x - m2 * y) =
% + (r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) (r1 * r2))^2
% + (1 + m1 * m2)^2 * s1 * s2
% - 2 * sqrt(s1 * s2) * (1 + m1 * m2) * (r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) (r1 * r2))

% + 2 * sqrt(s1 * s2) * ((r2 * (1 + m1 * m2) - x - m1 * y) * (r1 * (1 + m1 * m2) - x - m2 * y) + (1 + m1 * m2) * (r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) (r1 * r2))) =
% + (r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) * (r1 * r2))^2
% + (1 + m1 * m2)^2 * s1 * s2
% - s1 * (r2 * (1 + m1 * m2) - x - m1 * y)^2
% - s2 * (r1 * (1 + m1 * m2) - x - m2 * y)^2

% 4 * s1 * s2 * ((r2 * (1 + m1 * m2) - x - m1 * y) * (r1 * (1 + m1 * m2) - x - m2 * y) + (1 + m1 * m2) * (r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) * (r1 * r2)))^2 =
% + ((r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) * (r1 * r2))^2
% + (1 + m1 * m2)^2 * s1 * s2
% - s1 * (r2 * (1 + m1 * m2) - x - m1 * y)^2
% - s2 * (r1 * (1 + m1 * m2) - x - m2 * y)^2)^2

% eq11 = r1 == ((x + m1 * y) / (1 + m1^2))
% eq12 = s1 == (((x + m1 * y)^2 - (1 + m1^2) * (x^2 + y^2 - d1^2)) / (1 + m1^2)^2)
% eq21 = r2 == ((x + m2 * y) / (1 + m2^2))
% eq22 = s2 == (((x + m2 * y)^2 - (1 + m2^2) * (x^2 + y^2 - d2^2)) / (1 + m2^2)^2)

syms r1 s1 r2 s2 d1 d2 k m1 m2 x y
eqb = 4 * s1 * s2 * ((r2 * (1 + m1 * m2) - x - m1 * y) * (r1 * (1 + m1 * m2) - x - m2 * y) + (1 + m1 * m2) * (r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) * (r1 * r2)))^2 == ((r1 * (x + m1 * y) + r2 * (x + m2 * y) - x^2 - y^2 + k - (1 + m1 * m2) * (r1 * r2))^2 + (1 + m1 * m2)^2 * s1 * s2 - s1 * (r2 * (1 + m1 * m2) - x - m1 * y)^2 - s2 * (r1 * (1 + m1 * m2) - x - m2 * y)^2)^2
eq = subs(eqb, [r1, s1, r2, s2], [((x + m1 * y) / (1 + m1^2)),(((x + m1 * y)^2 - (1 + m1^2) * (x^2 + y^2 - d1^2)) / (1 + m1^2)^2),((x + m2 * y) / (1 + m2^2)),(((x + m2 * y)^2 - (1 + m2^2) * (x^2 + y^2 - d2^2)) / (1 + m2^2)^2)])

sol = reduceRedundancies([eq], [x, y, m1, m2])
fid = fopen('sol.txt', 'wt')
fprintf(fid, '%s\n', char(sol))
fclose(fid)

%%%%%%%% Solve for the x-axis aligned case
sol_a = subs(sol, [m1], [0])
fid_a = fopen('sol_a.txt', 'wt')
fprintf(fid_a, '%s\n', char(sol_a))
fclose(fid_a)
[p1, p2, p3, p4, p5, p6, p7, p8, p9] = compute_terms(sol_a)
[p1, p2, p3, p4, p5, p6, p7, p8, p9] = negate_terms(p1, p2, p3, p4, p5, p6, p7, p8, p9)
syms A1 B1 C1 D1 A2 B2 C2 D2 real
eq10 = A1 == d1^2*m2^2
eq14 = A2 == d1^2*m2^2
sol_c_a = compute_coeffs(p1, p2, p3, p4, p5, p6, p7, p8, p9, eq10)
fid_c_a = fopen('sol_c_a.txt', 'wt')
print_coeffs(fid_c_a, sol_c_a)
fclose(fid_c_a)

%%%%%%%% Solve for the orthogonal case
sol_o = reduceRedundancies([eq, m1 * m2 == -1], [x, y, m2])
fid_o = fopen('sol_o.txt', 'wt')%
fprintf(fid_o, '%s\n', char(sol_o))
fclose(fid_o)
[p1, p2, p3, p4, p5, p6, p7, p8, p9] = compute_terms(sol_o)
[p1, p2, p3, p4, p5, p6, p7, p8, p9] = negate_terms(p1, p2, p3, p4, p5, p6, p7, p8, p9)
syms A1 B1 C1 D1 A2 B2 C2 D2 real
eq17 = D2 == k^2*(m1^2 + 1)
sol_c_o = compute_coeffs(p1, p2, p3, p4, p5, p6, p7, p8, p9, eq17)
fid_c_o = fopen('sol_c_o.txt', 'wt')
print_coeffs(fid_c_o, sol_c_o)
fclose(fid_c_o)

%%%%%%%% Solve for the simplest case, namely, orthogonal and axis aligned
sol_oa = subs(sol_o, [m1], [0])
fid_oa = fopen('sol_oa.txt', 'wt')
fprintf(fid_oa, '%s\n', char(sol_oa))
fclose(fid_oa)
[p1, p2, p3, p4, p5, p6, p7, p8, p9] = compute_terms(sol_oa)
[p1, p2, p3, p4, p5, p6, p7, p8, p9] = negate_terms(p1, p2, p3, p4, p5, p6, p7, p8, p9)
syms A1 B1 C1 D1 A2 B2 C2 D2 real
eq13 = D1 == -k^2
sol_c_oa = compute_coeffs(p1, p2, p3, p4, p5, p6, p7, p8, p9, eq13)
fid_c_oa = fopen('sol_c_oa.txt', 'wt')
print_coeffs(fid_c_oa, sol_c_oa)
fclose(fid_c_oa)
